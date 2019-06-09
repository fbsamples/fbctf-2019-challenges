#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include <openssl/aes.h>

#include "crypto.h"
#include "dualec.h"

static FILE* drbg_random;

static uint8_t drbg_key_buf[32];
static AES_KEY drbg_key;
static uint8_t drbg_counter[16];

static size_t drbg_size;
static uint8_t drbg_temporary[32];

int should_reseed();
void drbg_reseed();
void inc_counter();

void drbg_init() {
	drbg_random = fopen("/dev/urandom", "rb");
	fread(drbg_key_buf, 1, sizeof(drbg_key_buf), drbg_random);

	AES_set_encrypt_key(drbg_key_buf, sizeof(drbg_key_buf) * 8, &drbg_key);
	dualec_init(drbg_random);
}

int drbg_generate(uint8_t* buf) {

	drbg_size = 0;

	if(should_reseed()) {
		drbg_reseed();
	}

	for(;drbg_size < 32; drbg_size += 16) {
		AES_encrypt(drbg_counter, &drbg_temporary[drbg_size], &drbg_key);
		inc_counter();
	}

	memcpy(buf, drbg_temporary, 32);
}

void drbg_reseed() {
	drbg_size = sizeof(drbg_key_buf);
	dualec_generate(drbg_temporary, drbg_size);
	AES_set_encrypt_key(drbg_temporary, drbg_size * 8, &drbg_key);
	inc_counter();
}

void inc_counter() {
	uint32_t* val = (uint32_t*) drbg_counter;

	if(++val[0]) return;
	if(++val[1]) return;
	if(++val[2]) return;
	++val[3];
}

int should_reseed() {
	//TODO red herring with time, always ends up returning true
	return 1;
}

uint8_t* encrypt(uint8_t* key, uint8_t* iv, uint8_t* buf, size_t size) {
	uint8_t* out = malloc(size);

	AES_KEY enc_key;
	AES_set_encrypt_key(key, 32*8, &enc_key);

	AES_ige_encrypt(buf, out, size, &enc_key, iv, AES_ENCRYPT);

	return out;
}
