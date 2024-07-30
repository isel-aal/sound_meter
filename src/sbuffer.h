#ifndef SBUFFER_H
#define SBUFFER_H

#include <stddef.h>

typedef struct sbuffer {
    float *buffer;        //  ponteiro para a zona de dados
    unsigned get;         //  posição de leitura
    unsigned put;         //  posição de escrita
    unsigned get_counter; //  contador de caracteres retirados
    unsigned put_counter; //  contador de caracteres colocados
    unsigned capacity;    //  capacidade do buffer
    unsigned max_counter; //  maior ocupação do buffer (para debug)
} Sbuffer;

Sbuffer *sbuffer_create(unsigned size);

void sbuffer_destroy(Sbuffer *this);

unsigned sbuffer_size(Sbuffer *this);

unsigned sbuffer_capacity(Sbuffer *this);

float *sbuffer_read_ptr(Sbuffer *this);

unsigned sbuffer_read_size(Sbuffer *this);

void sbuffer_read_consumes(Sbuffer *this, unsigned n);

float *sbuffer_write_ptr(Sbuffer *this);

unsigned sbuffer_write_size(Sbuffer *this);

void sbuffer_write_produces(Sbuffer *this, unsigned n);

unsigned sbuffer_free(Sbuffer *);

#endif
