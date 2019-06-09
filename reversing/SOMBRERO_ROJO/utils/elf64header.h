#include <stdlib.h>
#include <stdio.h>

typedef unsigned int uint;
typedef unsigned char uint8;
typedef unsigned short int  uint16;
typedef unsigned int uint32;
typedef long long int64;
typedef unsigned long long uint64;

typedef struct {
  uint8     e_ident[16];         /* Magic number and other info */
  uint16    e_type;              /* Object file type */
  uint16    e_machine;           /* Architecture */
  uint32    e_version;           /* Object file version */
  uint64    e_entry;             /* Entry point virtual address */
  uint64    e_phoff;             /* Program header table file offset */
  uint64    e_shoff;             /* Section header table file offset */
  uint32    e_flags;             /* Processor-specific flags */
  uint16    e_ehsize;            /* ELF header size in bytes */
  uint16    e_phentsize;         /* Program header table entry size */
  uint16    e_phnum;             /* Program header table entry count */
  uint16    e_shentsize;         /* Section header table entry size */
  uint16    e_shnum;             /* Section header table entry count */
  uint16    e_shstrndx;          /* Section header string table index */
} Elf64Hdr;