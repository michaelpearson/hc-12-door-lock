#ifndef __SHA_256_H
#define __SHA_256_H

#include "stm8s.h"

void calc_sha_256(uint8_t hash[32], const void *input, u8 len);

#endif


