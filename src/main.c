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

#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#include <getopt.h>
#include <glib.h>

#include "process.h"
#include "filter.h"
#include "config.h"
#include "in_out.h"
#include "mqtt.h"

static bool running = true;

static void int_handler(int unused) {
	running = false;
}


static void help(char *prog_name)
{
	printf("Usage: %s [options] <source file_name>\n"
		"options:\n"
		"\t--verbose\n"
		"\t-h, --help\n"
		"\t-v, --version\n"
		"\t-d, --device <device name>\n"
		"\t-i, --input <file name>\n"
		"\t-o, --output <file name>\n"
		"\t-f, --output_format <csv | json>\n"
		"\t-r, --sample_rate <rate>\n"
		"\t-n, --identification <name>\n"
		"\t-t, --duration <time>\n"
		"\t-c, --calibrate [<time>]\n"
		"\t-g, --config <filename>\n",
		prog_name);
}

static void about()
{
	printf("Sound meter v" CONFIG_VERSION " (" __DATE__ ")\n"
		"Based on MoSeMusic project by Guilherme Albano and David Meneses\n"
		"Ezequiel Conde (ezeq@cc.isel.ipl.pt)\n");
}

int main (int argc, char *argv[])
{
	static int verbose_flag = false;
	static struct option long_options[] = {
		{"verbose", no_argument, &verbose_flag, 1},
		{"help", no_argument, 0, 'h'},
		{"version", no_argument, 0, 'v'},
		{"device", required_argument, 0, 'd'},
		{"input", required_argument, 0, 'i'},
		{"output", required_argument, 0, 'o'},
		{"output_format", required_argument, 0, 'f'},
		{"sample_rate", required_argument, 0, 'r'},
		{"identification", required_argument, 0, 'n'},
		{"duration", required_argument, 0, 't'},
		{"calibrate", optional_argument, 0, 'c'},
		{"config", required_argument, 0, 'g'},
		{0, 0, 0, 0}
	};

	int option_index, option_char;
	int error_in_options = false;

	char *option_device_filename = NULL;
	char *option_input_filename = NULL;
	char *option_output_filename = NULL;
	char *option_output_format = NULL;
	char *option_sample_rate = NULL;
	char *option_identification = NULL;
	char *option_calibration_time = NULL;
	char *option_config_filename = NULL;
	int run_duration = 0;

	signal(SIGINT, int_handler);

	while ((option_char = getopt_long(argc, argv, ":hvi:o:f:r:d:l:t:c:g:",
			long_options, &option_index)) != -1) {
		switch (option_char) {
		case 0:	//	Opções longas com afetação de flag
			break;
		case 'h':
			help(argv[0]);
			return 0;
		case 'v':
			about();
			break;
		case 'd':
			option_device_filename = optarg;
			break;
		case 'i':
			option_input_filename = optarg;
			break;
		case 'o':
			option_output_filename = optarg;
			break;
		case 'f':
			option_output_format = optarg;
			break;
		case 'g':
			option_config_filename = optarg;
			break;
		case 'r': {
			option_sample_rate = optarg;
			break;
		}
		case 'n': {
			option_identification = optarg;
			break;
		}
		case 't': {
			run_duration = atoi(optarg);
			break;
		}
		case 'c': {
			option_calibration_time = optarg;
			break;
		}
		case ':':
			if (optopt == 'c')
				option_calibration_time = CONFIG_CALIBRATION_TIME;
			else {
				fprintf(stderr, "Error in option -%c argument\n", optopt);
				error_in_options = true;
			}
			break;
		case '?':
			error_in_options = true;
			break;
		}
	}
	if (error_in_options) {
		help(argv[0]);
		exit(EXIT_FAILURE);
	}

	//----------------------------------------------------------------------
	//	Determinar nome do ficheiro de configuração

	const char *config_filename = option_config_filename;
	if (NULL == config_filename) {
		config_filename = getenv("SOUND_METER_CONFIG_FILENAME");
		if (config_filename == NULL)
			config_filename = CONFIG_CONFIG_FILENAME;
	}
	if (verbose_flag)
		printf("Configuration filename: %s\n", config_filename);

	/* Só a partir da versão 2.76 da GLib
	struct GPathBuf config_pathname;
	g_path_buf_init(&config_pathname);

	char first_letter = option_config_filename[0];
	if (first_letter != '/' && first_letter != '.') { // Anexar path
		const char *path = getenv("SOUND_METER_CONFIG_FILEPATH");
		if (path == NULL)
			path = CONFIG_CONFIG_FILEPATH;
		g_path_buf_push(&config_pathname, path);
	}
	g_path_buf_push(&config_pathname, option_config_filename);

	const char *config_filename = g_path_buf_free_to_path(&config_pathname);
	*/
	char *config_pathname;
	char first_letter = config_filename[0];
	if (first_letter != '/' && first_letter != '.') { // absoluto / relativo
		const char *path = getenv("SOUND_METER_CONFIG_FILEPATH");
		if (path == NULL)
			path = CONFIG_CONFIG_FILEPATH;
		char *pathname = malloc(strlen(path) + strlen(config_filename) + 1);
		strcpy(pathname, path);
		strcat(pathname, config_filename);
		config_pathname = pathname;
	}
	else {
		config_pathname = strdup(config_filename);
	}
	if (verbose_flag)
		printf("Configuration pathname: %s\n", config_pathname);

	config_struct = config_load(config_pathname);

	free(config_pathname);

	//	As opções de linha de comando prevalecem sobre o ficheiro de configuração

	if (option_device_filename != NULL)
		config_struct->input_device = option_device_filename;

	if (option_input_filename != NULL)
		config_struct->input_file = option_input_filename;

	if (option_output_format != NULL)
		config_struct->output_format = option_output_format;

	if (option_sample_rate != NULL) {
		config_struct->sample_rate = atoi(option_sample_rate);
	}

	if (option_identification != NULL)
		config_struct->identification = option_identification;

	if (option_calibration_time != NULL)
		config_struct->calibration_time = atoi(option_calibration_time);

	output_set_filename(option_output_filename, option_input_filename);

	config_struct->segment_size = config_struct->segment_duration * config_struct->sample_rate;

	if (verbose_flag) {
		config_print();
		printf("\n"
			"\tOutput file: %s\n"
			"\tRun duration: %d seconds\n\n",
			output_get_filepath(),
			run_duration);
	}

	//--------------------------------------------------------------------------
	//	Operation

	bool continuous = option_input_filename == NULL;

	output_open(continuous);

	if (!input_device_open(config_struct))
		exit(EXIT_FAILURE);

	Timeweight *twfilter = timeweight_create();
	Afilter *afilter = aweighting_create(3);

	unsigned seconds = 0;	// Time elapsed based in segment duration

	int16_t *samples_int16 = malloc(CONFIG_BLOCK_SIZE * sizeof *samples_int16);
	float *block_a = malloc(CONFIG_BLOCK_SIZE * sizeof *block_a);
	float *block_c = malloc(CONFIG_BLOCK_SIZE * sizeof *block_c);

	unsigned segment_buffer_size = (((config_struct->segment_size + CONFIG_BLOCK_SIZE - 1)
								/ CONFIG_BLOCK_SIZE) + 1)
								* CONFIG_BLOCK_SIZE;
	Sbuffer *ring_b = sbuffer_create(segment_buffer_size);
	Sbuffer *ring_d = sbuffer_create(segment_buffer_size);

	Levels *levels = levels_create();

	//-------------------------------CALIBRAÇÃO---------------------------------

	float calibration_delta = 0;

	if (config_struct->calibration_time > 0) {
		float sum = 0;
		printf("\nCalibrating for %d seconds\n",config_struct->calibration_time);
		while (seconds < config_struct->calibration_time + CONFIG_CALIBRATION_GUARD) {
			size_t lenght_read = input_device_read(samples_int16, CONFIG_BLOCK_SIZE);
			if (lenght_read == 0)
				break;
			samples_int16_to_float(samples_int16, block_a, lenght_read);
			float *block_ring_b = sbuffer_write_ptr(ring_b);
			assert(lenght_read <= sbuffer_write_size(ring_b));

			aweighting_filtering(afilter, block_a, block_ring_b, lenght_read);
			sbuffer_write_produces(ring_b, lenght_read);
			process_block_square(block_ring_b, block_c, lenght_read);
			sbuffer_read_consumes(ring_b, lenght_read);
		
			float *block_ring_d = sbuffer_write_ptr(ring_d);
			assert(lenght_read <= sbuffer_write_size(ring_d));

			timeweight_filtering(twfilter, block_c, block_ring_d, lenght_read);
			sbuffer_write_produces(ring_d, lenght_read);

			if (sbuffer_size(ring_d) >= config_struct->segment_size) {
				process_segment(levels, ring_d, calibration_delta);
				if (seconds >= CONFIG_CALIBRATION_GUARD)
					sum += levels->LAE[0];
				levels->segment_number = 0;
				seconds += config_struct->segment_duration;
				if (verbose_flag)
					putchar('.');
			}
		}
		calibration_delta = config_struct->calibration_reference -
			linear_to_decibel(sum / config_struct->calibration_time);
		
		if (verbose_flag)
			printf("\nCalibration delta: %.1f\n", calibration_delta);
	}

	//----------------------------------------------------------------------

	printf("\nStarting sound level measuring...\n");

	Audit *wa = NULL;
	Audit *wb = NULL;
	Audit *wc = NULL;
	Audit *wd = NULL;

	if (!continuous) {
		wa = audit_create("a");
		wb = audit_create("b");
		wc = audit_create("c");
		wd = audit_create("d");
	}
	if (config_struct->mqtt_enable)
		mqtt_begin();

	lae_average_create(config_struct->laeq_time);

	if (verbose_flag)
		printf("LAeq, LAFmin, LAE, LAFmax, LApeak\n");

	while (running && (run_duration == 0 || seconds < run_duration)) {
		size_t lenght_read = input_device_read(samples_int16, CONFIG_BLOCK_SIZE);
		if (lenght_read == 0)
			break;
		samples_int16_to_float(samples_int16, block_a, lenght_read);
		float *block_ring_b = sbuffer_write_ptr(ring_b);
		assert(lenght_read <= sbuffer_write_size(ring_b));

		aweighting_filtering(afilter, block_a, block_ring_b, lenght_read);

		sbuffer_write_produces(ring_b, lenght_read);

		process_segment_lapeak(levels, ring_b, calibration_delta);

		process_block_square(block_ring_b, block_c, lenght_read);

		float *block_ring_d = sbuffer_write_ptr(ring_d);
		assert(lenght_read <= sbuffer_write_size(ring_d));

		timeweight_filtering(twfilter, block_c, block_ring_d, lenght_read);
		sbuffer_write_produces(ring_d, lenght_read);

		if (!continuous) {
			audit_append_samples(wa, block_a, lenght_read);
			audit_append_samples(wb, block_ring_b, lenght_read);
			audit_append_samples(wc, block_c, lenght_read);
			audit_append_samples(wd, block_ring_d, lenght_read);
		}

		if (sbuffer_size(ring_d) >= config_struct->segment_size) {
			process_segment(levels, ring_d, calibration_delta);
			seconds += config_struct->segment_duration;
			if (config_struct->mqtt_enable)
				mqtt_publish(levels, levels->segment_number - 1);
			if (verbose_flag) {
				int segment_index = levels->segment_number - 1;
				printf("\r%6.1f%6.1f%6.1f%6.1f%6.1f\n",
					levels->LAeq[segment_index],
					levels->LAFmin[segment_index],
					levels->LAE[segment_index],
					levels->LAFmax[segment_index],
					levels->LApeak[segment_index]);
			}
		}

		if (levels->segment_number == config_struct->record_period) {
			output_record(levels);
			levels->segment_number = 0;
		}
	}
	if (verbose_flag)
		printf("\nTotal time: %d\n", seconds);

	output_record(levels);
//	output_file_close();

	if (verbose_flag)
		printf("Saving configuration in " CONFIG_CONFIG_FILEPATH CONFIG_CONFIG_FILENAME "\n");
	config_save(CONFIG_CONFIG_FILEPATH CONFIG_CONFIG_FILENAME);

	if (!continuous) {
		audit_destroy(wa);
		audit_destroy(wb);
		audit_destroy(wc);
		audit_destroy(wd);
	}

	if (config_struct->mqtt_enable)
		mqtt_end();
	input_device_close();
	output_close();
	levels_destroy(levels);
	timeweight_destroy(twfilter);
	aweighting_destroy(afilter);
	lae_average_destroy();
	config_destroy(config_struct);
	free(block_a);
	free(block_c);
	free(samples_int16);
	sbuffer_destroy(ring_b);
	sbuffer_destroy(ring_d);
}
