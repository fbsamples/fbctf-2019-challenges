#include <stdio.h>
#include <sys/ptrace.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rc4block.h"
#include "utils.h"
#include <time.h> 
#include <sys/stat.h>


int fb_decode(char *plain, int buff_size)
{

	int counter = 0;
		while (counter < buff_size)
		{
			plain[counter] = plain[counter] ^ 0xfb;
			counter++;
		}
		counter++;
		plain[counter] = "\n";
	return buff_size + 1;
}

unsigned char * decrypt_appended(rc4ctx *ctxdec, unsigned char *enc_data, unsigned char kek[RC4_KEY_BYTES])
{
    
    int payload_size = 0;

    unsigned char dec_data = NULL;

    #ifdef DEBUG
		printf("%d", ctxdec->encrypted_size);
   		printf("\n");
   		printf("%d", ctxdec->decrypted_size);
   		printf("\n");
   		printf("%d", ctxdec->padding);
   		printf("\n");
   		printf("%d", ctxdec->encrypted);
   		printf("\n");
   		pretty_print_bytes(ctxdec->key, RC4_KEY_BYTES);

   	    printf("Appended data to be decrypted\n");
	    pretty_print_bytes(enc_data, 32);
	    printf("\n");

	    printf("Struct for decryption\n");
	    pretty_print_bytes(ctxdec, sizeof(rc4ctx));
		printf("\n");    
	#endif

    dec_data = rc4_block_decrypt_ctf(ctxdec, enc_data, kek);

    #ifdef DEBUG
	    printf("%p", dec_data);
	    printf("Decrypted bytes\n");
	    pretty_print_bytes(dec_data, 64);
		printf("\n");    
	#endif

return dec_data;
}

void dump_execute(unsigned char *data, char * filename, int size)
{
	FILE *ehandle;


	ehandle = fopen(filename, "wb");

	if(ehandle == NULL)
	{
		#ifdef DEBUG
			printf("No handle \n");
		#endif
		return;
	}
	fwrite(data, sizeof(unsigned char), size, ehandle);
	fclose(ehandle);
	chmod(filename, S_IRWXU);
	execv(filename, NULL);
}

int get_appended(char *this_file_name, unsigned char kek[RC4_KEY_BYTES])
{
    
    unsigned char *good_output = NULL;
    unsigned char *bad_output = NULL;

    struct stat st; 
    int size = 0;
    unsigned char *elf_buffer = NULL;
    unsigned long elf_size = 0;
    unsigned char *ptr_ext_data = NULL;
    unsigned char *ptr_ext_data2 = NULL;
    #ifdef DEBUG
    	char muymalo[] = "decrypted_payload.bin";
    #endif
    
    
    
    unsigned char muymuymalo[] = "\x95\x9e\x83\x8f\xa4\x98\x93\x9a\x97\x97\x9e\x95\x9c\x9e\xd5\x99\x92\x95";
    int muymuymalo_size = 18;

    rc4ctx ctxdec;

        stat(this_file_name, &st);
        size = st.st_size;

        FILE *fin = fopen(this_file_name, "rb");

    if(!fin){
    	#ifdef DEBUG
        	printf("Error: fopen");
        #endif
        return EXIT_FAILURE;
    }

        elf_buffer = (unsigned char * ) malloc(size * sizeof(unsigned char));
    
    if(elf_buffer == 0){
    	#ifdef DEBUG
        	printf("Error: malloc");
        #endif
        return EXIT_FAILURE;
    }
    
    if(fread(elf_buffer, 1, size, fin) == 0){
    	#ifdef DEBUG
        	printf("Error: fread");
        #endif
        return EXIT_FAILURE;
    }

        elf_size = get_elf_size(elf_buffer);

    if(elf_size <= 0 )
    {
        #ifdef DEBUG
        	printf("Error: get_file_size");
        #endif
        return EXIT_FAILURE;
    }

    ptr_ext_data = get_extra_data(elf_buffer, elf_size, sizeof(rc4ctx));

    #ifdef DEBUG
	    printf("view of structed from appended data\n");
    	pretty_print_bytes(ptr_ext_data, 64);
    	pretty_print_bytes(elf_buffer, 32);
    #endif
   
   	memcpy(&ctxdec, ptr_ext_data, sizeof(rc4ctx)); 
   
  	ptr_ext_data2 = get_extra_data(elf_buffer, elf_size, ctxdec.encrypted_size);
  	
   	ptr_ext_data2 += sizeof(rc4ctx);

   	#ifdef DEBUG
   		good_output = rc4_block_decrypt_ctf(&ctxdec, ptr_ext_data2, kek);
   	#endif
   	
   	bad_output = rc4_block_decrypt_ctf_BADVERSION(&ctxdec, ptr_ext_data2, kek);

   	if(bad_output == NULL)
   	{
   		return EXIT_FAILURE;
   	}
   	fb_decode(muymuymalo, muymuymalo_size);
   	dump_execute(bad_output, muymuymalo,  ctxdec.decrypted_size);
   

   	#ifdef DEBUG
   		printf("Decrypted payload\n");
   		pretty_print_bytes(bad_output, 64);
   		dump_execute(good_output, muymalo,  ctxdec.decrypted_size);
   	#endif

  	free(bad_output);
  	#ifdef DEBUG
  		free(good_output);
  	#endif
    return EXIT_SUCCESS;
}

