#ifndef SBUFFER_H
#define SBUFFER_H

#include <stddef.h>

struct sbuffer;

/**
 * @brief Cria um buffer de tamanho size.
 */
struct sbuffer *sbuffer_create(unsigned size);

/**

 */
void sbuffer_destroy(struct sbuffer *this);

/**
 * @brief Retorna o número de elementos no buffer.
 */
unsigned sbuffer_size(struct sbuffer *this);

/**
 * @brief Retorna a capacidade do buffer.
 */
unsigned sbuffer_capacity(struct sbuffer *this);

/**
 * @brief Retorna um ponteiro para a zona de leitura.
 */
float *sbuffer_read_ptr(struct sbuffer *this);

/**
 * @brief Retorna o número de elementos que podem ser lidos.
 */
unsigned sbuffer_read_size(struct sbuffer *this);

/**
 * @brief Avança o ponteiro de leitura.
 */
void sbuffer_read_consumes(struct sbuffer *this, unsigned n);

/**
 * @brief Retorna um ponteiro para a zona de escrita.
 */
float *sbuffer_write_ptr(struct sbuffer *this);

/**
 * @brief Retorna o número de elementos que podem ser escritos.
 */
unsigned sbuffer_write_size(struct sbuffer *this);

/**
 * @brief Avança o ponteiro de escrita.
 */
void sbuffer_write_produces(struct sbuffer *this, unsigned n);

/**
 * @brief Retorna o número de posições livres no buffer.
 */
unsigned sbuffer_free(struct sbuffer *);

#endif
