
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

int input_device_open(Config *config)
{
	if (config->input_file == NULL)	{
		device.device = DEVICE_SOUND_CARD;
		int result = snd_pcm_open(&device.alsa_handle, config->input_device, SND_PCM_STREAM_CAPTURE, 0);
		if (result < 0) {
			fprintf(stderr, "cannot open audio device %s (%s)\n",
					config->input_device,
					snd_strerror(result));
			exit(EXIT_FAILURE);
		}
		result = snd_pcm_set_params(device.alsa_handle,
					CONFIG_PCM_FORMAT, /* mudar */
					SND_PCM_ACCESS_RW_INTERLEAVED,
					1,
					config->sample_rate,
					1,
					500000); /* 0.5 sec */
		if (result < 0) {
			fprintf(stderr, "snd_pcm_set_params: %s\n", snd_strerror(result));
			exit(EXIT_FAILURE);
		}

		result = snd_pcm_prepare(device.alsa_handle);
		if (result < 0) {
			fprintf(stderr, "cannot prepare audio interface for use (%s)\n",
					snd_strerror(result));
			exit(EXIT_FAILURE);
		}
	}
	else {
		device.device = DEVICE_WAVE;
		device.wave = wave_load(config->input_file);
		if (device.wave == NULL) {
			fprintf(stderr, "Can't load wave file %s\n", config->input_file);
			exit(EXIT_FAILURE);
		}
		config->sample_rate = wave_get_sample_rate(device.wave);
		config->bits_per_sample = wave_get_bits_per_sample(device.wave);
	}
	return 0;
}

