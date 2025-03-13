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
#include "sbuffer.h"

static inline float linear_to_decibel(float linear)
{
	return 20.0f * log10(linear / CONFIG_PRESSURE_REFERENCE);
}

static inline float decibel_to_linear(float decibel)
{
	return CONFIG_PRESSURE_REFERENCE * pow(10, decibel / 20.0f);
}

typedef struct {
	unsigned segment_number;
	float *LAeq;
	float *LApeak;
	float *LAFmax;
	float *LAFmin;
	float *LAE;
	int direction;	//	Direção da fonte sonora (0-360 graus)
} Levels;

Levels *levels_create();
void levels_destroy(Levels *);

void process_block_square(float *input, float *output, unsigned length);
void process_segment_lapeak(Levels *levels, struct sbuffer *ring, struct config *config);
void process_segment_levels(Levels *levels, struct sbuffer *ring, struct config *config);
void process_segment_direction(Levels *levels, struct sbuffer *ring[], struct config *config);

void lae_average_create(unsigned laeq_time);	//	Para cálculo de LAeq
void lae_average_destroy();

typedef struct {
	float *prms;
	unsigned block_number;
} Calibrator;

#endif
