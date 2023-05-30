
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

#include <alsa/asoundlib.h>
#include <jansson.h>
#include "config.h"
#include "in_out.h"

static Input_device device;

extern int sample_rate;

int input_device_open(const char *file_name, const char *device_name, Config *config) {
	if (file_name == NULL) {
		device.device = DEVICE_SOUND_CARD;

		int result = snd_pcm_open(&device.alsa_handle, device_name, SND_PCM_STREAM_CAPTURE, 0);
		if (result < 0) {
			fprintf (stderr, "cannot open audio device %s (%s)\n",
							device_name,
							snd_strerror (result));
							exit(EXIT_FAILURE);
		}
		result = snd_pcm_set_params(device.alsa_handle,
									CONFIG_PCM_FORMAT,      /* mudar */
									SND_PCM_ACCESS_RW_INTERLEAVED,
									1,
									config->sample_rate,     /* configurar a partir de variável */
									1,
									500000);   /* 0.5 sec */
		if (result < 0) {
			fprintf(stderr, "snd_pcm_set_params: %s\n", snd_strerror(result));
			exit(EXIT_FAILURE);
		}

		result = snd_pcm_prepare(device.alsa_handle);
		if (result < 0) {
			fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
							snd_strerror(result));
			exit(EXIT_FAILURE);
		}
	}
	else {
		device.device = DEVICE_WAVE;
		device.wave = wave_load(file_name);
		if (device.wave == NULL) {
			fprintf (stderr, "Can't load wave file %s\n", file_name);
			exit(EXIT_FAILURE);
		}
		config->sample_rate = wave_get_sample_rate(device.wave);
	}
	return 0;
}

size_t input_device_read(void *buffer, size_t nframes) {
	if (device.device == DEVICE_SOUND_CARD) {
		snd_pcm_sframes_t read_frames = snd_pcm_readi(device.alsa_handle, buffer, nframes);
		if (read_frames < 0) {
			fprintf (stderr, "read from audio interface failed (%s)\n",
					snd_strerror(read_frames));
			exit(EXIT_FAILURE);
		}
		return read_frames;
	}
	else if (device.device == DEVICE_WAVE) {
		return wave_read_samples(device.wave, buffer, nframes);
	}
	return 0;
}

void input_device_close() {
	if (device.device == DEVICE_SOUND_CARD) {
		snd_pcm_close (device.alsa_handle);
	}
	else if (device.device == DEVICE_WAVE) {
		wave_destroy(device.wave);
	}
}

//------------------------------------------------------------------------------
//	Output

enum {OUTPUT_FORMAT_JSON, OUTPUT_FORMAT_CSV };
//static int output_format = OUTPUT_FORMAT_JSON;
static char *output_filename = NULL;
static FILE *output_fd = NULL;

void output_file_close() {
#if 0
	if (output_format == OUTPUT_FORMAT_JSON)
		json_dumpf(root, output_fd, 0);
#endif
	free(output_filename);
	if (output_fd != NULL)
		fclose(output_fd);
	output_fd = NULL;
}

#if 0
static char *output_new_filename(const char *base_filename, int suffix) {
	char *filename = malloc(strlen(base_filename) + 3 + 1);
	strcpy(filename, base_filename);
	if (filename == NULL) {
		fprintf(stderr, "Out of memory\n");
		exit(-1);
	}
	char *end = filename + strlen(filename);
	char *point = strrchr(filename, '.');
	memmove(point + 3, point, end - point + 1);
	snprintf(point, 4, "%03d", suffix);
	*(point + 3) = '.';
	return filename;
}
#endif

static unsigned output_time;		//	Tempo de corrido para o ficheiro atual
static time_t calendar;

static char *output_new_filename(unsigned time) {
	size_t filename_size = strlen("AAAAMMDDHHMMSS") + strlen(config_struct->output_extention) + 1;
	char *filename = malloc(filename_size);
	if (filename == NULL) {
		fprintf(stderr, "Out of memory\n");
		exit (EXIT_FAILURE);
	}
	calendar += time;
	strftime(filename, filename_size, "%Y%m%d%H%M%S", localtime(&calendar));
	strcat(filename, config_struct->output_extention);
	return filename;
}

static void output_file_open(char *filename) {
	size_t filepath_size = strlen(config_struct->output_path) + strlen(filename) + 1;
	char *filepath = malloc(filepath_size);
	if (filepath == NULL) {
		fprintf(stderr, "Out of memory\n");
		exit (EXIT_FAILURE);
	}
	strcpy(filepath, config_struct->output_path);
	strcat(filepath, filename);
	output_fd = fopen(filepath, "w");
	if (output_fd == NULL) {
		fprintf(stderr, "fopen(%s, \"w\") error: %s\n", filepath, strerror(errno));
		exit(EXIT_FAILURE);
	}
	free(filepath);
	fprintf(output_fd, "LAE, LApeak, LAFmax, LAFmin, LAeq\n");
	output_time = 0;
}

void output_record(Levels *levels) {
	if (levels->segment_number == 0)
		return;
	if (output_fd == NULL)
		output_file_open(output_filename);
	for (unsigned i = 0; i < levels->segment_number; ++i) {
		fprintf(output_fd, "%5.1f, %5.1f, %5.1f, %5.1f, %5.1f\n",
			levels->LAE_db[i], levels->LApeak_db[i],
				levels->LAFmax_db[i], levels->LAFmin_db[i],
				levels->LAeq_db[i]);

	}
	output_time += config_struct->record_period;
	fflush(output_fd);
	fsync(fileno(output_fd));
	if (output_time >= config_struct->file_period) {
		output_file_close();
		output_filename = output_new_filename(config_struct->segment_duration * output_time);
//		output_file_open(output_filename);
		output_time = 0;
	}
}

static const char *strip_filename(const char *filename) {
	const char *ptr = strrchr(filename, '/');
	return ptr != NULL ? ptr + 1 : filename;
}

void output_set_filename(const char *filename, const char *extension) {
	const char *name = strip_filename(filename);
	output_filename = malloc(strlen(name) + strlen(extension) + 1);
	if (output_filename == NULL) {
		fprintf(stderr, "Out of memory\n");
		exit (EXIT_FAILURE);
	}
	strcpy(output_filename, name);
	strcat(output_filename, extension);
}

char *output_get_filename() {
	return output_filename;
}

void output_init(int continous) {
	if (continous != 0) {
		calendar = time(NULL);
		output_filename = output_new_filename(0);
//		output_file_open(output_filename);
	}
	else
		output_file_open(output_filename);
}