size_t input_device_read(void *buffer, size_t nframes)
{
	if (device.device == DEVICE_SOUND_CARD) {
		snd_pcm_sframes_t read_frames = snd_pcm_readi(device.alsa_handle, buffer, nframes);
		if (read_frames < 0) {
			fprintf(stderr, "read from audio interface failed (%s)\n",
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

void input_device_close()
{
	if (device.device == DEVICE_SOUND_CARD) {
		snd_pcm_close(device.alsa_handle);
	}
	else if (device.device == DEVICE_WAVE) {
		wave_destroy(device.wave);
	}
}

//------------------------------------------------------------------------------
//	Output

enum
{
	OUTPUT_FORMAT_JSON,
	OUTPUT_FORMAT_CSV
};
// static int output_format = OUTPUT_FORMAT_JSON;
static char *output_filename = NULL;
static FILE *output_fd = NULL;

void output_file_close()
{
#if 0
	if (output_format == OUTPUT_FORMAT_JSON)
		json_dumpf(root, output_fd, 0);
#endif
	free(output_filename);
	if (output_fd != NULL)
		fclose(output_fd);
	output_fd = NULL;
}

static unsigned output_time; //	Tempo decorrido para o ficheiro atual
static time_t calendar;

static char *output_new_filename(unsigned time)
{
	size_t filename_size = strlen("AAAAMMDDHHMMSS") + strlen(config_struct->output_format) + 1;
	char *filename = malloc(filename_size);
	if (filename == NULL) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}
	calendar += time;
	strftime(filename, filename_size, "%Y%m%d%H%M%S", localtime(&calendar));
	strcat(filename, config_struct->output_format);
	return filename;
}

static void output_file_open(char *filename)
{
	size_t filepath_size = strlen(config_struct->output_path) + strlen(filename) + 1;
	char *filepath = malloc(filepath_size);
	if (filepath == NULL) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}
	strcpy(filepath, config_struct->output_path);
	strcat(filepath, filename);
	output_fd = fopen(filepath, "w");
	if (output_fd == NULL) {
		fprintf(stderr, "fopen(%s, \"w\") error: %s\n", filepath, strerror(errno));
		exit(EXIT_FAILURE);
	}
	free(filepath);
	fprintf(output_fd, "LAeq, LAFmin, LAE, LAFmax, LApeak\n");
}

void output_record(Levels *levels)
{
	for (unsigned i = 0; i < levels->segment_number; ++i) {
		fprintf(output_fd, "%5.1f, %5.1f, %5.1f, %5.1f, %5.1f\n",
				// fprintf(output_fd, "%5.6f, %5.6f, %5.6f, %5.6f, %5.6f\n",
				levels->LAeq[i], levels->LAFmin[i], levels->LAE[i],
				levels->LAFmax[i], levels->LApeak[i]);
	}
	output_time += config_struct->record_period; //	tempo de registo
	fflush(output_fd);
	fsync(fileno(output_fd));
	if (output_time >= config_struct->file_period) { // altura de mudança de ficheiro
		output_file_close();
		output_filename = output_new_filename(config_struct->segment_duration * output_time);
		output_file_open(output_filename);
		output_time = 0;
	}
}

static const char *get_filename(const char *filename)
{
	const char *ptr = strrchr(filename, '/');
	return ptr != NULL ? ptr + 1 : filename;
}

static const char *get_extention(const char *filename)
{
	const char *ptr = strrchr(filename, '.');
	return ptr != NULL ? ptr + 1 : NULL;
}

static char *get_stem(const char *fullname)
{
	const char *point = strrchr(fullname, '.');
	if (point == NULL)
		point = fullname + strlen(fullname);
	const char *slash = strrchr(fullname, '/');
	if (slash == NULL)
		slash = fullname;
	else
		slash += 1;
	size_t stem_size = point - slash;

	char *stem = malloc(stem_size + 1);
	if (stem == NULL)
		return NULL;
	memcpy(stem, slash, stem_size);
	*(stem + stem_size) = '\0';
	return stem;
}

void output_set_filename(const char *fullpath, const char *extension)
{
	const char *filename = get_filename(fullpath);
	output_filename = malloc(strlen(filename) + strlen(extension) + 1);
	if (output_filename == NULL) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}
	strcpy(output_filename, filename);
	strcat(output_filename, extension);
}

char *output_get_filename()
{
	return output_filename;
}

void output_init(int continous)
{
	if (continous) {
		calendar = time(NULL);
		output_filename = output_new_filename(0);
	}
	output_file_open(output_filename);
	atexit(output_file_close);
}

//------------------------------------------------------------------------------

Audit *audit_create(char *id)
{
	Audit *audit = malloc(sizeof *audit);
	if (audit == NULL)
		return NULL;
	audit->id = id;
	audit->wave = wave_create(config_struct->bits_per_sample, 1);
	wave_set_sample_rate(audit->wave, config_struct->sample_rate);
	return audit;
}

static char *audit_make_filename(Config *config, char *id)
{
	const char *extention = get_extention(config->input_file);
	char *stem = get_stem(config->input_file);
	if (stem == NULL)
		return NULL;
	size_t filepath_size = strlen(config->output_path) + strlen(stem) + 1 + strlen(id) + 1 + strlen(extention) + 1;
	char *filepath = malloc(filepath_size);
	if (filepath == NULL) {
		free(stem);
		return NULL;
	}
	strcpy(filepath, config->output_path);
	strcat(filepath, stem);
	free(stem);
	strcat(filepath, ".");
	strcat(filepath, id);
	strcat(filepath, ".");
	strcat(filepath, extention);
	return filepath;
}

int audit_append_samples(Audit *audit, float *data, unsigned data_size)
{
	int16_t *buffer = malloc(data_size * sizeof *buffer);
	if (buffer == NULL)
		return EXIT_FAILURE;
	samples_float_to_int16(data, buffer, data_size);
	size_t wrote_frames = wave_append_samples(audit->wave, (char *)buffer, data_size);
	free(buffer);
	return wrote_frames == data_size;
}

void audit_destroy(Audit *audit)
{
	wave_format_update(audit->wave);
	char *filename = audit_make_filename(config_struct, audit->id);
	wave_store(audit->wave, filename);
	free(filename);
	wave_destroy(audit->wave);
	free(audit);
}
