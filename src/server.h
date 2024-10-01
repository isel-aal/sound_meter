#ifndef SERVER_H
#define SERVER_H

void server_init();
void server_end();

void server_send(uint64_t ts, float laeq, float lafmin, float lae, float lafmax, float lapeak);

#endif
