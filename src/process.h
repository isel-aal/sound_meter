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

#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "config.h"

typedef struct {
	unsigned blocks_per_segment;
	unsigned block_size;
	unsigned last_block_size;
	unsigned block_number;		//	Número do bloco no segmento
	unsigned count;				//	Porção de dados úteis no bloco
	int16_t *sample_int16;		//	Amostras em formato inteiro com sinal 16 bits
	float *sample_float;		//	Amostras em formato float
} Block;

Block *block_create(unsigned bps, unsigned size, unsigned size_last);
void block_destroy(Block *);
void block_sample_to_float(Block *);
unsigned block_next_size(Block *);

typedef struct {
	float *prms;
	unsigned block_number;
} Calibrator;

Calibrator *calibrator_create(unsigned blocks_per_segment, unsigned calibration_time);
void calibrator_destroy(Calibrator *);
void calibrator_block(Block *buff, Calibrator *cal);
float calibrator_calculate(Calibrator *cal);

typedef struct {
	unsigned block_number;		//	Número do bloco no segmento corrente
	float *LApeak;				//	Valores calculados para cada bloco no segmento corrente
	float *LAFmax;
	float *LAFmin;
	float *LAE;
	
	unsigned segment_number;
	float LAeq_db[CONFIG_RECORD_PERIOD];	//	Valores calculados para cada segmento, num periodo de registo
	float LApeak_db[CONFIG_RECORD_PERIOD];
	float LAFmax_db[CONFIG_RECORD_PERIOD];
	float LAFmin_db[CONFIG_RECORD_PERIOD];
	float LAE_db[CONFIG_RECORD_PERIOD];
} Levels;

Levels *levels_create(unsigned blocks_per_segment);
void levels_destroy(Levels *);
void process_block_square(Block *buff);							// quadrado de todas as amostras
void process_block_lapeak(Block *buff, Levels *levels);			// cálculo do lapeak
void process_block(Block *buff, Levels *);					// cálculo dos níveis lae,lmax,lmin
void process_segment(Levels *levels, float calibrated_value);	// cálculo dos níveis para 1 segmento

void lae_average_create(unsigned laeq_time);					//	Para cálculo de LAeq
void lae_average_destroy();
#endif