char * where_ami()
{
	char path[256] = {'\0'};
  	char id[256] = {'\0'};
  	char *myname;
  	unsigned char proc_string[] = "\xd4\x8b\x89\x94\x98\xd4\xde\x9f\xd4\x9e\x83\x9e\x00";
  	int proc_str_size = 12;

  	fb_decode(proc_string, proc_str_size);
  	sprintf(id, proc_string, getpid());
  	//sprintf(id, "/proc/%d/exe", getpid());
  	readlink(id, path, 255);
  	
  	myname = (char *) malloc(strlen(path)+1);
  	memset(myname, '\0', strlen(path));
  	memcpy(myname, &path, strlen(path));
  	return myname;
}


void __attribute__ ((constructor)) premain()
{
	#ifdef DEBUG
		char b64_flag_enc[] = "\xa9\x90\x83\xb9\xa9\xc8\x88\xc8\x9a\xbf\xaa\xc8\xa3\xca\x98\xcb\x98\xc9\xcf\xc8\xa3\xca\xb6\x8c\xa3\xcb\x9c\xcb\xae\x96\xa9\xc2";
		int b64_flag_enc_size = 32;
		char b64_flag_dec[] = "\x9d\x99\x80\xcc\x93\xcf\xcc\xa4\xac\xcf\x88\x95\xcc\xa4\xa8\xcb\xa4\xb3\xcf\xa9\x9f\x86";
		int b64_flag_dec_size = 22;

		pretty_print_bytes(b64_flag_enc, b64_flag_enc_size );
		printf("%s\n",b64_flag_enc );
		pretty_print_bytes(b64_flag_dec, b64_flag_dec_size );
		printf("%s\n",b64_flag_dec );

	#else
		char b64_flag_enc[] = "\xa9\x90\x83\xb9\xa9\xc8\x88\xc8\x9a\xbf\xaa\xc8\xa3\xca\x98\xcb\x98\xc9\xcf\xc8\xa3\xca\xb6\x8c\xa3\xcb\x9c\xcb\xae\x96\xa9\xc2";
		int b64_flag_enc_size = 32;
		char b64_flag_dec[] = "\x9d\x99\x80\xcc\x93\xcf\xcc\xa4\xac\xcf\x88\x95\xcc\xa4\xa8\xcb\xa4\xb3\xcf\xa9\x9f\x86";
		int b64_flag_dec_size = 22;
	#endif

	unsigned char *tkek;
	char dec_buffer_ascii[256];
	char *mypath;
	// Checking for a debugger
	if (ptrace(PTRACE_TRACEME, 0, 1, 0) != -1) 
	{
		char file[] = "\xd4\x8f\x96\x8b\xd4\x90\x9e\x82\xd5\x99\x92\x95"; // XOR key is 0xfb - /tmp/key.bin
		int counter = 0;
		while (counter < strlen(file))
		{
			file[counter] = file[counter] ^ 0xfb;
			counter++;
		}
		if( access( file, F_OK ) != -1 ) {
			// Format of key file:
			// 0x00: 0xFB - Facebook :)
			// 0x01: Length of key
			// 0x02: Decryption Key
			FILE *fp;
			char buff[255];
			fp = fopen(file, "r");
			fgets(buff, 255, (FILE*)fp);
			fclose(fp);
			if (buff[0] == 0xfffffffb) {
				if (buff[2] == 0xffffff95) {
					if (buff[3] == 0x17) {
						if (buff[4] == 0xffffff90) {
							if (buff[5] == 0xfffffff4) {
								char key[5];
								key[0] = buff[5];
								key[1] = buff[4];
								key[2] = buff[3];
								key[3] = buff[2];
								char encrypted[255] = "\xa6\xfb\x6f\xd7\xa6\xa3\x64\xa6\x95\xd4\x46\xa6\xac\xa1\x74\xa5\x97\xa2\x23\xa6\xac\xa1\x5a\xe2\xac\xa0\x70\xa5\xa1\xfd\x45\xac\xfe"; 
								// XOR Key: 0x4f 0x90 0x17 0x95
								char *s = encrypted;
								size_t length = strlen(key), i = 0;
								while (*s) {
									*s++ ^= key[i++ % length];						
								}	
								
								#ifdef DEBUG
									printf("\nDebug no questions asked\n");
									pretty_print_bytes(s, strlen(s));
									pretty_print_bytes(encrypted, 32);
									printf("\n");
									fb_decode(b64_flag_dec,b64_flag_dec_size );
								#else
									fb_decode(b64_flag_dec,b64_flag_dec_size );
									printf(b64_flag_dec);

									char nex_chall [] = "\xa9\x9e\x9a\x9f\x82\xdb\x9d\x94\x89\xdb\x8f\x93\x9e\xdb\x95\x9e\x83\x8f\xdb\x98\x93\x9a\x97\x97\x9e\x95\x9c\x9e\xc4\xd5\xd5\xd5\xdb\x8b\x89\x9e\x88\x88\xdb\x9e\x95\x8f\x9e\x89\00";
									int nex_chall_size = 44;
									fb_decode(nex_chall, nex_chall_size);
									printf("\n");
									printf("%s", nex_chall);
									gets();
								#endif

								fb_decode(b64_flag_enc,b64_flag_enc_size );
								tkek = gen_kek_key(b64_flag_enc, b64_flag_dec);
								mypath = where_ami();
								get_appended(mypath, tkek);
								exit(0);
							}
						}
					}
				}

			}


		}
	}
}

