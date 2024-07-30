
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

#include "FilterCoefs_48000.h"
//#define alfa (1.813894426370144 * pow(10, -4))

//#define alfa 0.00167
#define alfa 0.0001666527785

//Inits time weight filter
Timeweight *timeweight_create()
{
	Timeweight *tw = malloc(sizeof *tw);
	tw->previous = 0;
	return tw;
}

void timeweight_destroy(Timeweight *tw)
{
	free(tw);
}

void timeweight_filtering(Timeweight *tw, float *x, float *y, unsigned n)
{
	// y[n] = (1−α)x[n]+αy[n−1]
	for (unsigned i = 0; i < n; i++)
//		tw->previous = y[i] = ((1 - alfa) * x[i]) + (alfa * tw->previous);
		// y[n] = α * x[n] + (1 - α) * y[n−1]
		tw->previous = y[i] = ((alfa) * x[i]) + ((1 - alfa) * tw->previous);
}

static void shift_right(float u[], int size)
{
	memmove(&u[1], &u[0], (size - 1) * sizeof u[0]);
	u[0] = 0;
}

Afilter *aweighting_create(int N)
{
	Afilter *af = malloc(sizeof *af);
	af->coefs = A_WEIGHTED_taps;
	af->u = calloc(3 * N, sizeof(float));
	af->N = N;
	return af;
}

void aweighting_destroy(Afilter *af)
{
	free(af->u);
	free(af);
}

/*------------------------------------------------------------------------------
		<-- mais recente               mais antigo ->

	      0       1       2       3       4       5
	   ------- ------- ------- ------- ------- -------
	u |       |       |       |       |       |       |
	   ------- ------- ------- ------- ------- -------
	   u1(n)   u1(n-1) u1(n-2) u2(n)   u2(n-1) u2(n-2)


	        0   1   2   3   4   5   6   7   8   9   10  11
	       --- --- --- --- --- --- --- --- --- --- --- ---
	coefs |   |   |   |   |   |   |   |   |   |   |   |   |
	       --- --- --- --- --- --- --- --- --- --- --- ---
	       b10 b11 b12 a10 a11 a12 b20 b21 b22 a20 a21 a22
*/

static float biquad(float x, float *u, const float *a, const float *b)
{
	// ui(n) = xi(n) + ai(0) * ui(n-1) + ai(1) * ui(n-2)
	u[0] = x - a[1] * u[1] - a[2] * u[2];
	// yi(n) = bi(0) * ui(n) + bi(1) * ui(n-1) + bi[2] * ui(n-2)
	return b[0] * u[0] + b[1] * u[1] + b[2] * u[2];
}

static float cascade_biquad(float x, float *u, const float *coefs, int N)
{
	float y = x;
	for (int i = 0; i < N; i++)
		y = biquad(y, u + i * 3, coefs + 3 + i * 6, coefs + i * 6);
	return y;
}

void aweighting_filtering(Afilter *af, float x[], float y[], unsigned size)
{
	for (int n = 0; n < size; n++) {
		shift_right(af->u, af->N);
		shift_right(af->u + 3, af->N);
		shift_right(af->u + 6, af->N);
		float a = cascade_biquad(x[n], af->u, af->coefs, af->N);
		y[n] = a;
//		assert(a >= -1.0 && a <= +1.0);
	}
}
