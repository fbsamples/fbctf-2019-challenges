#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <elf.h>
#include <sys/stat.h>
#include "elf64header.h"



unsigned long get_elf_size(const unsigned char* elf_file);
unsigned char* get_extra_data(const unsigned char* buffer, unsigned long buffer_size, int ext_data_size);
//void read_elf_header(const char* elfFile, const char* outputFile);