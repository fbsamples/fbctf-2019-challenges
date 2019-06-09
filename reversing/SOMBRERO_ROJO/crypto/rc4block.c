#include "rc4block.h"


unsigned char * gen_kek_key(char *str1, char *str2)
{
	unsigned char * buf;
	char *cat_strings;
	SHA256_CTX ctx;
	int idx;
	int pass = 1;

	int size_of_new_string = strlen(str1) + strlen(str2) + 1;
	cat_strings = (char *) malloc(size_of_new_string);

	buf = (unsigned char *) malloc(sizeof(unsigned char) * SHA256_BLOCK_SIZE);

	strcpy(cat_strings, str2);
	strcat(cat_strings, str1);

	#ifdef DEBUG
		printf("String to be hashed %s\n", cat_strings);
		printf("Getting hashes!\n");
	#endif 
	
	sha256_init(&ctx);
	sha256_update(&ctx, cat_strings, size_of_new_string);
	sha256_final(&ctx, buf);

	#ifdef DEBUG
		printf("hash of strings\n");
		pretty_print_bytes(buf, SHA256_BLOCK_SIZE);
		printf("\n");
	#endif

	free(cat_strings);

return buf;
}

bool check_padding(unsigned char *padding, int size)
{
	int i = 0;
	unsigned char padbyte = 0xFB;
	while(i < size)
	{
		if(padding[i] != padbyte)
		{
				return false;
		}
		i++;
	}
return true;
}

unsigned int rc4_encrypt_key_ctf(rc4ctx *crypt_ctx, unsigned char key[RC4_KEY_BYTES])
{
	unsigned int key_pad_size = 64;
	unsigned char key_enc_buffer[key_pad_size];
	unsigned char *key_ptr = NULL;
	
	#ifdef DEBUG
		printf("[+] Encrypting with key\n");

		if(key == NULL)
		{
			unsigned char key[RC4_KEY_BYTES];
			memset(&key, 0x20, RC4_KEY_BYTES);
		}
	#endif 

	memset(crypt_ctx->key_pad_check, 0xFB, RC4_KEY_BYTES);
	
	rc4_crypt(key_enc_buffer, crypt_ctx->key, RC4_KEY_BYTES, key);
	rc4_crypt(key_enc_buffer+RC4_KEY_BYTES, crypt_ctx->key_pad_check, RC4_KEY_BYTES, key);
	
	memcpy(crypt_ctx->key, key_enc_buffer, key_pad_size);
	return 0;

}

unsigned char * rc4_block_encrypt_ctf(rc4ctx *encctx, unsigned char* indata, unsigned int data_size, unsigned char kek[RC4_KEY_BYTES])
{
	
	unsigned char *newBuffer = NULL;
	unsigned int blocksize = RC4_BLOCK_SIZE_BYTES;
	unsigned int paddingsize = data_size % blocksize;
	unsigned int origsize = data_size;
	unsigned int newsize = 0;
	unsigned int tempsize = 0;
	unsigned char* tempindata = 0;
	unsigned char* output = NULL;

	#ifdef DEBUG
		printf("[+] rc4_block_encrypt_ctf.\n");
		printf("   [+] Getting padding size.\n");
	#endif
	
	encctx->decrypted_size = data_size;

	if(paddingsize > 0)
	{
		newsize = (blocksize  - paddingsize) + data_size;
	}
	else
	{
		newsize = data_size;
	}
	
	encctx->padding = blocksize  - paddingsize;
	
	if(newsize <= 0)
	{
		#ifdef DEBUG
			fprintf(stderr, "[*] rc4_block_encrypt_ctf: Encrypted size is zero\n");
		#endif

		return NULL;
	}
	
	encctx->encrypted_size = newsize;
	
	#ifdef DEBUG
		printf("   [+] Padding size %d\n", encctx->padding);
		printf("   [+] Encrypted size %d\n", encctx->encrypted_size);
		printf("   [+] Original Decrypted size %d\n", encctx->decrypted_size);
	#endif


	output = (unsigned char *) malloc(newsize * sizeof(unsigned char));
	if(output == NULL)
	{
		#ifdef DEBUG
			fprintf(stderr, "[*]rc4_block_encrypt_ctf: Memory error rc4_block_encrypt_ctf!\n");
		#endif
		return NULL;
	}
		memset(output, 0, newsize);
	
	//Create temp buffer with correct block size padding
	tempindata = (unsigned char *) malloc(newsize * sizeof(unsigned char)); //, sizeof(unsigned char));
	if(tempindata == NULL)
	{
		#ifdef DEBUG
			fprintf(stderr, "[*] rc4_block_encrypt_ctf: Memory error rc4_block_encrypt!\n");
		#endif
		return NULL;
	}
	
	memset(tempindata, 0, newsize);
	

	memcpy(tempindata, indata, data_size);
	
	tempsize = newsize;
	
	#ifdef DEBUG
		printf("   [+] Encrypting data in byte blocks %d.\n", blocksize);
	#endif
	
	while(tempsize > 0)
	{
		rc4_crypt(output, tempindata , blocksize, encctx->key);
	
		//increment pointer by block size
		tempindata += blocksize;
		output += blocksize;
		
		//Decrement temp buffer size
		tempsize -= blocksize;
	}
	#ifdef DEBUG
	printf("[+] Finished encrypting the datas! New encrypted data size %d\n", newsize);
	#endif

	encctx->encrypted = true;

	rc4_encrypt_key_ctf(encctx, kek);

	tempindata -= newsize;
	free(tempindata);
	output -= newsize;
	return output;

}

