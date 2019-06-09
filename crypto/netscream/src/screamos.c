#include <stdio.h>
#include <string.h>


#include "crypto.h"

void print();

int main(int argc, char* argv[]) {

	// arg check
	if(argc != 2) return 1;

	// initialize random numbers
	drbg_init();

	// open file
	FILE* fid = fopen(argv[1], "r");
	if(fid == NULL) return 2;

	// get length
	fseek(fid, 0, SEEK_END);
	long fsize = ftell(fid);
	fseek(fid, 0, SEEK_SET);

	// round up to aes block size
	fsize = ((fsize + 15) / 16) * 16;

	// read file into buffer
	uint8_t* buf = calloc(fsize, 1);
	fread(buf, fsize, 1, fid);
	fclose(fid);

	// generate key and iv
	uint8_t iv[32];
	drbg_generate(iv);

	uint8_t key[32];
	drbg_generate(key);

	// write key to file
	FILE* fkey = fopen("key","w");
	fwrite(key, sizeof(key), 1, fkey);
	fclose(fkey);

	// start writing encrypted file
	FILE* fenc = fopen("enc", "w");
	fwrite(iv, 32, 1, fenc);

	// encrypt data
	uint8_t* enc = encrypt(key, iv, buf, fsize);

	fwrite(enc, fsize, 1, fenc);
	fclose(fenc);



	return 0;
}

void print(uint8_t* buf, size_t size) {
	for(size_t i = 0; i < size; ++i) {
		if (i % 32 == 0) putchar('\n');
		printf("%02x", buf[i]);
	}
}
