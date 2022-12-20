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

static char *identification = CONFIG_IDENTIFICATION;

int sample_rate;
float calibrated_value = CONFIG_CALIBRATOR_REFERENCE;

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
		"\t-c, --calibrate [<time>]\n",
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
		{0, 0, 0, 0}
	};

	int error_in_options = false;

	char *option_input_device = CONFIG_INPUT_DEVICE;
	char *option_input_filename = CONFIG_INPUT_FILENAME;
	char *option_output_filename = NULL;
	sample_rate = CONFIG_SAMPLE_RATE;
	char *option_identification = CONFIG_IDENTIFICATION;
	char *option_duration = CONFIG_DURATION;
	char *option_output_format = CONFIG_OUTPUT_FORMAT;
	char *option_calibrate_time = NULL;
	int option_index, option_char;

	int run_duration = 0;

	while ((option_char = getopt_long(argc, argv, ":hvi:o:f:r:d:l:t:c:", long_options, &option_index)) != -1) {
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
			option_input_device = optarg;
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
		case 'r': {
			sample_rate = atoi(optarg);
			break;
		}
		case 'n': {
			option_identification = optarg;
			break;
		}
		case 't': {
			option_duration = optarg;
			break;
		}
		case 'c': {
			option_calibrate_time = optarg;
			break;
		}
		case ':':
			if (optopt == 'c')
				option_calibrate_time = "5"; // CONFIG_CALIBRATION_TIME;
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
		exit(1);
	}
	identification = option_identification;

	if (option_output_filename != NULL)
		output_set_filename(option_output_filename, "");
	else if (option_input_filename != NULL)
		output_set_filename(option_input_filename, option_output_format);
	else
		output_set_filename(CONFIG_OUTPUT_FILENAME, option_output_format);


	if (verbose_flag)
		printf("Program arguments:\n"
			"\tInput device: %s\n"
			"\tOutput file_name: %s\n"
			"\tSample Rate: %d\n"
			"\tIdentification: %s\n"
			"\tCalibrate: %s\n\n",
			option_input_filename != NULL? option_input_filename : option_input_device,
			output_get_filename(),
			sample_rate,
			option_identification,
			option_calibrate_time);

	if (verbose_flag && option_input_filename == NULL && option_duration != NULL)
		printf("Duration: %s\n", option_duration);

	if (option_duration != NULL) {
		run_duration = atoi(option_duration);
	}
	output_init(run_duration != 0 ? CONFIG_FILE_TIME : 0 );

	input_device_open(option_input_filename, option_input_device);	/* testar o resultado */

	int samples_per_segment = CONFIG_SEGMENT_DURATION * sample_rate;
	int blocks_per_segment = samples_per_segment / CONFIG_BLOCK_SIZE;
	int last_block_size = samples_per_segment - blocks_per_segment * CONFIG_BLOCK_SIZE;
	blocks_per_segment += last_block_size != 0;

	if (verbose_flag) {
		printf("\tSample rate: %d\n", sample_rate);
		printf("\tSegment duration: %d second\n", CONFIG_SEGMENT_DURATION);
		printf("\tBlock size: %d\n", CONFIG_BLOCK_SIZE);
		printf("\tBlocks per segment: %d\n", blocks_per_segment);
		printf("\tLast block size: %d\n\n", last_block_size);
	}
	Block *block = block_create(blocks_per_segment, CONFIG_BLOCK_SIZE, last_block_size);

	Timeweight *twfilter = timeweight_create();
	Afilter *afilter = afilter_create(afilter_get_coef_a(CONFIG_SAMPLE_RATE),
									afilter_get_coef_b(CONFIG_SAMPLE_RATE), 6);

	unsigned seconds = 0;	// Time elapsed based in segment duration

	//-------------------------------CALIBRAÇÃO-------------------------------------------------
	if (option_calibrate_time != NULL) {
		unsigned calibration_time = strtoul(option_calibrate_time, NULL, 10);
		unsigned calibration_amount = calibration_time + CONFIG_CALIBRATION_GUARD;
		Calibrator *cal = calibrator_create(blocks_per_segment, calibration_time);
		printf("Starting Calibration in %d seconds ...\n", calibration_amount);
		while (1) {
			if (seconds >= calibration_amount) {
				printf("Calculating calibrated value...\n");
				calibrated_value = calibrator_calculate(cal);
				break;
			}
			unsigned block_size = block_next_size(block);
			unsigned block_size_read = input_device_read(block->sample_int16, block_size);
			if (block_size_read == 0)
				break;
			block->count = block_size_read;

			if (seconds >= (calibration_amount - calibration_time)) {
				block_sample_to_float(block);
				afilter_filtering(block, afilter);
				process_block_square(block);
				timeweight_filtering(block, twfilter);
				calibrator_block(block, cal);
			}

			if (++block->block_number == blocks_per_segment) {
				seconds += CONFIG_SEGMENT_DURATION;
				block->block_number = 0;
				printf("%d ", seconds);
			}
		}
		calibrator_destroy(cal);
	}

	//--------------------------------------------------------------------------------
	printf("%lf <-> %lf\n", (double)CONFIG_CALIBRATOR_REFERENCE, calibrated_value);

	//--------------------------------------------------------------------------

	printf("Starting sound level measuring...\n");
#if 0
	//-----Connect to Python Server----
	Client_Struct *cli = initClient();

	int socket_desc = 0;
	struct sockaddr_in server;
	connectSocket(cli, socket_desc, server);
#endif
	lae_average_create(CONFIG_LAEQ_TIME);
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
			process_segment(levels, calibrated_value);
			levels->block_number = 0;
			block->block_number = 0;
			seconds += CONFIG_SEGMENT_DURATION;
		}

		if (levels->segment_number == CONFIG_RECORD_PERIOD) {
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
}
