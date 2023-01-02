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
#include "process.h"
#include "filter.h"
#include "config.h"
#include "in_out.h"

static void help(char *prog_name) {
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
		"\t-g, --config <file name>\n",
		prog_name);
}

static void about() {
	printf("Sound meter v" CONFIG_VERSION " (" __DATE__ ")\n"
		"Based on MoSeMusic project by Guilherme Albano and David Meneses\n"
		"Ezequiel Conde (ezeq@cc.isel.ipl.pt)\n");
}

int main (int argc, char *argv[]) {
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
	char *option_input_filename = CONFIG_INPUT_FILENAME;
	char *option_output_filename = NULL;
	char *option_output_extention = NULL;
	char *option_sample_rate = NULL;
	char *option_identification = NULL;
	char *option_calibration_time = NULL;
	char *option_file_config = NULL;
	int run_duration = 0;

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
			option_output_extention = optarg;
			break;
		case 'g':
			option_file_config = optarg;
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

	//	Ler as configurações em ficheiro

	char *config_filename;
	if (option_file_config != NULL)
		config_filename = strdup(option_file_config);
	else
		config_filename = strdup(CONFIG_CONFIG_FILENAME);

	if (config_filename[0] != '/') {
		char *config_path = getenv("SOUND_METER_PATH_CONFIG");
		if (config_path != NULL) {
			char *filepath = malloc(strlen(config_path) + strlen(config_filename) + 1);
			if (filepath == NULL) {
				fprintf(stderr, "Out of memory\n");
				exit(EXIT_FAILURE);
			}
			strcpy(filepath, config_path);
			strcat(filepath, config_filename);
			free(config_filename);
			config_filename = filepath;
		}
	}

	if (verbose_flag)
		printf("Configuration file: %s\n", config_filename);

	config_struct = config_load(config_filename);
	if (config_struct == NULL)
		exit(EXIT_FAILURE);
	
	free(config_filename);

	//	As opções de linha de comando prevalecem sobre o ficheiro

	if (option_device_filename != NULL) {
		if (config_struct->input_device != NULL)
			free(config_struct->input_device);
		config_struct->input_device = strdup(option_device_filename);
	}

	if (option_output_extention != NULL) {
		if (config_struct->output_extention != NULL)
			free(config_struct->output_extention);
		config_struct->output_extention = strdup(option_output_extention);
	}

	if (option_sample_rate != NULL) {
		config_struct->sample_rate = atoi(option_sample_rate);
	}

	if (option_identification != NULL) {
		if (config_struct->identification != NULL)
			free(config_struct->identification);
		config_struct->identification = strdup(option_identification);
	}

	if (option_calibration_time != NULL) {
		config_struct->calibration_time = atoi(option_calibration_time);
	}	

	if (option_output_filename != NULL)
		output_set_filename(option_output_filename, "");
	else if (option_input_filename != NULL)
		output_set_filename(option_input_filename, config_struct->output_extention);

	if (verbose_flag)
		printf("Program arguments:\n"
			"\tFile config: %s\n"
			"\tInput device: %s\n"
			"\tInput file name: %s\n"	
			"\tOutput file name: %s\n"
			"\tOutput directory: %s\n"
			"\tIdentification: %s\n"
			"\tSample Rate: %d\n"
			"\tSegment duration: %d second\n"
			"\tBlock size: %d samples\n"
			"\tCalibration reference: %.1f db\n"
			"\tCalibration time: %d\n"
			"\tRecord period: %d segments\n"
			"\tFile period: %d segments\n"
			"\tRun duration: %d seconds\n\n",
			config_filename,
			config_struct->input_device,
			option_input_filename,
			output_get_filename(),
			config_struct->output_path,
			config_struct->identification,
			config_struct->sample_rate,
			config_struct->segment_duration,
			config_struct->block_size,
			config_struct->calibration_reference,
			config_struct->calibration_time,
			config_struct->record_period,
			config_struct->file_period,
			run_duration);

	//	------------------------------------------------------------------------

	int continous = option_input_filename == NULL && option_input_filename == NULL;
	output_init(continous);

	int result = input_device_open(option_input_filename, config_struct->input_device, config_struct);
	if (result != 0)
		exit(EXIT_FAILURE);

	int samples_per_segment = config_struct->segment_duration * config_struct->sample_rate;
	int blocks_per_segment = samples_per_segment / config_struct->block_size;
	int last_block_size = samples_per_segment - blocks_per_segment * config_struct->block_size;
	blocks_per_segment += last_block_size != 0;

	if (verbose_flag) {
		printf("\tBlocks per segment: %d\n", blocks_per_segment);
		printf("\tLast block size: %d\n\n", last_block_size);
	}
	Block *block = block_create(blocks_per_segment, config_struct->block_size, last_block_size);

	Timeweight *twfilter = timeweight_create();
	Afilter *afilter = afilter_create(afilter_get_coef_a(config_struct->sample_rate),
									afilter_get_coef_b(config_struct->sample_rate), 6);

	unsigned seconds = 0;	// Time elapsed based in segment duration

	//-------------------------------CALIBRAÇÃO---------------------------------
	float calibration_delta = 0;
	if (config_struct->calibration_time > 0) {
		unsigned calibration_amount = config_struct->calibration_time + CONFIG_CALIBRATION_GUARD;
		Calibrator *cal = calibrator_create(blocks_per_segment, config_struct->calibration_time);
		printf("Starting Calibration in %d seconds ...\n", calibration_amount);
		while (1) {
			if (seconds >= calibration_amount) {
				printf("Calculating calibrated value...\n");
				calibration_delta = config_struct->calibration_reference - calibrator_calculate(cal);
				break;
			}
			unsigned block_size = block_next_size(block);
			unsigned block_size_read = input_device_read(block->sample_int16, block_size);
			if (block_size_read == 0)
				break;
			block->count = block_size_read;

			if (seconds >= (calibration_amount - config_struct->calibration_time)) {
				block_sample_to_float(block);
				afilter_filtering(block, afilter);
				process_block_square(block);
				timeweight_filtering(block, twfilter);
				calibrator_block(block, cal);
			}

			if (++block->block_number == blocks_per_segment) {
				seconds += config_struct->segment_duration;
				block->block_number = 0;
				printf("%d ", seconds);
			}
		}
		calibrator_destroy(cal);
	}

	printf("\tCalibration adjust: %.1f db\n", calibration_delta);

	//--------------------------------------------------------------------------

	printf("\nStarting sound level measuring...\n");
#if 0
	//-----Connect to Python Server----
	Client_Struct *cli = initClient();

	int socket_desc = 0;
	struct sockaddr_in server;
	connectSocket(cli, socket_desc, server);
#endif
	lae_average_create(config_struct->laeq_time);
	Levels *levels = levels_create(blocks_per_segment);
	while (run_duration == 0 || seconds < run_duration) {
		unsigned block_size = block_next_size(block);
//		printf("%zd %zd\n", block->block_number, block_size);
		unsigned block_size_read = input_device_read(block->sample_int16, block_size);
		if (block_size_read == 0)
			break;
		block->count = block_size_read;
		block_sample_to_float(block);
		afilter_filtering(block, afilter);
		process_block_square(block);
		process_block_lapeak(block, levels);
		timeweight_filtering(block, twfilter);
		process_block(block, levels);

		if (++block->block_number == blocks_per_segment) {
			process_segment(levels, calibration_delta);
			levels->block_number = 0;
			block->block_number = 0;
			seconds += config_struct->segment_duration;
		}

		if (levels->segment_number == config_struct->record_period) {
			output_record(levels);
			levels->segment_number = 0;
//			cli->send_counter = cli->send_counter + 1;
		}
	}
	output_record(levels);
	output_file_close();

	printf("Total time: %d\n", seconds);
	input_device_close();
	block_destroy(block);
	levels_destroy(levels);
	timeweight_destroy(twfilter);
	afilter_destroy(afilter);
	lae_average_destroy();
	config_destroy(config_struct);
}
