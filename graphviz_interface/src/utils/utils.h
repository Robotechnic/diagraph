#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#define SQRT2 (1.41421356237)

void big_endian_encode(uint8_t *buffer, int value);
int big_endian_decode(uint8_t const *buffer, int size);

#endif // UTILS_H