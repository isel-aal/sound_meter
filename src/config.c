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

static json_t *config_json;

static struct config config = {
	.identification = CONFIG_IDENTIFICATION,
	.input_device = CONFIG_INPUT_DEVICE,
	.output_path = CONFIG_OUTPUT_PATH,
	.output_filename = CONFIG_OUTPUT_FILENAME,
	.output_format = CONFIG_OUTPUT_FORMAT,
	.sample_rate = CONFIG_SAMPLE_RATE,
	.bits_per_sample = CONFIG_BITS_PER_SAMPLE,
	.block_size = CONFIG_BLOCK_SIZE,
	.segment_duration = CONFIG_SEGMENT_DURATION,
	.record_period = CONFIG_RECORD_PERIOD,
	.file_period = CONFIG_FILE_PERIOD,
	.laeq_time = CONFIG_LAEQ_TIME,
	.calibration_reference = CONFIG_CALIBRATION_REFERENCE,
	.mqtt_enable = CONFIG_MQTT_ENABLE,
	.mqtt_broker = CONFIG_MQTT_BROKER,
	.mqtt_topic = CONFIG_MQTT_TOPIC,
	.mqtt_qos = CONFIG_MQTT_QOS,
	.mqtt_device_credential = CONFIG_MQTT_DEVICE_CREDENTIAL,
	.server_socket = CONFIG_SERVER_SOCKET,
};

void config_print() 
{
	printf("Program configuration:\n"
		"\tIdentification: %s\n"
		"\tInput device: %s\n"
		"\tInput file: %s\n"
		"\tOutput path: %s\n"
		"\tOutput file: %s\n"
		"\tOutput format: %s\n"
		"\tSample Rate: %d\n"
		"\tBits per sample: %d\n"
		"\tBlock size: %d samples\n"
		"\tSegment duration: %d seconds\n"
		"\tSegment size: %d samples\n"
		"\tRecord period: %d segments\n"
		"\tFile period: %d segments\n"
		"\tCalibration time: %d\n"
		"\tCalibration reference: %.1f db\n"
		"\tMQTT: %s\n"
		"\tMQTT Broker: %s\n"
		"\tMQTT Topic: %s\n"
		"\tMQTT qos: %d\n"
		"\tMQTT device credential: %s\n"
		"\tServer socket: %s\n",
		config_struct->identification,
		config_struct->input_device,
		config_struct->input_file,
		config_struct->output_path,
		config_struct->output_filename,
		config_struct->output_format,
		config_struct->sample_rate,
		config_struct->bits_per_sample,
		config_struct->block_size,
		config_struct->segment_duration,
		config_struct->segment_size,
		config_struct->record_period,
		config_struct->file_period,
		config_struct->calibration_time,
		config_struct->calibration_reference,
		config_struct->mqtt_enable? "enabled" : "disabled",	
		config_struct->mqtt_broker,
		config_struct->mqtt_topic,
		config_struct->mqtt_qos,
		config_struct->mqtt_device_credential,
		config_struct->server_socket
		);
}

Config *config_struct = &config;

