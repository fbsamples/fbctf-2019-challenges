#ifndef CRYPTO_H
#define CRYPTO_H

#include <stdint.h>
#include <stdlib.h>

void drbg_init();
int drbg_generate(uint8_t* buf);

uint8_t* encrypt(uint8_t* key, uint8_t* iv, uint8_t* buf, size_t size);

#endif