int encrypt_file_ctf(char *infile, char *outfile, unsigned char key[RC4_KEY_BYTES], unsigned char kek[RC4_KEY_BYTES] )
{
	struct stat st;
	unsigned int size = 0;
	unsigned char *input = NULL;
	unsigned char *output;
	rc4ctx encrypted_ctx;

	if(key == NULL)
	{
		memset(&encrypted_ctx.key, 0x20, RC4_KEY_BYTES);
	}
	else{
		memcpy(&encrypted_ctx.key, key, RC4_KEY_BYTES);
	}
	
	FILE *fp = fopen(infile, "rb");
	FILE *fo = fopen(outfile, "wb");

	if(fp == 0 || fo == 0)
	{
		#ifdef DEBUG
			printf("[*]encrypt_file: File handle problem\n");
		#endif
		return -1;
	}

	stat(infile, &st);
	size = st.st_size;

	if(size <= 0)
	{
		#ifdef DEBUG
			fprintf(stderr, "[*]encrypt_file: Stat File size  Error!");
		#endif
		
		fclose(fp);
		fclose(fo);
		return -1;
	}
	

	input = (unsigned char *) calloc(size+1, sizeof(unsigned char));

	if(input == NULL)
	{
		#ifdef DEBUG
			fprintf(stderr, "[*]encrypt_file: Memory Error!");
		#endif
		fclose(fp);
		fclose(fo);
		return -1;
	}


	if(fread(input, sizeof(unsigned char), size, fp) < 0)
	{
		#ifdef DEBUG
			fprintf(stderr, "[*]encrypt_file: fread Error!\n");
		#endif

		fclose(fp);
		fclose(fo);
		return -1;
	}
	

	output = rc4_block_encrypt_ctf(&encrypted_ctx, input, size, kek);

	if(output == NULL)
	{
		return -1;
	}
		
	encrypted_ctx.decrypted_size = size;

	fwrite(&encrypted_ctx, sizeof(rc4ctx), 1, fo);
	fwrite(output, sizeof(unsigned char), encrypted_ctx.encrypted_size, fo);


	fclose(fo);
	fclose(fp);
	return 0;
}

unsigned int rc4_decrypt_key_ctf(rc4ctx *crypt_ctx, unsigned char key[RC4_KEY_BYTES])
{
	unsigned int key_pad_size = 64;
	unsigned char key_enc_buffer[key_pad_size];
	unsigned char *key_ptr = NULL;

	#ifdef DEBUG
		printf("[+] Decrypting with key\n");
		if(key == NULL)
		{
			unsigned char key[RC4_KEY_BYTES];
			memset(&key, 0x2D, RC4_KEY_BYTES);
		}
	#endif

	rc4_crypt(key_enc_buffer, crypt_ctx->key_pad_check, RC4_KEY_BYTES, key);
	
	
	if(check_padding(key_enc_buffer, RC4_KEY_BYTES))
	{
		#ifdef DEBUG
			printf("[+] Good padding!\n");
		#endif 

		rc4_crypt(key_enc_buffer, crypt_ctx->key, RC4_KEY_BYTES, key);
		memcpy(crypt_ctx->key, key_enc_buffer, RC4_KEY_BYTES);
		return 0;
	}
	else
	{
		#ifdef DEBUG
			printf("[*] Bad padding\n");
		#endif

		return -1;
	}

}

