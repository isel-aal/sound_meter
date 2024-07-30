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
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <jansson.h>
#include "config.h"

void config_destroy(Config *config)
{
	free(config->identification);
	free(config->input_device);
	free(config->output_path);
	if (config->input_file != NULL)
		free(config->input_file);
	free(config);
}

Config *config_load(const char *config_filename)
{
	Config *config = malloc(sizeof *config);
	if (config == NULL) {
		fprintf(stderr, "Out of memory\n");
		return NULL;
	}
	memset(config, 0, sizeof *config);

	FILE *config_fd = fopen(config_filename, "r");
	if (config_fd == NULL) {
		fprintf(stderr, "Configuration file: %s; error: %s\n", config_filename, strerror(errno));
		free(config);
		return NULL;
	}
	json_error_t error;
	json_t *root = json_loadf(config_fd, 0, &error);
	if (root == NULL) {
		fprintf(stderr, "%s: error on line %d: %s\n", config_filename, error.line, error.text);
		fclose(config_fd);
		free(config);
		return NULL;
	}
	json_t *json_identification = json_object_get(root, "identification");
	if (json_identification != NULL && json_is_string(json_identification))
		config->identification = strdup(json_string_value(json_identification));
	else
		config->identification = strdup(CONFIG_IDENTIFICATION);

	json_t *json_input_device = json_object_get(root, "input_device");
	if (json_input_device != NULL && json_is_string(json_input_device))
		config->input_device = strdup(json_string_value(json_input_device));
	else
		config->input_device = strdup(CONFIG_INPUT_DEVICE);

	json_t *json_sample_rate = json_object_get(root, "sample_rate");
	if (json_sample_rate != NULL && json_is_number(json_sample_rate))
		config->sample_rate = json_number_value(json_sample_rate);
	else
		config->sample_rate = CONFIG_SAMPLE_RATE;

	json_t *json_segment_duration = json_object_get(root, "segment_duration");
	if (json_segment_duration != NULL && json_is_number(json_segment_duration))
		config->segment_duration = json_number_value(json_segment_duration);
	else
		config->segment_duration = CONFIG_SEGMENT_DURATION;

	json_t *json_block_size = json_object_get(root, "block_size");
	if (json_block_size != NULL && json_is_number(json_block_size))
		config->block_size = json_number_value(json_block_size);
	else
		config->block_size = CONFIG_BLOCK_SIZE;

	json_t *json_record_period = json_object_get(root, "record_period");
	if (json_record_period != NULL && json_is_number(json_record_period))
		config->record_period = json_number_value(json_record_period);
	else
		config->record_period = CONFIG_RECORD_PERIOD;

	json_t *json_file_period = json_object_get(root, "file_period");
	if (json_file_period != NULL && json_is_number(json_file_period))
		config->file_period = json_number_value(json_file_period);
	else
		config->file_period = CONFIG_FILE_TIME;

	json_t *json_laeq_time = json_object_get(root, "laeq_time");
	if (json_laeq_time != NULL && json_is_number(json_laeq_time))
		config->laeq_time = json_number_value(json_laeq_time);
	else
		config->laeq_time = CONFIG_LAEQ_TIME;

	json_t *json_output_path = json_object_get(root, "output_path");
	if (json_output_path != NULL && json_is_string(json_output_path))
		config->output_path = strdup(json_string_value(json_output_path));
	else
		config->output_path = strdup(CONFIG_OUTPUT_PATH);

	json_t *json_calibration_value = json_object_get(root, "calibration_reference");
	if (json_calibration_value != NULL && json_is_number(json_calibration_value))
		config->calibration_reference = json_number_value(json_calibration_value);
	else
		config->calibration_reference = CONFIG_CALIBRATION_DEFAULT;

	config->output_format = CONFIG_OUTPUT_FORMAT;

	json_decref(root);
	fclose(config_fd);
	return config;
}

