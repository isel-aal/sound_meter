
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

bool input_device_open(Config *config)
{
	if (config->input_file == NULL)	{
		device.device = DEVICE_SOUND_CARD;
		int result = snd_pcm_open(&device.alsa_handle, config->input_device, SND_PCM_STREAM_CAPTURE, 0);
		if (result < 0) {
			fprintf(stderr, "cannot open audio device %s (%s)\n",
					config->input_device,
					snd_strerror(result));
			return false;
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
			return false;
		}

		result = snd_pcm_prepare(device.alsa_handle);
		if (result < 0) {
			fprintf(stderr, "cannot prepare audio interface for use (%s)\n",
					snd_strerror(result));
			return false;
		}
	}
	else {
		device.device = DEVICE_WAVE;
		device.wave = wave_load(config->input_file);
		if (device.wave == NULL) {
			fprintf(stderr, "Can't load wave file %s\n", config->input_file);
			return false;
		}
		config->sample_rate = wave_get_sample_rate(device.wave);
		config->bits_per_sample = wave_get_bits_per_sample(device.wave);
	}
	return true;
}

size_t input_device_read(void *buffer, size_t nframes)
{
	if (device.device == DEVICE_SOUND_CARD) {
		snd_pcm_sframes_t read_frames = snd_pcm_readi(device.alsa_handle, buffer, nframes);
		if (read_frames < 0) {
			fprintf(stderr, "read from audio interface failed (%s)\n",
					snd_strerror(read_frames));
			return 0;
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
		int result_code = snd_pcm_close(device.alsa_handle);
		if (result_code < 0)
			fprintf(stderr, "Error closing sound card\n");
	}
	else if (device.device == DEVICE_WAVE) {
		wave_destroy(device.wave);
	}
}

//------------------------------------------------------------------------------
//	Output

static char *output_filepath = NULL;

static FILE *output_fd = NULL;

static json_t *output_json;

static int output_index;

static time_t calendar;
static unsigned output_time; //	Tempo decorrido para o ficheiro atual

static void output_new_filename(time_t time);
static void output_file_open(char *filepath);

void output_open(bool continous)
{
	calendar = time(NULL);
	if (continous)
		output_new_filename(0);
	output_file_open(output_filepath);
}

void output_close()
{
	output_file_close();
	free(output_filepath);
}

void output_file_close()
{
	if (strcmp(config_struct->output_format, ".json") == 0) {
		json_dumpf(output_json, output_fd, JSON_REAL_PRECISION(3));
		json_decref(output_json);
	}
	if (output_fd != NULL)
		fclose(output_fd);
	output_fd = NULL;
}

static char *output_init_filename()
{
	size_t date_position = strlen(config_struct->output_path)
		+ strlen(config_struct->output_filename);
	size_t filepath_size = date_position
		+ strlen("AAAAMMDDHHMMSS")
		+ strlen(config_struct->output_format) + 1;
	char *filepath = malloc(filepath_size);
	if (filepath == NULL) {
		fprintf(stderr, "Out of memory\n");
		return NULL;
	}
	strcpy(filepath, config_struct->output_path);
	strcat(filepath, config_struct->output_filename);
	strcat(filepath, "AAAAMMDDHHMMSS");
	strcat(filepath, config_struct->output_format);
	return filepath;
}

static void output_new_filename(time_t time)
{
	calendar += time;
	size_t date_size = strlen("AAAAMMDDHHMMSS");
	char buffer[date_size + 1];
	size_t date_position = strlen(config_struct->output_path)
		+ strlen(config_struct->output_filename);
	strftime(buffer, sizeof buffer, "%Y%m%d%H%M%S", localtime(&calendar));
	for (size_t i = 0; i < date_size; ++i)
		(output_filepath + date_position)[i] = buffer[i];
}

static json_t *LAeq_json;
static json_t *LAE_json;
static json_t *LAFmin_json;
static json_t *LAFmax_json;
static json_t *LApeak_json;

static void output_file_open(char *filepath)
{
	output_fd = fopen(filepath, "w");
	if (output_fd == NULL) {
		fprintf(stderr, "fopen(%s, \"w\") error: %s\n", filepath, strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (strcmp(config_struct->output_format, ".csv") == 0)
		fprintf(output_fd, "LAeq, LAFmin, LAE, LAFmax, LApeak\n");
	else if (strcmp(config_struct->output_format, ".json") == 0) {
	/*
	{
		"ts":xxxxxxxxx,
		"segment": xx,
		"levels": {
			"LAeq": [],
			"LAE": [],
			"LAFmin": [],
			"LAFmax": [],
			"LApeak": []
		}
	}
	*/
	output_json = json_object();
	if (output_json == NULL) {
		fprintf(stderr, "Output: error creating JSON object \"output_json\".\n");
		return;
	}
	json_t *object_json = json_integer(calendar);
	if (object_json != NULL) {
		if (json_object_set_new(output_json, "ts", object_json) != 0) {
			fprintf(stderr, "Output: error adding JSON field \"ts\" ("__FILE__": %d)\n", __LINE__);
			return;
		}
	}
	object_json = json_integer(config_struct->segment_duration);
	if (object_json != NULL) {
		if (json_object_set_new(output_json, "segment", object_json) != 0) {
			fprintf(stderr, "Output: error adding JSON field \"segment\" ("__FILE__": %d)\n", __LINE__);
			return;
		}
	}
	json_t *levels_json = json_object();
	if (levels_json != NULL) {
		if (json_object_set_new(output_json, "levels", levels_json) != 0) {
			fprintf(stderr, "Output: error adding JSON field \"levels\" ("__FILE__": %d)\n", __LINE__);
			return;
		}
	}
	LAeq_json = json_array();
	if (LAeq_json != NULL) {
		if (json_object_set_new(levels_json, "LAeq", LAeq_json) != 0) {
			fprintf(stderr, "Output: error adding JSON field \"LAeq\" ("__FILE__": %d)\n", __LINE__);
			return;
		}
	}
	LAE_json = json_array();
	if (LAE_json != NULL) {
		if (json_object_set_new(levels_json, "LAE", LAE_json) != 0) {
			fprintf(stderr, "Output: error adding JSON field \"LAE\" ("__FILE__": %d)\n", __LINE__);
			return;
		}
	}
	LAFmin_json = json_array();
	if (LAFmin_json != NULL) {
		if (json_object_set_new(levels_json, "LAFmin", LAFmin_json) != 0) {
			fprintf(stderr, "Output: error adding JSON field \"LAFmin\" ("__FILE__": %d)\n", __LINE__);
			return;
		}
	}
	LAFmax_json = json_array();
	if (LAFmax_json != NULL) {
		if (json_object_set_new(levels_json, "LAFmax", LAFmax_json) != 0) {
			fprintf(stderr, "Output: error adding JSON field \"LAFmax\" ("__FILE__": %d)\n", __LINE__);
			return;
		}
	}
	LApeak_json = json_array();
	if (LApeak_json != NULL) {
		if (json_object_set_new(levels_json, "LApeak", LApeak_json) != 0) {
			fprintf(stderr, "Output: error adding JSON field \"LApeak\" ("__FILE__": %d)\n", __LINE__);
			return;
		}
	}
	output_index = 0;
	}
	else {
		fprintf(stderr, "Output: no output format recognized\n");
	}
}

#define JSON_ARRAY_SET(level_array_json, level_value) \
{ \
	json_t *real_json = json_real(level_value); \
	if (real_json == NULL) { \
		fprintf(stderr, "Output: error creating real_json ("__FILE__": %d)\n", __LINE__); \
		return; \
	} \
	if (json_array_append_new(level_array_json, real_json) != 0) { \
		fprintf(stderr, "Output: error set " #level_array_json "[i] ("__FILE__": %d)\n", __LINE__); \
		return; \
	} \
}

void output_record(Levels *levels)
{
	if (strcmp(config_struct->output_format, ".csv") == 0)
	{
	for (unsigned i = 0; i < levels->segment_number; ++i) {
		fprintf(output_fd, "%5.1f, %5.1f, %5.1f, %5.1f, %5.1f\n",
				// fprintf(output_fd, "%5.6f, %5.6f, %5.6f, %5.6f, %5.6f\n",
				levels->LAeq[i], levels->LAFmin[i], levels->LAE[i],
				levels->LAFmax[i], levels->LApeak[i]);
	}
	}
	else if (strcmp(config_struct->output_format, ".json") == 0)
	{
	for (unsigned i = 0; i < levels->segment_number; ++i) {
		JSON_ARRAY_SET(LAeq_json, levels->LAeq[i]);
		JSON_ARRAY_SET(LAE_json, levels->LAE[i]);
		JSON_ARRAY_SET(LAFmin_json, levels->LAFmin[i]);
		JSON_ARRAY_SET(LAFmax_json, levels->LAFmax[i]);
		JSON_ARRAY_SET(LApeak_json, levels->LApeak[i]);
	}
	output_index += levels->segment_number;
	}
	output_time += config_struct->record_period; //	tempo de registo
	fflush(output_fd);
	fsync(fileno(output_fd));
	if (output_time >= config_struct->file_period) { // altura de mudança de ficheiro
		output_file_close();
		output_new_filename(config_struct->segment_duration * output_time);
		output_file_open(output_filepath);
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

static char *concat2(const char *path, const char *filename) {
	char *filepath = malloc(strlen(path) + strlen(filename) + 1);
	if (filepath == NULL) {
		fprintf(stderr, "Out of memory\n");
		return NULL;
	}
	strcpy(filepath, path);
	strcat(filepath, filename);
	return filepath;
}

static char *concat3(const char *path, const char *filename, const char *extention) {
	char *filepath = malloc(strlen(path) + strlen(filename) + strlen(extention) + 1);
	if (filepath == NULL) {
		fprintf(stderr, "Out of memory\n");
		return NULL;
	}
	strcpy(filepath, path);
	strcat(filepath, filename);
	strcat(filepath, extention);
	return filepath;
}

void output_set_filename(const char *option_output_filename, const char *option_input_filename)
{
	if (option_output_filename != NULL) {
		char first_letter = option_output_filename[0];
		if (first_letter != '/' && first_letter != '.') // absoluto / relativo
			output_filepath = concat2(config_struct->output_path, option_output_filename);
		else
			output_filepath = strdup(option_output_filename);
	}
	else if (option_input_filename != NULL) {
		output_filepath = concat3(config_struct->output_path,
					get_filename(option_input_filename),
					config_struct->output_format);
	}
	else {
		output_filepath = output_init_filename();
	}
}

char *output_get_filepath()
{
	return output_filepath;
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
