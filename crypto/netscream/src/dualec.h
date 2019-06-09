#ifndef DUALEC_H
#define DUALEC_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

void dualec_init(FILE* random);
void dualec_generate(uint8_t* buf, size_t size);

#endif
