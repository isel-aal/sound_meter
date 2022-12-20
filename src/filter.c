
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
#include <assert.h>
#include "filter.h"

#define alfa (1.813894426370144 * pow(10, -4))

//#define alfa 0.000167

//Inits time weight filter
Timeweight *timeweight_create(){
	Timeweight *tw = malloc(sizeof *tw);
	tw->previous = 0;
	return tw;
}

void timeweight_destroy(Timeweight *tw) {
	free(tw);
}

void timeweight_filtering(Block *block, Timeweight *tw) {
	// y[n] = (1−α)x[n]+αy[n−1]
	for (unsigned i = 0; i < block->count; i++) {
		block->sample_float[i] = ((1 - alfa) * block->sample_float[i]) + (alfa * tw->previous);
		tw->previous = block->sample_float[i];
	}
}

/*
 u = [1, 2, 3, 4, 5]
 shiftright(u, 5, 50)
 u = [50, 1, 2, 3, 4]
 */

static void shift_right(float *u, int size, float x) {	// insere o valor da posição 0 e shifta tudo pra direita
	memmove(&u[1], &u[0], size * sizeof(float));
	u[0] = x;
}

Afilter *afilter_create(float *coef_a, float *coef_b, int N) {
	Afilter *af = malloc(sizeof *af);
	af->coef_a = coef_a;
	af->coef_b = coef_b;
	af->u = calloc(N + 1, sizeof(float));
	af->N = N;
	return af;
}

void afilter_destroy(Afilter *af) {
	free(af->u);
	free(af);
}

void afilter_filtering(Block *block, Afilter *af) {
	float *y = calloc(block->count, sizeof(float));

	for (int n = 0; n < block->count; n++) {
		// shifta para o lado e u[n] = x[n] => u[0] = x[n]
		shift_right(af->u, af->N, block->sample_float[n] / CONFIG_SAMPLE_NORM);	//	Normalização +1 ... -1

		for (int i = 1; i <= af->N; i++) {
			// u(n) = u(n) + (-1)a(i) * u(n - i)
			af->u[0] = af->u[0] + (((-1) * af->coef_a[i]) * af->u[i]);
		}
		for (int i = 0; i <= af->N; i++) {
			// y(n) = y(n) + b(i) * u(n-i)
			y[n] = y[n] + (af->coef_b[i] * af->u[i]);
		}
	}
	memcpy(block->sample_float, y, block->count * sizeof *block->sample_float);
	free(y);
}

static float coef_a[] = {
	1.00000000000000000,
	-2.12979364760736134,
	0.42996125885751674,
	1.62132698199721426,
	-0.96669962900852902,
	0.00121015844426781,
	0.04400300696788968};

static float coef_b[] = {
	0.169994948147430,
	0.280415310498794,
	-1.120574766348363,
	0.131562559965936,
	0.974153561246036,
	-0.282740857326553,
	-0.152810756202003};

float *afilter_get_coef_a(int sample_rate) {
	assert(sample_rate == 48000);
	return coef_a;
}

float *afilter_get_coef_b(int sample_rate) {
	assert(sample_rate == 48000);
	return coef_b;
}
