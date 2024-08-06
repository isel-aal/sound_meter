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

#ifndef INPUT_H
#define INPUT_H

#include <alsa/asoundlib.h>
#include <wave.h>
#include "config.h"
#include "process.h"

typedef struct input_device {
	enum {DEVICE_WAVE, DEVICE_SOUND_CARD} device;
	union {
		Wave *wave;
		snd_pcm_t *alsa_handle;
	};
} Input_device;

bool input_device_open(Config *);
size_t input_device_read(void *buffer, size_t frames);
void input_device_close();

void output_open(bool);
void output_close();

void output_set_filename(const char *filename, const char *extension);
char *output_get_filepath();
void output_record(Levels *levels);
void output_file_close();


typedef struct audit {
	char *id;
	Wave *wave;
} Audit;

Audit *audit_create(char *id);
int audit_append_samples(Audit *audit, float *block, unsigned length);
void audit_destroy(Audit *audit);

 #endif