unsigned char * rc4_block_decrypt_ctf(rc4ctx *ctxdec, unsigned char* indata, unsigned char kek[RC4_KEY_BYTES])
{

	unsigned char *newBuffer = NULL;
	unsigned int blocksize = RC4_BLOCK_SIZE_BYTES;
	unsigned int size_wout_padding = 0;
	unsigned int data_size;
	unsigned int newsize = 0;
	unsigned int tempsize = 0;
	unsigned char *tempindata = 0;
	unsigned char *output = NULL;
	unsigned char *output_nopadding = NULL;

	#ifdef DEBUG
		printf("[+] rc4_block_decrypt_ctf.\n");
	#endif 
	rc4_decrypt_key_ctf(ctxdec, kek);

	size_wout_padding = ctxdec->encrypted_size - ctxdec->padding;

	#ifdef DEBUG
		printf("[+] Size of encrypted data %d - Size of padding %d  = %d \n", ctxdec->encrypted_size, ctxdec->padding, size_wout_padding );
	#endif
	
	if(size_wout_padding <= 0)
	{
		#ifdef DEBUG
			printf("%s\n", "[*]rc4_block_decrypt: Size zero for original decrypted size\n" );
		#endif
		return NULL;
	}

	output = (unsigned char *) calloc(ctxdec->encrypted_size, sizeof(unsigned char));
	output_nopadding = (unsigned char *) calloc(size_wout_padding, sizeof(unsigned char));
	
	if((output == NULL) || (output_nopadding == NULL))
	{
		#ifdef DEBUG
			printf("%s\n", "[*]rc4_block_decrypt: Memory allocation error\n " );
		#endif 
		return NULL;
	}		
	tempsize = ctxdec->encrypted_size;

	while(tempsize > 0)
	{
		rc4_crypt(output, indata , blocksize, ctxdec->key);
	
		//increment pointer by block size
		indata += blocksize;
		output += blocksize;
		
		//Decrement temp buffer size
		tempsize -= blocksize;
	}

	output -= ctxdec->encrypted_size;// + RC4_KEY_BYTES;
	
	memcpy(output_nopadding, output, size_wout_padding);
	free(output); 

	return output_nopadding;

}

unsigned char * rc4_block_decrypt_ctf_BADVERSION(rc4ctx *ctxdec, unsigned char* indata, unsigned char kek[RC4_KEY_BYTES])
{

	unsigned char *newBuffer = NULL;
	unsigned int blocksize = RC4_BLOCK_SIZE_BYTES;
	unsigned int size_wout_padding = 0;
	unsigned int data_size;
	unsigned int newsize = 0;
	unsigned int tempsize = 0;
	unsigned char *tempindata = 0;
	unsigned char *output = NULL;
	unsigned char *output_nopadding = NULL;
	unsigned char BAD_key[RC4_KEY_BYTES];

	#ifdef DEBUG
		printf("[+] rc4_block_decrypt_ctf_BADVERSION.\n");
	#endif

	rc4_decrypt_key_ctf(ctxdec, kek);

	size_wout_padding = ctxdec->encrypted_size - ctxdec->padding;

	#ifdef DEBUG
		printf("[+] Size of encrypted data %d - Size of padding %d  = %d \n", ctxdec->encrypted_size, ctxdec->padding, size_wout_padding );
	#endif
	/******	
	Copying key over to BAD_key causes it to be 
	off by one byte because of the sturct layout
	******/
	memcpy(BAD_key, (ctxdec->key+1), RC4_KEY_BYTES);	
	
	if(size_wout_padding <= 0)
	{
		#ifdef DEBUG
			printf("%s\n", "[*]rc4_block_decrypt: Size zero for original decrypted size\n" );
		#endif
		
		return NULL;
	}

	output = (unsigned char *) calloc(ctxdec->encrypted_size, sizeof(unsigned char));
	output_nopadding = (unsigned char *) calloc(size_wout_padding, sizeof(unsigned char));
	
	if((output == NULL) || (output_nopadding == NULL))
	{
		#ifdef DEBUG
			printf("%s\n", "[*]rc4_block_decrypt: Memory allocation error\n " );
		#endif
		return NULL;
	}	
	
	tempsize = ctxdec->encrypted_size;

	while(tempsize > 0)
	{

	     rc4_crypt(output, indata , blocksize, BAD_key);

		indata += blocksize;
		output += blocksize;
		
		tempsize -= blocksize;
	}

	output -= ctxdec->encrypted_size;
	
	memcpy(output_nopadding, output, size_wout_padding);
	free(output); 

	return output_nopadding;

}

