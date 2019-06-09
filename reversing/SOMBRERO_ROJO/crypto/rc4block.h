#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "sha256.h"

#define RC4_KEY_BYTES	32
#define RC4_BLOCK_SIZE_BYTES 256

typedef struct RC4Ctx{
	unsigned int encrypted_size;
	unsigned int decrypted_size;
	unsigned int padding;
	bool encrypted;
	unsigned char key[RC4_KEY_BYTES];
	unsigned char key_pad_check[RC4_KEY_BYTES];
	
} rc4ctx;

void rc4_crypt(unsigned char *output, unsigned char *input, unsigned int length, unsigned char key[RC4_KEY_BYTES]);

unsigned char * rc4_block_encrypt(rc4ctx *encctx, unsigned char* indata, unsigned int dwSize);
unsigned char * rc4_block_decrypt(rc4ctx *encctx, unsigned char* indata);
int encrypt_file(char *infile, char *outfile, unsigned char key[]);
int decrypt_file(char *infile, char *outfile);

unsigned char * rc4_block_encrypt_ctf(rc4ctx *encctx, unsigned char* indata, unsigned int data_size, unsigned char kek[RC4_KEY_BYTES]);
unsigned char * rc4_block_decrypt_ctf_BADVERSION(rc4ctx *ctxdec, unsigned char* indata, unsigned char kek[RC4_KEY_BYTES]);
unsigned char * rc4_block_decrypt_ctf(rc4ctx *ctxdec, unsigned char* indata, unsigned char kek[RC4_KEY_BYTES]);
int encrypt_file_ctf(char *infile, char *outfile, unsigned char key[], unsigned char kek[RC4_KEY_BYTES]);
int decrypt_file_ctf(char *infile, char *outfile, unsigned char kek[RC4_KEY_BYTES]);
unsigned int rc4_encrypt_key_ctf(rc4ctx *crypt_ctx, unsigned char key[RC4_KEY_BYTES]);
unsigned int rc4_decrypt_key_ctf(rc4ctx *crypt_ctx, unsigned char key[RC4_KEY_BYTES]);
bool check_padding(unsigned char *padding, int size);
unsigned char * gen_kek_key(char *str1, char *str2); 