#include <threads.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "config.h"

static mtx_t server_mutex;
static cnd_t server_condition;
static thrd_t server_thread;

static unsigned long long server_ts;
static float server_laeq;
static float server_lapeak;
static float server_lafmax;
static float server_lafmin;
static float server_lae;


static void timespec_add_mili(struct timespec *ts, unsigned milis) {
	if ( 999999999 - ts->tv_nsec > 1000000L * milis) {
		ts->tv_nsec += 1000000L * milis;
	}
	else {
		ts->tv_sec += 1;
		ts->tv_nsec = 1000000L * milis - (999999999 - ts->tv_nsec);
	}
}

static int sockcli_table[5];
static size_t sockcli_table_current;

#define ARRAY_SIZE(a) (sizeof (a) / sizeof (a)[0])

extern bool running;

static int server_thread_func(void *not_used) {
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd < 0) {
		fprintf(stderr, "Error in \"socket(AF_UNIX, SOCK_DGRAM, 0)\"");
		exit(EXIT_FAILURE);
	}
        int flags = fcntl(sockfd, F_GETFL, 0);
        if (flags < 0) {
                fprintf(stderr, "Error in \"fcntl(sockfd, F_GETFL, 0)\"");
                exit(EXIT_FAILURE);
        }
        int result = fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
        if (result < 0) {
                fprintf(stderr, "Error in \"fcntl(sockfd, F_SETFL, 0)\"");
                exit(EXIT_FAILURE);
        }
	struct sockaddr_un sockaddr_local;
	sockaddr_local.sun_family = AF_UNIX;
	strcpy(sockaddr_local.sun_path, config_struct->server_socket);
	unlink(sockaddr_local.sun_path);
	size_t len = sizeof(sockaddr_local.sun_family) + strlen(sockaddr_local.sun_path);
	result = bind(sockfd, (struct sockaddr *)&sockaddr_local, len);
	if (result < 0) {
		fprintf(stderr, "Error in \"bind(sock, local, len)\"");
		exit(EXIT_FAILURE);
	}

	result = listen(sockfd, ARRAY_SIZE(sockcli_table));
	if (result < 0) {
		fprintf(stderr, "Error in \"listen(sockfd, 0)\"");
		exit(EXIT_FAILURE);
	}

//	printf("Waiting for connection in \"sound_server_socket\" ...\n");
        mtx_lock(&server_mutex);
        while (running) {
		struct timespec time_point;
                timespec_get(&time_point, TIME_UTC);
                timespec_add_mili(&time_point, 100);     /* daqui a 100 milisegundos */
                int result = cnd_timedwait(&server_condition, &server_mutex, &time_point);
                if (result == thrd_success) {
                        char buffer[120];
                        snprintf(buffer, sizeof buffer, "{\"ts\": %lld, \"values\": "
                                         "{\"LAeq\": %.1f, \"LAFmin\": %.1f, \"LAE\": %.1f, \"LAFmax\": %.1f, \"LApeak\": %.1f } }",
                                server_ts, server_laeq, server_lafmin, server_lae, server_lafmax, server_lapeak);
                        for (int i = 0; i < sockcli_table_current; ) {
                                ssize_t written = write(sockcli_table[i], buffer, strlen(buffer));
                                if (written < 0) {
                                    close(sockcli_table[i]);
                                    sockcli_table[i] = sockcli_table[--sockcli_table_current];
                                }
                                else {
                                        i++;
                                }
                        }
                }
                else if (result == thrd_timedout 
                        && sockcli_table_current < ARRAY_SIZE(sockcli_table)) {
                        int sockclifd = accept(sockfd, NULL, 0);
                        if (sockclifd < 0) {
                                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                                        fprintf(stderr, "Error in \"accept(sockfd, NULL, 0)\"");
                                        exit(EXIT_FAILURE);
                                }
                        }
                        else { /* nova ligação */
                                int flags = fcntl(sockclifd, F_GETFL, 0);
                                if (flags < 0) {
                                        fprintf(stderr, "Error in \"fcntl(sockfd, F_GETFL, 0)\"");
                                        exit(EXIT_FAILURE);
                                }
                                int result = fcntl(sockclifd, F_SETFL, flags | O_NONBLOCK);
                                if (result < 0) {
                                        fprintf(stderr, "Error in \"fcntl(sockfd, F_SETFL, 0)\"");
                                        exit(EXIT_FAILURE);
                                }
                                sockcli_table[sockcli_table_current++] = sockclifd;
                        }
                }
        }
        mtx_unlock(&server_mutex);
        return 0;
}

void server_init() {
        mtx_init(&server_mutex, mtx_plain);
        cnd_init(&server_condition);
        if (thrd_success != thrd_create(&server_thread, server_thread_func, NULL)) {
                fprintf(stderr, "Error in \"thrd_create(&thread_server, server_thread_func, NULL)\"");
                exit(EXIT_FAILURE);
        }
}

void server_end() {
        int result;
        thrd_join(server_thread, &result);
}

void server_send(uint64_t ts, float laeq, float lafmin, float lae, float lafmax, float lapeak) {
	mtx_lock(&server_mutex);
        server_ts = ts;
        server_laeq = laeq;
        server_lapeak = lapeak;
        server_lafmax = lafmax;
        server_lafmin = lafmin;
        server_lae = lae;
	mtx_unlock(&server_mutex);
	cnd_signal(&server_condition);
}

