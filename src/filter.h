
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

#ifndef _FILTER_H_
#define _FILTER_H_

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "process.h"

typedef struct {
	float previous; // saves y[n-1]
} Timeweight;

Timeweight *timeweight_create();
void timeweight_destroy(Timeweight *);
//void timeweight_filtering(Block *buff, Timeweight *tw);
void timeweight_filtering(const float *x, float *y, size_t size, Timeweight *tw);

typedef struct {
	float *u;		// variável de estado
	float *coef_a;	// coeficientes do filtro
	float *coef_b;	// coeficientes do filtro
	int N;			// ordem do filtro
} Afilter;

Afilter *aweighting_create(float *coef_a, float *coef_b, int N);
void aweighting_destroy(Afilter *);
void aweighting_filtering(const float *x, float *y, size_t size, Afilter *af);

float *aweighting_get_coef_a(int);
float *aweighting_get_coef_b(int);

#endif