void config_save(Config *config, const char *config_filename) {
	json_t *config_json = json_object();
	if (config_json == NULL) {
		fprintf(stderr, "Error in saving configuration - creating JSON object root.\n");
		return;
	}
	/* identification */
	json_t *str = json_string(config->identification);
	if (str == NULL) {
		fprintf(stderr, "Error saving configuration - creating JSON string\n");
		json_decref(config_json);
		return;
	}

	if (json_object_set(config_json, "identification", str) != 0) {
		fprintf(stderr, "Error saving configuration - adding JSON key/value\n");
		json_decref(config_json);
		return;
	}
	json_decref(str);

	/* input_device */
	str = json_string(config->input_device);
	if (str == NULL) {
		fprintf(stderr, "Error saving configuration - creating JSON string\n");
		json_decref(config_json);
		return;
	}

	if (json_object_set(config_json, "input_device", str) != 0) {
		fprintf(stderr, "Error saving configuration - adding JSON key/value\n");
		json_decref(config_json);
		return;
	}
	json_decref(str);

	/* output_path */
	str = json_string(config->output_path);
	if (str == NULL) {
		fprintf(stderr, "Error saving configuration - creating JSON string\n");
		json_decref(config_json);
		return;
	}

	if (json_object_set(config_json, "output_path", str) != 0) {
		fprintf(stderr, "Error saving configuration - adding JSON key/value\n");
		json_decref(config_json);
		return;
	}
	json_decref(str);

	/* output_format */
	str = json_string(config->output_format);
	if (str == NULL) {
		fprintf(stderr, "Error saving configuration - creating JSON string\n");
		json_decref(config_json);
		return;
	}

	if (json_object_set(config_json, "output_format", str) != 0) {
		fprintf(stderr, "Error saving configuration - adding JSON key/value\n");
		json_decref(config_json);
		return;
	}
	json_decref(str);

	/* sample_rate */
	json_t *number = json_integer(config->sample_rate);
	if (str == NULL) {
		fprintf(stderr, "Error saving configuration - creating JSON string\n");
		json_decref(config_json);
		return;
	}

	if (json_object_set(config_json, "sample_rate", number) != 0) {
		fprintf(stderr, "Error saving configuration - adding JSON key/value\n");
		json_decref(config_json);
		return;
	}
	json_decref(number);

	/* segment_duration */
	number = json_integer(config->segment_duration);
	if (str == NULL) {
		fprintf(stderr, "Error saving configuration - creating JSON string\n");
		json_decref(config_json);
		return;
	}

	if (json_object_set(config_json, "segment_duration", number) != 0) {
		fprintf(stderr, "Error saving configuration - adding JSON key/value\n");
		json_decref(config_json);
		return;
	}
	json_decref(number);

	/* block_size */
	number = json_integer(config->block_size);
	if (str == NULL) {
		fprintf(stderr, "Error saving configuration - creating JSON string\n");
		json_decref(config_json);
		return;
	}

	if (json_object_set(config_json, "block_size", number) != 0) {
		fprintf(stderr, "Error saving configuration - adding JSON key/value\n");
		json_decref(config_json);
		return;
	}
	json_decref(number);

	/* record_period */
	number = json_integer(config->record_period);
	if (str == NULL) {
		fprintf(stderr, "Error saving configuration - creating JSON string\n");
		json_decref(config_json);
		return;
	}

	if (json_object_set(config_json, "record_period", number) != 0) {
		fprintf(stderr, "Error saving configuration - adding JSON key/value\n");
		json_decref(config_json);
		return;
	}
	json_decref(number);

	/* file_period */
	number = json_integer(config->file_period);
	if (str == NULL) {
		fprintf(stderr, "Error saving configuration - creating JSON string\n");
		json_decref(config_json);
		return;
	}

	if (json_object_set(config_json, "file_period", number) != 0) {
		fprintf(stderr, "Error saving configuration - adding JSON key/value\n");
		json_decref(config_json);
		return;
	}
	json_decref(number);

	/* calibration_reference */
	number = json_integer(config->calibration_reference);
	if (str == NULL) {
		fprintf(stderr, "Error saving configuration - creating JSON string\n");
		json_decref(config_json);
		return;
	}

	if (json_object_set(config_json, "calibration_reference", number) != 0) {
		fprintf(stderr, "Error saving configuration - adding JSON key/value\n");
		json_decref(config_json);
		return;
	}
	json_decref(number);

	if (json_dump_file (config_json, config_filename, JSON_INDENT(8)) != 0) {
		fprintf(stderr, "Error saving configuration (JSON format). File: %s\n",
				config_filename);
	}
	json_decref(config_json);
}

Config *config_struct;
