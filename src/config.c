#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <jansson.h>
#include "config.h"

void config_destroy(Config *config) {
	free(config->identification);
	free(config->input_device);
	free(config->output_path);
	free(config);
}

Config *config_load(const char *config_filename) {
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

	json_t *json_calibration_time = json_object_get(root, "calibration_time");
	if (json_calibration_time != NULL && json_is_number(json_calibration_time))
		config->calibration_time = json_number_value(json_calibration_time);
	else
		config->calibration_time = CONFIG_CALIBRATION_TIME;

	json_t *json_calibration_value = json_object_get(root, "calibration_reference");
	if (json_calibration_value != NULL && json_is_number(json_calibration_value))
		config->calibration_reference = json_number_value(json_calibration_value);
	else
		config->calibration_reference = CONFIG_CALIBRATION_DEFAULT;
	
	config->output_extention = CONFIG_OUTPUT_FORMAT;

	json_decref(root);
	fclose(config_fd);
	return config;
}

Config *config_struct;
