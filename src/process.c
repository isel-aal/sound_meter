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
	Block *block = malloc(sizeof *block);
	if (block == NULL)
		return NULL;
	block->sample_int16 = malloc(block_size * sizeof block->sample_int16[0]);
	if (block->sample_int16 == NULL) {
		free(block);
		return NULL;
	}
	block->sample_float = malloc(block_size * sizeof block->sample_float[0]);
	if (block->sample_float == NULL) {
		free(block);
		free(block->sample_int16);
		return NULL;
	}
	block->blocks_per_segment = blocks_per_segment;
	block->block_size = block_size;
	block->last_block_size = last_block_size;
	block->count = 0;
	block->block_number = 0;
	return block;
}

void block_destroy(Block *block) {
	free(block->sample_int16);
	free(block->sample_float);
	free(block);
}

void block_sample_to_float(Block *block) {
	for (unsigned i = 0; i < block->count; i++)
		block->sample_float[i] = block->sample_int16[i];
}

unsigned block_next_size(Block *block) {
	if (block->block_number == block->blocks_per_segment - 1)	//	Último bloco do segmento?
		return block->last_block_size;
	else
		return block->block_size;
}

//=============================================================================

static Ring_float *laeq_ring;
static double laeq_accumulator;

/**
 * lae_average_create:
 * Inicializar o cálculo de LAEq
 */
void lae_average_create(unsigned laeq_time) {
	laeq_ring = ring_float_new(laeq_time);
	laeq_accumulator = 0;
}

void lae_average_destroy() {
	ring_float_destroy(laeq_ring);
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
	if (ring_float_full(laeq_ring))
		laeq_accumulator -= ring_float_read(laeq_ring);
	laeq_accumulator += lae;
	ring_float_write(laeq_ring, lae);
	return laeq_accumulator / ring_float_counter(laeq_ring);
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
		sum += block->sample_float[i] + CONFIG_MARGIN;
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

static float calibrate(float calibrated_value, float linear) {	// calibrar um valor linear
	return (LINEAR_TO_DECIBEL(linear) + CONFIG_CALIBRATOR_REFERENCE - calibrated_value);
}

#if 0
static float decalibrate(float calibrated_value, float calibrated) {	
	return (calibrated + calibrated_value - CONFIG_CALIBRATOR_REFERENCE);
}
#endif

//=============================================================================

Levels *levels_create(unsigned blocks_per_segment) {
	Levels *levels = malloc(sizeof *levels);
	if (levels == NULL)
		return NULL;
	levels->LApeak = malloc(blocks_per_segment * sizeof *levels->LApeak);
	if (levels->LApeak == NULL) {
		free(levels);
		return NULL;
	}
	levels->LAFmax = malloc(blocks_per_segment * sizeof *levels->LApeak);
	if (levels->LAFmax == NULL) {
		free(levels);
		free(levels->LApeak);
		return NULL;
	}
	levels->LAFmin = malloc(blocks_per_segment * sizeof *levels->LApeak);
	if (levels->LAFmin == NULL) {
		free(levels);
		free(levels->LApeak);
		free(levels->LAFmax);
		return NULL;
	}
	levels->LAE = malloc(blocks_per_segment * sizeof *levels->LApeak);
	if (levels->LAE == NULL) {
		free(levels);
		free(levels->LApeak);
		free(levels->LAFmax);
		free(levels->LAFmin);
		return NULL;
	}
	levels->segment_number = 0;
	levels->block_number = 0;
	return levels;
}

void levels_destroy(Levels *levels) {
	free(levels->LApeak);
	free(levels->LAFmax);
	free(levels->LAFmin);
	free(levels->LAE);
	free(levels);
}

void process_block_square(Block *block) {
	for (unsigned i = 0; i < block->count; i++)
		block->sample_float[i] = pow(block->sample_float[i], 2);
}

void process_block_lapeak(Block *block, Levels *levels) {
	float peak = block->sample_float[0] + CONFIG_MARGIN;
	for (unsigned i = 1; i < block->count; i++) {
		if (peak < block->sample_float[i] + CONFIG_MARGIN)
			peak = block->sample_float[i] + CONFIG_MARGIN;
	}
	levels->LApeak[levels->block_number] = sqrt(peak);;
}

void process_block(Block *block, Levels *levels) {
	float sum = block->sample_float[0] + CONFIG_MARGIN;
	float max = block->sample_float[0] + CONFIG_MARGIN;
	float min = block->sample_float[0] + CONFIG_MARGIN;
	for (unsigned i = 1; i < block->count; i++) {
		if (max < block->sample_float[i] + CONFIG_MARGIN)
			max = block->sample_float[i] + CONFIG_MARGIN;
		if (min > block->sample_float[i] + CONFIG_MARGIN)
			min = block->sample_float[i] + CONFIG_MARGIN;
		sum += block->sample_float[i] + CONFIG_MARGIN;
	}
	levels->LAFmax[levels->block_number] = sqrt(max);
	levels->LAFmin[levels->block_number] = sqrt(min);
	levels->LAE[levels->block_number] = sqrt(sum / block->count);
	levels->block_number++;
}

void process_segment(Levels *levels, float calibrated_value) {
	float lae = levels->LAE[0];
	float max = levels->LAFmin[0];
	float min = levels->LAFmax[0];
	float peak = levels->LApeak[0];

	for (unsigned i = 1; i < levels->block_number; i++) {
		lae = lae + levels->LAE[i];
		if (min > levels->LAFmin[i])
			min = levels->LAFmin[i];
		if (max < levels->LAFmax[i])
			max = levels->LAFmax[i];
		if (peak < levels->LApeak[i])
			peak = levels->LApeak[i];
	}
	lae = lae / levels->block_number;
	float laeq = lae_average(lae);
	levels->LAeq_db[levels->segment_number] = calibrate(calibrated_value, laeq);
	levels->LAFmax_db[levels->segment_number] = calibrate(calibrated_value, max);
	levels->LAFmin_db[levels->segment_number] = calibrate(calibrated_value, min);
	levels->LAE_db[levels->segment_number] = calibrate(calibrated_value, lae);
	levels->LApeak_db[levels->segment_number] = calibrate(calibrated_value, peak);
	levels->segment_number++;
}
