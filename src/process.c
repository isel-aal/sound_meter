/*
Copyright 2022 Guilherme Albano, David Meneses e Laboratório de Audio e Acústica do ISEL

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
/*
Este ficheiro é baseado no ficheiro com o mesmo nome pertencente
ao PFC MoSEMusic realizado por Guilherme Albano e David Meneses
*/

#include "process.h"
#include "config.h"
#include "ring.h"

#define LINEAR_TO_DECIBEL(linear)		(20.0f * log10(linear / CONFIG_PRESSURE_REFERENCE))
#define DECIBEL_TO_LINEAR(dBSPL)		(CONFIG_PRESSURE_REFERENCE * pow(10, dBSPL / 20.0f))

Block *block_create(unsigned blocks_per_segment, unsigned block_size, unsigned last_block_size) {
	//	Dimensão do array de amostras originais
	size_t array_sample_int16_size = block_size * sizeof ((Block*)0)->sample_int16[0];
	//	Dimensão do array de amostras em formato float
	size_t array_sample_size = block_size * sizeof ((Block*)0)->sample_a[0];
	Block *block = malloc(sizeof *block
							+ array_sample_int16_size
							+ array_sample_size * 4);
	if (block == NULL)
		return NULL;
	block->sample_int16 = (int16_t*)((char *)block + sizeof *block);
	block->sample_a = (float*)((char *)block->sample_int16 +array_sample_int16_size);
	block->sample_b = (float*)((char *)block->sample_a + array_sample_size);
	block->sample_c = (float*)((char *)block->sample_b + array_sample_size);
	block->sample_d = (float*)((char *)block->sample_c + array_sample_size);

	block->blocks_per_segment = blocks_per_segment;
	block->block_size = block_size;
	block->last_block_size = last_block_size;
	block->count = 0;
	block->block_number = 0;
	return block;
}

void block_destroy(Block *block) {
	free(block);
}

void block_sample_to_float(Block *block) {
	for (unsigned i = 0; i < block->count; i++)
		//	Converte para float e normaliza no intervalo +1 ... -1
		block->sample_a[i] = block->sample_int16[i] / CONFIG_SAMPLE_NORM;
}

unsigned block_next_size(Block *block) {
	if (block->block_number == block->blocks_per_segment - 1)	//	Último bloco do segmento?
		return block->last_block_size;
	else
		return block->block_size;
}

//=============================================================================

static double laeq_accumulator;
static size_t laeq_counter;

/**
 * lae_average_create:
 * Inicializar o cálculo de LAEq
 */
void lae_average_create(unsigned laeq_time) {
	laeq_accumulator = 0;
	laeq_counter = 0;
}

void lae_average_destroy() {
}

/**
 * lae_average:
 * @lae: Valor LAE do segmento corrente
 *
 *  Cálcular LAEq
 *
 * Returns: Valor LAEq
 */
float lae_average(float lae) {
	laeq_accumulator += lae;
	laeq_counter++;
	return laeq_accumulator / laeq_counter;
}

//=============================================================================

Calibrator *calibrator_create(unsigned blocks_per_segment, unsigned calibration_time) {
	Calibrator *cal = malloc(sizeof *cal);
	if (cal == NULL)
		return NULL;
	cal->prms = malloc(blocks_per_segment * calibration_time * sizeof *cal->prms);
	if (cal->prms == NULL) {
		free(cal);
		return NULL;
	}
	cal->block_number = 0;
	return cal;
}

void calibrator_destroy(Calibrator *cal) {
	free(cal->prms);
	free(cal);
}

void calibrator_block(Block *block, Calibrator *cal) {
	float sum = 0;
	for (int i = 0; i < block->count; i++)
		sum += block->sample_a[i] + CONFIG_MARGIN;
	float prms = sqrt(sum / block->count);	// calculo do rms por bloco linear
	cal->prms[cal->block_number] = prms;
	cal->block_number = cal->block_number + 1;
}

float calibrator_calculate(Calibrator *cal) {
	float sum = 0;
	for (int i = 0; i < cal->block_number; i++)
		sum += cal->prms[i];
	return LINEAR_TO_DECIBEL(sum / cal->block_number);
}

static float calibrate(float calibration_delta, float linear) {
	return (LINEAR_TO_DECIBEL(linear) + calibration_delta);
}