/* decrypt_file_ctf for testing */
int decrypt_file_ctf(char *infile, char *outfile, unsigned char kek[RC4_KEY_BYTES]) 
{
	struct stat st;
	unsigned int fsize = 0;
	unsigned char *input = NULL;
	unsigned char *output;
	rc4ctx decrypted_ctx;

	FILE *fp = fopen(infile, "rb");
	FILE *fo = fopen(outfile, "wb");

	if(fp == 0 || fo == 0){
		#ifdef DEBUG
			printf("File handle problem\n");
		#endif
		return -1;
	}

	stat(infile, &st);
	fsize = st.st_size;

	if(fread(&decrypted_ctx, 1, sizeof(rc4ctx), fp) < 0)
	{
		#ifdef DEBUG
			fprintf(stderr, "fread Error!\n");
		#endif

		fclose(fp);
		fclose(fo);
		return -1;
	}



	if((decrypted_ctx.encrypted_size + sizeof(rc4ctx)) != fsize)
	{
		#ifdef DEBUG
			fprintf(stderr, "Stat file size: %d Encrypted file size: %d\n", fsize, decrypted_ctx.encrypted_size);
			fprintf(stderr, "Decrypted context error esize %d dsize %d padding %d, bool_encrypted %d\n",
			decrypted_ctx.encrypted_size, 
			decrypted_ctx.decrypted_size,
			decrypted_ctx.padding, 
			decrypted_ctx.encrypted);
		#endif 
		fclose(fp);
		fclose(fo);
		return -1;
	}

	input = (unsigned char *) calloc(decrypted_ctx.encrypted_size, sizeof(unsigned char));

	if(input == NULL)
	{
		#ifdef DEBUG
			fprintf(stderr, "Memory Error!");
		#endif
		fclose(fp);
		fclose(fo);
		return -1;
	}

	if(fread(input, sizeof(unsigned char), decrypted_ctx.encrypted_size, fp) < 0)
	{
		#ifdef DEBUG
			fprintf(stderr, "fread Error!\n");
		#endif 
		fclose(fp);
		fclose(fo);
		return -1;
	}
	#ifdef DEBUG
    	printf("Input data bytes: ->");
    	pretty_print_bytes(input, 64);
    #endif 
	output = rc4_block_decrypt_ctf(&decrypted_ctx, input, kek);

	fwrite(output, sizeof(unsigned char), decrypted_ctx.decrypted_size, fo);

	fclose(fo);
	fclose(fp);

return 0;
}

