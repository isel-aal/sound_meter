/*	
Copyright 2022 Laboratório de Audio e Acústica do ISEL

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

#ifndef RING_H
#define RING_H

#include <stddef.h>

typedef struct {
	float *put, *get;
	size_t counter;
	size_t size;
	float *end;
	float array[];
} Ring_float;


Ring_float *ring_float_new(size_t size);
void ring_float_destroy(Ring_float *);

void ring_float_write(Ring_float *ring, float value);
float ring_float_read(Ring_float *ring);
int ring_float_empty(Ring_float *ring);
int ring_float_counter(Ring_float *ring);
int ring_float_full(Ring_float *ring);

#endif