#if 0
static float decalibrate(float calibration_delta, float calibrated) {
	return (calibrated + calibration_delta - CONFIG_CALIBRATOR_REFERENCE);
}
#endif

//=============================================================================

Levels *levels_create(unsigned blocks_per_segment) {
	Levels *levels = malloc(sizeof *levels);
	if (levels == NULL)
		return NULL;
	size_t block_data_size = blocks_per_segment * sizeof *levels->LApeak;
	float *buffer = malloc(4 * block_data_size);
	if (buffer == NULL) {
		free(levels);
		return NULL;
	}

	levels->LApeak = buffer;
	levels->LAFmax = buffer += blocks_per_segment;
	levels->LAFmin = buffer += blocks_per_segment;
	levels->LAEsum = buffer += blocks_per_segment;
	levels->block_number = 0;

	size_t segment_data_size = config_struct->record_period * sizeof *levels->LAeq_db;
	buffer = malloc(5 * segment_data_size);
	if (buffer == NULL) {
		free(levels->LApeak);
		free(levels);
		return NULL;
	}
	levels->LAeq_db = buffer;
	levels->LApeak_db = buffer += config_struct->record_period;
	levels->LAFmax_db = buffer += config_struct->record_period;
	levels->LAFmin_db = buffer += config_struct->record_period;
	levels->LAE_db = buffer += config_struct->record_period;

	levels->segment_number = 0;
	return levels;
}

void levels_destroy(Levels *levels) {
	free(levels->LApeak);
	free(levels->LAeq_db);
	free(levels);
}

void process_block_square(Block *block) {
	for (unsigned i = 0; i < block->count; i++)
		block->sample_c[i] = pow(block->sample_b[i], 2);
}

void process_block_lapeak(Block *block, Levels *levels) {
	float peak = block->sample_c[0] + CONFIG_MARGIN;
	for (unsigned i = 1; i < block->count; i++) {
		if (peak < block->sample_c[i] + CONFIG_MARGIN)
			peak = block->sample_c[i] + CONFIG_MARGIN;
	}
	levels->LApeak[levels->block_number] = sqrt(peak);
}

void process_block(Block *block, Levels *levels) {
	float sum = block->sample_d[0] + CONFIG_MARGIN;
	float max = block->sample_d[0] + CONFIG_MARGIN;
	float min = block->sample_d[0] + CONFIG_MARGIN;
	for (unsigned i = 1; i < block->count; i++) {
		if (max < block->sample_d[i] + CONFIG_MARGIN)
			max = block->sample_d[i] + CONFIG_MARGIN;
		if (min > block->sample_d[i] + CONFIG_MARGIN)
			min = block->sample_d[i] + CONFIG_MARGIN;
		sum += block->sample_d[i] + CONFIG_MARGIN;
	}
	levels->LAFmax[levels->block_number] = sqrt(max);
	levels->LAFmin[levels->block_number] = sqrt(min);
	levels->LAEsum[levels->block_number] = sum;
	levels->block_number++;
}

void process_segment(Levels *levels, float calibration_delta) {
	float lae = levels->LAEsum[0];
	float max = levels->LAFmax[0];
	float min = levels->LAFmin[0];
	float peak = levels->LApeak[0];

	for (unsigned i = 1; i < levels->block_number; i++) {
		lae = lae + levels->LAEsum[i];
		if (min > levels->LAFmin[i])
			min = levels->LAFmin[i];
		if (max < levels->LAFmax[i])
			max = levels->LAFmax[i];
		if (peak < levels->LApeak[i])
			peak = levels->LApeak[i];
	}
	lae = lae / (CONFIG_SEGMENT_DURATION * CONFIG_SAMPLE_RATE);
	lae = sqrt(lae);
	float laeq = lae_average(lae);
	levels->LAeq_db[levels->segment_number] = calibrate(calibration_delta, laeq);
	levels->LAFmax_db[levels->segment_number] = calibrate(calibration_delta, max);
	levels->LAFmin_db[levels->segment_number] = calibrate(calibration_delta, min);
	levels->LAE_db[levels->segment_number] = calibrate(calibration_delta, lae);
	levels->LApeak_db[levels->segment_number] = calibrate(calibration_delta, peak);
	levels->segment_number++;
}
