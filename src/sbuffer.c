#include <stdlib.h>
#include <assert.h>

#include "sbuffer.h"

static inline unsigned min(unsigned a, unsigned b) {
    return a < b ? a : b;
}

Sbuffer *sbuffer_create(unsigned capacity) {
    Sbuffer *this = malloc(sizeof *this);
    if (this == NULL)
        return NULL;
    this->capacity = capacity;
    this->buffer = malloc(this->capacity * sizeof *this->buffer);
    if (this->buffer == NULL) {
        free(this);
        return NULL;
    }
    this->get = this->put = this->put_counter = this->get_counter = 0;
	this->max_counter = 0;
    return this;
}

void sbuffer_destroy(Sbuffer *this) {
    free(this->buffer);
	free(this);
}

unsigned sbuffer_size(Sbuffer *this) {
    return this->put_counter - this->get_counter;
}

unsigned sbuffer_capacity(Sbuffer *this) {
    return this->capacity;
}

float *sbuffer_read_ptr(Sbuffer *this) {
    return &this->buffer[this->get];
}

unsigned sbuffer_read_size(Sbuffer *this) {
    return min(sbuffer_size(this), sbuffer_capacity(this) - this->get);
}

void sbuffer_read_consumes(Sbuffer *this, unsigned n) {
    assert(this->get < sbuffer_capacity(this));
    this->get += n;
    if (this->get >= sbuffer_capacity(this))
        this->get -= sbuffer_capacity(this);
    this->get_counter += n;
    assert(this->get < sbuffer_capacity(this));
}

float *sbuffer_write_ptr(Sbuffer *this) {
    return &this->buffer[this->put];
}

unsigned sbuffer_write_size(Sbuffer *this) {
    return min(sbuffer_capacity(this) - sbuffer_size(this), sbuffer_capacity(this) - this->put);
}

void sbuffer_write_produces(Sbuffer *this, unsigned n) {
    assert(this->put < this->capacity);
    assert(n <= sbuffer_free(this));
    this->put += n;
    if (this->put >= sbuffer_capacity(this))
        this->put -= sbuffer_capacity(this);
    this->put_counter += n;
    if (sbuffer_size(this) > this->max_counter)
        this->max_counter = sbuffer_size(this);
    assert(this->put < sbuffer_capacity(this));
    assert(this->max_counter <= sbuffer_capacity(this));
}

unsigned sbuffer_free(Sbuffer *this) {
	return this->capacity - sbuffer_size(this);
}
