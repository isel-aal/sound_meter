/*
Copyright 2024 Laboratório de Audio e Acústica do ISEL

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

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"

#include "config.h"
#include "mqtt.h"

static MQTTClient client;

bool mqtt_begin() {
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    if ((rc = MQTTClient_create(&client,
        config_struct->mqtt_broker, config_struct->identification,
        MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS) {
         fprintf(stderr, "Failed to create MQTT client, return code %d\n", rc);
         return false;
    }

	conn_opts.username = config_struct->mqtt_device_credential;
	conn_opts.password = config_struct->mqtt_device_credential;
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "Failed to connect MQTT, return code %d\n", rc);
        return false;
    }
    return true;
}

#define TIMEOUT     10000L

bool mqtt_publish(Levels *levels, int segment_number) {
	char payload[120];
    unsigned long long ts = (uint64_t)time(NULL) * 1000;
	sprintf(payload, "{\"ts\": %lld, \"values\": "
		"{\"LAeq\": %.1f, \"LAFmin\": %.1f, \"LAE\": %.1f, \"LAFmax\": %.1f, \"LApeak\": %.1f } }",
        ts,
        levels->LAeq[segment_number],
        levels->LAFmin[segment_number],
        levels->LAE[segment_number],
        levels->LAFmax[segment_number],
        levels->LApeak[segment_number]);

//    fprintf(stderr, "%s\n", payload);
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    pubmsg.payload = payload;
    pubmsg.payloadlen = strlen(payload);
    pubmsg.qos = config_struct->mqtt_qos;
    pubmsg.retained = 0;
    int rc;
    if ((rc = MQTTClient_publishMessage(client,
        config_struct->mqtt_topic, &pubmsg, &token)) != MQTTCLIENT_SUCCESS) {
         fprintf(stderr, "Failed to publish MQTT message, return code %d\n", rc);
         return false;
    }
/*
    printf("Waiting for up to %d seconds for publication of %s\n"
            "on topic %s for client with ClientID: %s\n",
            (int)(TIMEOUT/1000), payload, config_struct->mqtt_topic, config_struct->identification);
    rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
    printf("Message with delivery token %d delivered\n", token);
*/
    return true;
}

bool mqtt_end() {
    int rc;
    if ((rc = MQTTClient_disconnect(client, 10000)) != MQTTCLIENT_SUCCESS)
        fprintf(stderr, "Failed to disconnect MQTT, return code %d\n", rc);
    MQTTClient_destroy(&client);
    return rc == MQTTCLIENT_SUCCESS;
}