#define CONFIG_UPDATE_FROM_JSON_STRING(key) \
{ \
	json_t *json_object = json_object_get(config_json, #key); \
	if (json_object != NULL && json_is_string(json_object)) \
		config->key = json_string_value(json_object); \
	else \
		fprintf(stderr, "Config: error updating config_struct ("__FILE__": %d)\n", __LINE__); \
}

#define CONFIG_UPDATE_FROM_JSON_INTEGER(key) \
{ \
	json_t *json_object = json_object_get(config_json, #key); \
	if (json_object != NULL && json_is_integer(json_object)) \
		config->key = json_integer_value(json_object); \
	else \
		fprintf(stderr, "Config: error updating config_struct ("__FILE__": %d)\n", __LINE__); \
}

#define CONFIG_UPDATE_FROM_JSON_REAL(key) \
{ \
	json_t *json_object = json_object_get(config_json, #key); \
	if (json_object != NULL && json_is_real(json_object)) \
		config->key = json_real_value(json_object); \
	else \
		fprintf(stderr, "Config: error updating config_struct ("__FILE__": %d)\n", __LINE__); \
}

#define CONFIG_UPDATE_FROM_JSON_BOOL(key) \
{ \
	json_t *json_object = json_object_get(config_json, #key); \
	if (json_object != NULL && json_is_boolean(json_object)) \
		config->key = json_is_true(json_object); \
	else \
		fprintf(stderr, "Config: error updating config_struct ("__FILE__": %d)\n", __LINE__); \
}

static void config_update_from_json(struct config *config, json_t *config_json) 
{
	CONFIG_UPDATE_FROM_JSON_STRING(identification);
	CONFIG_UPDATE_FROM_JSON_STRING(input_device);
	CONFIG_UPDATE_FROM_JSON_STRING(output_path);
	CONFIG_UPDATE_FROM_JSON_STRING(output_filename);
	CONFIG_UPDATE_FROM_JSON_STRING(output_format);

	CONFIG_UPDATE_FROM_JSON_INTEGER(sample_rate);
	CONFIG_UPDATE_FROM_JSON_INTEGER(bits_per_sample);
	CONFIG_UPDATE_FROM_JSON_INTEGER(block_size);
	CONFIG_UPDATE_FROM_JSON_INTEGER(segment_duration);
	CONFIG_UPDATE_FROM_JSON_INTEGER(record_period);
	CONFIG_UPDATE_FROM_JSON_INTEGER(file_period);
	CONFIG_UPDATE_FROM_JSON_INTEGER(laeq_time);

	CONFIG_UPDATE_FROM_JSON_REAL(calibration_reference);

	CONFIG_UPDATE_FROM_JSON_BOOL(mqtt_enable);
	CONFIG_UPDATE_FROM_JSON_STRING(mqtt_broker);
	CONFIG_UPDATE_FROM_JSON_STRING(mqtt_topic);
	CONFIG_UPDATE_FROM_JSON_INTEGER(mqtt_qos);
	CONFIG_UPDATE_FROM_JSON_STRING(mqtt_device_credential);
	CONFIG_UPDATE_FROM_JSON_STRING(server_socket);
}

#define	CONFIG_UPDATE_TO_JSON_STRING(config_struct, config_json, key) \
{	\
	if (config_struct->key != NULL) { \
		json_t *string_json = json_object_get(config_json, #key); \
		if (string_json != NULL) { \
			if (json_string_set(string_json, config_struct->key) != 0) \
				fprintf(stderr, "Config: error set json string ("__FILE__": %d)\n", __LINE__); \
			else \
				config->key = json_string_value(string_json); \
		} \
		else { \
			string_json = json_string(config->key); \
			if (string_json != NULL) { \
				if (json_object_set_new(config_json, #key, string_json) != 0) \
					fprintf(stderr, "Config: error set json string ("__FILE__": %d)\n", __LINE__); \
			} \
			else { \
				fprintf(stderr, "Config: error creating json string ("__FILE__": %d)\n", __LINE__); \
			} \
		} \
	} \
}

#define	CONFIG_UPDATE_TO_JSON(type, config_struct, config_json, key) \
{	\
	json_t *json_object = json_object_get(config_json, #key); \
	if (json_object != NULL) { \
		if (json_##type##_set(json_object, config_struct->key) == 0) \
			config->key = json_##type##_value(json_object); \
		else \
			fprintf(stderr, "Config: error set json integer ("__FILE__": %d)\n", __LINE__); \
	} \
	else { \
		json_object = json_##type(config->key); \
		if (json_object != NULL) { \
			if (json_object_set_new(config_json, #key, json_object) != 0) \
				fprintf(stderr, "Config: error set json " #type " ("__FILE__": %d)\n", __LINE__); \
		} \
		else { \
			fprintf(stderr, "Config: error creating json " #type " ("__FILE__": %d)\n", __LINE__); \
		} \
	} \
}

#define	CONFIG_UPDATE_TO_JSON_INTEGER(config_struct, config_json, key) \
{	\
	json_t *integer_json = json_object_get(config_json, #key); \
	if (integer_json != NULL) { \
		if (json_integer_set(integer_json, config_struct->key) != 0) \
			fprintf(stderr, "Config: error set json integer ("__FILE__": %d)\n", __LINE__); \
		else \
			config->key = json_integer_value(integer_json); \
	} \
	else { \
		integer_json = json_integer(config->key); \
		if (integer_json != NULL) { \
			if (json_object_set_new(config_json, #key, integer_json) != 0) \
				fprintf(stderr, "Config: error set json integer ("__FILE__": %d)\n", __LINE__); \
		} \
		else { \
			fprintf(stderr, "Config: error creating json integer ("__FILE__": %d)\n", __LINE__); \
		} \
	} \
}

#define	CONFIG_UPDATE_TO_JSON_REAL(config_struct, config_json, key) \
{	\
	json_t *real_json = json_object_get(config_json, #key); \
	if (real_json != NULL) { \
		if (json_real_set(real_json, config_struct->key) != 0) \
			fprintf(stderr, "Config: error set json real ("__FILE__": %d)\n", __LINE__); \
		else \
			config->key = json_real_value(real_json); \
	} \
	else { \
		real_json = json_real(config->key); \
		if (real_json != NULL) { \
			if (json_object_set_new(config_json, #key, real_json) != 0) \
				fprintf(stderr, "Config: error set json real ("__FILE__": %d)\n", __LINE__); \
		} \
		else { \
			fprintf(stderr, "Config: error creating json real ("__FILE__": %d)\n", __LINE__); \
		} \
	} \
}

#define	CONFIG_UPDATE_TO_JSON_BOOL(config_struct, config_json, key) \
{	\
	if (json_object_set_new(config_json, #key, json_boolean(config->key)) != 0) \
			fprintf(stderr, "Config: error set json integer ("__FILE__": %d)\n", __LINE__); \
}

static void config_update_to_json(struct config *config, json_t *config_json)
{
	CONFIG_UPDATE_TO_JSON(string, config, config_json, identification);
	CONFIG_UPDATE_TO_JSON_STRING(config, config_json, input_device);
	CONFIG_UPDATE_TO_JSON_STRING(config, config_json, output_path);
	CONFIG_UPDATE_TO_JSON_STRING(config, config_json, output_filename);
	CONFIG_UPDATE_TO_JSON_STRING(config, config_json, output_format);
	
	CONFIG_UPDATE_TO_JSON_INTEGER(config, config_json, sample_rate);
	CONFIG_UPDATE_TO_JSON_INTEGER(config, config_json, bits_per_sample);
	CONFIG_UPDATE_TO_JSON_INTEGER(config, config_json, block_size);
	CONFIG_UPDATE_TO_JSON_INTEGER(config, config_json, segment_duration);
	CONFIG_UPDATE_TO_JSON_INTEGER(config, config_json, record_period);
	CONFIG_UPDATE_TO_JSON_INTEGER(config, config_json, file_period);
	CONFIG_UPDATE_TO_JSON_INTEGER(config, config_json, laeq_time);

	CONFIG_UPDATE_TO_JSON_REAL(config, config_json, calibration_reference);

	CONFIG_UPDATE_TO_JSON_BOOL(config, config_json, mqtt_enable);
	CONFIG_UPDATE_TO_JSON_STRING(config, config_json, mqtt_broker);
	CONFIG_UPDATE_TO_JSON_STRING(config, config_json, mqtt_topic);
	CONFIG_UPDATE_TO_JSON_INTEGER(config, config_json, mqtt_qos);
	CONFIG_UPDATE_TO_JSON_STRING(config, config_json, mqtt_device_credential);
	CONFIG_UPDATE_TO_JSON_STRING(config, config_json, server_socket);
}

void config_destroy()
{
	json_decref(config_json);
}

struct config *config_load(const char *config_filename)
{
	config_json = json_object();
	if (config_json == NULL) {
		fprintf(stderr, "Config: error creating JSON object root.\n");
		return NULL;
	}
	config_update_to_json(config_struct, config_json);

	FILE *config_fd = fopen(config_filename, "r");
	if (config_fd != NULL) {
		json_error_t error;
		json_t *file_json = json_loadf(config_fd, 0, &error);
		if (file_json != NULL) {
			if (json_object_update(config_json, file_json) != 0)
				fprintf(stderr, "Error merging default configuration with file configuration\n");
			json_decref(file_json);
		}
		else {
			fprintf(stderr, "%s: error on line %d: %s\n"
							"Using default configuration",
							config_filename, error.line, error.text);
		}
		fclose(config_fd);
	}
	else {
		fprintf(stderr, "Config: error in file: %s: %s\n"
						"Using default configuration\n",
						config_filename, strerror(errno));
	}

	config_update_from_json(config_struct, config_json);
	return config_struct;
}

void config_save(const char *config_filename)
{
	config_update_to_json(config_struct, config_json);

	if (json_dump_file (config_json, config_filename, JSON_INDENT(8)) != 0) {
		fprintf(stderr, "Error saving configuration (JSON format). File: %s\n",
				config_filename);
	}
}
