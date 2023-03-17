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

#include <stdlib.h>

#include "ring.h"

Ring_float *ring_float_new(size_t size) {
	Ring_float *ring = malloc(sizeof * ring + size * sizeof ring->array[0]);
	if (ring == NULL)
		return NULL;
	ring->size = size;
	ring->put = ring->get = ring->array;
	ring->end = ring->array + ring->size;
	ring->counter = 0;
	return ring;
}

void ring_float_destroy(Ring_float *ring) {
	free(ring);
}

void ring_float_write(Ring_float *ring, float value) {
	*ring->put = value;
	if (ring->put < ring->end)
		ring->put += 1;
	else
		ring->put = ring->array;

	if (ring->counter == ring->size)
		ring->get = ring->put;
	else
		ring->counter++;
}

float ring_float_read(Ring_float *ring) {
	float value = *ring->get;
	if (ring->get < ring->end)
		ring->get += 1;
	else
		ring->get = ring->array;
	ring->counter--;
	return value;
}

int ring_float_empty(Ring_float *ring) {
	return ring->counter == 0;
}

int ring_float_counter(Ring_float *ring) {
	return ring->counter;
}

int ring_float_full(Ring_float *ring) {
	return ring->counter == ring->size;
}