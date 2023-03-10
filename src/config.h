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

#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_VERSION              "1.0"

#define CONFIG_CONFIG_FILENAME		"sound_meter.conf"

#define CONFIG_OUTPUT_PATH			"data/"

#define CONFIG_INPUT_DEVICE         "default"

#define CONFIG_INPUT_FILENAME       NULL

#define CONFIG_OUTPUT_FILENAME      "sound_meter"

#define CONFIG_OUTPUT_FORMAT		".csv"

#define CONFIG_IDENTIFICATION       "XXXX__NNNN"


#define CONFIG_CALIBRATION_FACTOR   1.0

#define CONFIG_CALIBRATION_DEFAULT	94.0f		//	Valor de referência do calibrador

#define CONFIG_PRESSURE_REFERENCE	(20.0f * pow(10, -6))		// valor de pressão de referencia 20 uP (Pascal)

#define CONFIG_PCM_FORMAT           SND_PCM_FORMAT_S16_LE
#define CONFIG_BITS_PER_SAMPLE      16
#define CONFIG_SAMPLE_NORM          ((float) (1 << (CONFIG_BITS_PER_SAMPLE - 1)))

#define CONFIG_MARGIN               pow(10, -10)        	// margem de -100dB

#define CONFIG_SAMPLE_RATE			48000
#define CONFIG_SEGMENT_DURATION 	1		// duração de um segmento em segundos
#define CONFIG_BLOCK_SIZE			1024	// dimensão de um bloco em número de amostras

//  Os seguintes tempos são definidos em número de segmentos
#define CONFIG_RECORD_PERIOD		60	    	//	periodo de registo e envio
#define CONFIG_FILE_TIME			(60 * 60)	//	periodo de mudança de ficheiro de registo
#define CONFIG_LAEQ_TIME			((60 / CONFIG_SEGMENT_DURATION) * 60 * 24)	//	periodo de cálculo de LAEeq (1 dia)

#define CONFIG_CALIBRATION_TIME 	0		//	tempo útil de calibração
#define CONFIG_CALIBRATION_GUARD	3 		//	guarda desde o arranque do programa até ao início da calibração

typedef struct config {
	char *identification;
	char *input_device;
	unsigned sample_rate;
	unsigned segment_duration;
	unsigned block_size;
	unsigned record_period;
	unsigned file_period;
	char *output_path;
	char *output_extention;
	unsigned laeq_time;
	
	unsigned calibration_time;
	float calibration_reference;
} Config;

Config *config_load(const char *config_filename);
void config_destroy(Config *config);

extern Config *config_struct;

#endif