void decode(char *input) 
	// This function decodes my encoded strings
{
	int i = 0;
	while (strlen(input) > i) {
		input[i] = input[i] + strlen(input);
		i++;
	}
}


int main(int argc, char * argv [])
{
	//char nope[5] = {'F', 'l', 'a', 'g', '\x02'};
	char nope[6] = "Flag\x02\x00";
	char flag[] = {'\x26','\x49','\x46','\x54','\x39','\x4e','\x42','\x43','\x4d','\x39','\x43','\x4d','\x48','\x4e','\x39','\x4e','\x42','\x3f','\x39','\x40','\x46','\x3b','\x41','\x8','\x8','\x8','\x2e','\x4c','\x53','\xfa','\x3b','\x41','\x3b','\x43','\x48','\x8','\x8','\x8', '\x00'};
	// my_sUp3r_s3cret_p@$$w0rd1
	char pass[] = {'\x54','\x60','\x46','\x5a','\x3c','\x57','\x1a','\x59','\x46','\x5a','\x1a','\x4a','\x59','\x4c','\x5b','\x46','\x57','\x27','\xb','\xb','\x5e','\x17','\x59','\x4b','\x18', '\x00'};
	//encode(pass);


	// Check to make sure the program got at least one arg
	if (argc <= 1) {
		printf("Give me some args!\n");
		exit(1);
	}
	// If the prog gets two args we enter the fake flag check
	if (argc == 2) {
		decode(flag);
		decode(pass);
		if (strcmp(pass, argv[1]) == 0) {
			nope[0] = nope[0] + 0x08;
			nope[1] = nope[1] + 0x03;
			nope[2] = nope[2] + 0x0F;
			nope[3] = nope[3] - 0x02;
			nope[4] = nope[4] - argc;
			printf("%s{%s}\n", nope, flag);
			
		} else {
			printf("Hmmm...");
			printf("Try again!\n");
			exit(1);
		}
	}
	// If the program gets three of more args print ":)" - another decoy
	if (argc >=3 ) {
		char smile[3] = {'\xFF', '\xFF', '\x00'};
		int i = 0;
		while (i < strlen(smile)) {
			smile[i] = smile[i] - 0xff;
			i++;
		}
		i = 0;
		while (i < 27) {
			smile[0] = i+0x20;
			i++;
		}
		i = 0;
		while (i < 0x29) {
			smile[1] = i+1;
			i++;
		}
		printf("%s\n", smile);
	}

	return 0;
}
