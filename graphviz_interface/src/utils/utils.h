#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

void big_endian_encode(uint8_t *buffer, int value);
int big_endian_decode(uint8_t *buffer);

#endif // UTILS_H