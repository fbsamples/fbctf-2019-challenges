#include "rc4block.h"



unsigned char key[RC4_KEY_BYTES] = rand_key;

int main(int argc, char *argv[])
{
	unsigned char half_kek1 = HALF_KEY_ENC_KEY1;
	unsigned char half_kek2 = HALF_KEY_ENC_KEY2;
	unsigned char *tkek;

	tkek = gen_kek_key(HALF_KEY_ENC_KEY1, HALF_KEY_ENC_KEY2);

	if(argc < 4)
	{
		printf("usage: %s <infile> <outfile> [-d | -e]\n", argv[0]);
	}

	if(strcmp(argv[3] , "-e") == 0)
	{
		if(encrypt_file_ctf(argv[1], argv[2], key, tkek) != 0)
		{
			fprintf(stderr, "%s\n", "[*]Encrypt file error" );
			return -1;
		}
		printf("[*] New encrypted file %s\n", argv[2]);
	}

	if(strcmp(argv[3] , "-d") == 0)
	{
		if(decrypt_file_ctf(argv[1], argv[2], tkek) != 0)
		{
			fprintf(stderr, "%s\n", "[*]Decrypt file error\n" );
			return -1;
		}	

		printf("[*] New decrypted file %s\n", argv[2]);
	}

return 0;

}