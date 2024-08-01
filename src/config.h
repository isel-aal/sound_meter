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

#include <stdbool.h>

#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_VERSION "1.0"

#define CONFIG_CONFIG_FILENAME "sound_meter_config.json"
#define CONFIG_CONFIG_FILEPATH "./"

#define CONFIG_OUTPUT_PATH "data/"

#define CONFIG_INPUT_DEVICE "default"

#define CONFIG_INPUT_FILENAME NULL

#define CONFIG_OUTPUT_FILENAME "sound_meter"

#define CONFIG_OUTPUT_FORMAT ".csv"

#define CONFIG_IDENTIFICATION "XXXX_NNNN"

#define CONFIG_CALIBRATION_FACTOR 1.0

#define CONFIG_CALIBRATION_DEFAULT 94.0f //	Valor de referência do calibrador

#define CONFIG_PRESSURE_REFERENCE 0.00002f // valor de pressão de referencia 20 uP (Pascal)

#define CONFIG_PCM_FORMAT SND_PCM_FORMAT_S16_LE
#define CONFIG_BITS_PER_SAMPLE 16
#define CONFIG_FRAME_SIZE (CONFIG_BITS_PER_SAMPLE / 8)
#define CONFIG_SAMPLE_NORM ((float)(1 << (CONFIG_BITS_PER_SAMPLE - 1)))

#define CONFIG_MARGIN 0 //	pow(10, -10)        	// margem de -100dB

#define CONFIG_SAMPLE_RATE 48000
#define CONFIG_SEGMENT_DURATION 1 // duração de um segmento em segundos
#define CONFIG_BLOCK_SIZE 1024	  // dimensão de um bloco em número de amostras

//  Os seguintes tempos são definidos em número de segmentos
#define CONFIG_RECORD_PERIOD 60										//	periodo de registo e envio
#define CONFIG_FILE_TIME (60 * 60)									//	periodo de mudança de ficheiro de registo
#define CONFIG_LAEQ_TIME ((60 / CONFIG_SEGMENT_DURATION) * 60 * 24) //	periodo de cálculo de LAEeq (1 dia)

#define CONFIG_CALIBRATION_TIME 0  //	tempo útil de calibração
#define CONFIG_CALIBRATION_GUARD 2 //	guarda desde o arranque do programa até ao início da calibração


#define	CONFIG_MQTT_ENABLE	false
#define	CONFIG_MQTT_BROKER	"tcp://demo.thingsboard.io:1883"
#define CONFIG_MQTT_TOPIC	"v1/devices/me/telemetry"
#define CONFIG_MQTT_QOS		1
#define CONFIG_MQTT_DEVICE_CREDENTIAL	"undefined"

#define	CONFIG_MQTT_PUBLISH_PERIOD	10	//	Tempo de publicação em número de segmentos

typedef struct config
{
	char *identification;		// identificador da estação
	char *input_device;		// identificador da card de som
	char *input_file;		// ficheiro WAVE com som de entrada
	unsigned sample_rate;		// ritmo de amostragem
	unsigned bits_per_sample;	// número de bits por amostra
	unsigned block_size;		// dimensão de um bloco em número de amostras
	unsigned segment_duration;	// duração de um segmento de processamento
	unsigned segment_size;		// dimensão de um segmento em número de amostas
	unsigned record_period;		// periodo de registo de dados em numero de segmentos
	unsigned file_period;		// periodo de criação de novo ficheiro de registo
	char *output_path;		// diretoria onde são depositados os ficheiros criados
	char *output_format;		// formato dos dados de saída (extensão do ficheiro de saída)
	unsigned laeq_time;		// duração da janela deslizante de laeq

	unsigned calibration_time;	// tempo despendido na calibração
	float calibration_reference;	// valor de referência de calibração

	bool mqtt_enable;
	char *mqtt_broker;
	char *mqtt_topic;
	int mqtt_qos;
	char *mqtt_device_credential;
	int mqtt_publish_period;
} Config;

Config *config_load(const char *config_filename);
void config_save(Config *config, const char *config_filename);

void config_destroy(Config *config);

extern Config *config_struct;

#endif