unsigned char * rc4_block_encrypt(rc4ctx *encctx, unsigned char* indata, unsigned int data_size)
{
	
	unsigned char *newBuffer = NULL;
	unsigned int blocksize = RC4_BLOCK_SIZE_BYTES;
	unsigned int paddingsize = data_size % blocksize;
	unsigned int origsize = data_size;
	unsigned int newsize = 0;
	unsigned int tempsize = 0;
	unsigned char* tempindata = 0;
	unsigned char* output = NULL;

	#ifdef DEBUG
		printf("[+] rc4_block_encrypt.\n");
		printf("   [+] Getting padding size.\n");
	#endif

	encctx->decrypted_size = data_size;

	if(paddingsize > 0)
	{
		newsize = (blocksize  - paddingsize) + data_size;
	}
	else
	{
		newsize = data_size;
	}
	
	encctx->padding = blocksize  - paddingsize;
	
	if(newsize <= 0)
	{
		#ifdef DEBUG
			fprintf(stderr, "[*] rc4_block_encrypt: Encrypted size is zero\n");
		#endif

		return NULL;
	}
	
	encctx->encrypted_size = newsize;
	
	#ifdef DEBUG
		printf("   [+] Padding size %d\n", encctx->padding);
		printf("   [+] Encrypted size %d\n", encctx->encrypted_size);
		printf("   [+] Original Decrypted size %d\n", encctx->decrypted_size);
	#endif


	output = (unsigned char *) malloc(newsize * sizeof(unsigned char));
	if(output == NULL)
	{
		#ifdef DEBUG
			fprintf(stderr, "[*]rc4_block_encrypt: Memory error rc4_block_encrypt!\n");
		#endif

		return NULL;
	}
		memset(output, 0, newsize);

	tempindata = (unsigned char *) malloc(newsize * sizeof(unsigned char)); //, sizeof(unsigned char));
	if(tempindata == NULL)
	{
		#ifdef DEBUG
			fprintf(stderr, "[*] rc4_block_encrypt: Memory error rc4_block_encrypt!\n");
		#endif
		return NULL;
	}
	
	memset(tempindata, 0, newsize);
	

	memcpy(tempindata, indata, data_size);
	
	tempsize = newsize;
	
	#ifdef DEBUG
		printf("   [+] Encrypting data in byte blocks %d.\n", blocksize);
	#endif
	
	while(tempsize > 0)
	{
		rc4_crypt(output, tempindata , blocksize, encctx->key);
	
		//increment pointer by block size
		tempindata += blocksize;
		output += blocksize;
		
		//Decrement temp buffer size
		tempsize -= blocksize;
	}

	#ifdef DEBUG
		printf("[+] Finished encrypting the datas! New encrypted data size %d\n", newsize);
	#endif 
	encctx->encrypted = true;

	tempindata -= newsize;
	free(tempindata);

	output -= newsize;

	return output;

}

unsigned char * rc4_block_decrypt(rc4ctx *ctxdec, unsigned char* indata)
{

	unsigned char *newBuffer = NULL;
	unsigned int blocksize = RC4_BLOCK_SIZE_BYTES;
	unsigned int size_wout_padding = 0;
	unsigned int data_size;
	unsigned int newsize = 0;
	unsigned int tempsize = 0;
	unsigned char *tempindata = 0;
	unsigned char *output = NULL;
	unsigned char *output_nopadding = NULL;

	#ifdef DEBUG
		printf("[+] rc4_block_decrypt.\n");
	#endif
	size_wout_padding = ctxdec->encrypted_size - ctxdec->padding;

	#ifdef DEBUG
		printf("[+] Size of encrypted data %d - Size of padding %d  = %d \n", ctxdec->encrypted_size, ctxdec->padding, size_wout_padding );
	#endif

	if(size_wout_padding <= 0)
	{
		#ifdef DEBUG
			fprintf(stderr, "%s\n", "[*]rc4_block_decrypt: Size zero for original decrypted size\n" );
		#endif
		return NULL;
	}

	output = (unsigned char *) calloc(ctxdec->encrypted_size, sizeof(unsigned char));
	output_nopadding = (unsigned char *) calloc(size_wout_padding, sizeof(unsigned char));
	
	if((output == NULL) || (output_nopadding == NULL))
	{
		#ifdef DEBUG
			fprintf(stderr, "%s\n", "[*]rc4_block_decrypt: Memory allocation error\n " );
		#endif

		return NULL;
	}		
	tempsize = ctxdec->encrypted_size;

	while(tempsize > 0)
	{
		rc4_crypt(output, indata , blocksize, ctxdec->key);
	
		indata += blocksize;
		output += blocksize;
		
		tempsize -= blocksize;
	}

	output -= ctxdec->encrypted_size;
	
	memcpy(output_nopadding, output, size_wout_padding);
	free(output); 

	return output_nopadding;

}

void rc4_crypt(unsigned char *output, unsigned char *input, unsigned int length, unsigned char key[RC4_KEY_BYTES])
{
	unsigned char sbox[256];
	unsigned char ti, tj;
	int i, j;
	unsigned int k;

	for (i = 0; i < 256; i++) 
	{
		sbox[i] = (unsigned char)(i & 0xFF);
	}

	for (i = 0, j = 0; i < 256; i++)
	{
		ti = sbox[i];
		j = (key[i % RC4_KEY_BYTES] + ti + j) & 0xFF;
		sbox[i] = sbox[j];
		sbox[j] = ti;
	}

	i = j = 0;

	for (k = 0; k < length; k++)
	{
		i += 1;
		i &= 0xFF;
        ti = sbox[i];
		j += ti;
		j &= 0xFF;
		tj = sbox[j];
                        
		sbox[i] = tj;
		sbox[j] = ti;
		
		output[k] = input[k] ^ sbox[(ti + tj) & 0xFF];
	}
}


