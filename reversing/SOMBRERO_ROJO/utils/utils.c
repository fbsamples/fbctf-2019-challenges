#include "utils.h"

unsigned long get_elf_size(const unsigned char* elf_file)
{
    Elf64_Ehdr header;

    unsigned long file_size = 0;

    memcpy(&header, elf_file, sizeof(Elf64_Ehdr));


    if(memcmp(header.e_ident, ELFMAG, SELFMAG) != 0){
        return EXIT_FAILURE;
    }

    file_size = header.e_shoff + (header.e_shnum * header.e_shentsize);

    return file_size;
}

unsigned char* get_extra_data(const unsigned char* buffer, unsigned long buffer_size, int ext_data_size)
{
    unsigned char *ext_buffer = NULL;

    ext_buffer = (unsigned char * ) malloc(ext_data_size * sizeof(unsigned char));

    if(ext_buffer == NULL)
    {
        return NULL;
    }

    memcpy(ext_buffer, (buffer + buffer_size), ext_data_size);
    //memcpy(ext_buffer, buffer, ext_data_size);
    #ifdef DEBUG
      pretty_print_bytes(ext_buffer, 64);
    #endif

    return ext_buffer;
}

void pretty_print_bytes(unsigned char *thebytes, unsigned int size)
{
  printf("{");
  for(int i = 0; i < size; i++)
  {
    printf("0x%02x, ", thebytes[i]);
  }
  printf("}");
  printf("\n");
}

int check_if_elf(unsigned char * elf_data)
{
  if(elf_data[0] == 0x7f &&
       elf_data[1] == 'E' &&
       elf_data[2] == 'L' &&
       elf_data[3] == 'F') {
        return 1;
       }
  return 0;
}