int encrypt_file(char *infile, char *outfile, unsigned char key[RC4_KEY_BYTES])
{
	struct stat st;
	unsigned int size = 0;
	unsigned char *input = NULL;
	unsigned char *output;
	rc4ctx encrypted_ctx;

	if(key == NULL)
	{
		memset(&encrypted_ctx.key, 0x20, RC4_KEY_BYTES);
	}
	else{
		memcpy(&encrypted_ctx.key, key, RC4_KEY_BYTES);
	}
	
	FILE *fp = fopen(infile, "rb");
	FILE *fo = fopen(outfile, "wb");

	if(fp == 0 || fo == 0)
	{
		#ifdef DEBUG
			printf("[*]encrypt_file: File handle problem\n");
		#endif
		return -1;
	}

	stat(infile, &st);
	size = st.st_size;

	if(size <= 0)
	{
		#ifdef DEBUG
			fprintf(stderr, "[*]encrypt_file: Stat File size  Error!");
		#endif
		fclose(fp);
		fclose(fo);
		return -1;
	}
	

	input = (unsigned char *) calloc(size+1, sizeof(unsigned char));

	if(input == NULL)
	{
		#ifdef DEBUG
			fprintf(stderr, "[*]encrypt_file: Memory Error!");
		#endif
		fclose(fp);
		fclose(fo);
		return -1;
	}


	if(fread(input, sizeof(unsigned char), size, fp) < 0)
	{
		#ifdef DEBUG
			fprintf(stderr, "[*]encrypt_file: fread Error!\n");
		#endif
		fclose(fp);
		fclose(fo);
		return -1;
	}
	

	output = rc4_block_encrypt(&encrypted_ctx, input, size);

	if(output == NULL)
	{
		return -1;
	}

	encrypted_ctx.decrypted_size = size;

	fwrite(&encrypted_ctx, sizeof(rc4ctx), 1, fo);
	fwrite(output, sizeof(unsigned char), encrypted_ctx.encrypted_size, fo);


	fclose(fo);
	fclose(fp);
	return 0;
}

int decrypt_file(char *infile, char *outfile) //unsigned char key[RC4_KEY_BYTES])
{
	struct stat st;
	unsigned int fsize = 0;
	unsigned char *input = NULL;
	unsigned char *output;
	rc4ctx decrypted_ctx;

	//memset(&encrypted_ctx.key, 0x20, RC4_KEY_BYTES);


	FILE *fp = fopen(infile, "rb");
	FILE *fo = fopen(outfile, "wb");

	if(fp == 0 || fo == 0){

		return -1;
	}



	stat(infile, &st);
	fsize = st.st_size;

	if(fread(&decrypted_ctx, 1, sizeof(rc4ctx), fp) < 0)
	{

		fclose(fp);
		fclose(fo);
		return -1;
	}

	if((decrypted_ctx.encrypted_size + sizeof(rc4ctx)) != fsize)
	{
		#ifdef DEBUG
			fprintf(stderr, "Stat file size: %d Encrypted file size: %d\n", fsize, decrypted_ctx.encrypted_size);
			fprintf(stderr, "Decrypted context error esize %d dsize %d padding %d, bool_encrypted %d\n",
			decrypted_ctx.encrypted_size, 
			decrypted_ctx.decrypted_size,
			decrypted_ctx.padding, 
			decrypted_ctx.encrypted);
		#endif

		fclose(fp);
		fclose(fo);
		return -1;
	}

	input = (unsigned char *) calloc(decrypted_ctx.encrypted_size, sizeof(unsigned char));

	if(input == NULL)
	{

		fclose(fp);
		fclose(fo);
		return -1;
	}

	if(fread(input, sizeof(unsigned char), decrypted_ctx.encrypted_size, fp) < 0)
	{
		fclose(fp);
		fclose(fo);
		return -1;
	}
	

	output = rc4_block_decrypt(&decrypted_ctx, input);

	fwrite(output, sizeof(unsigned char), decrypted_ctx.decrypted_size, fo);

	fclose(fo);
	fclose(fp);

return 0;

}