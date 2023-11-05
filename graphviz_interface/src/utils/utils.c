#include "utils/utils.h"

void big_endian_encode(uint8_t *buffer, int value) {
    for (int i = 0; i < sizeof(int); i++) {
        buffer[i] = (value >> (8 * (sizeof(int) - i - 1))) & 0xFF;
    }
}

int big_endian_decode(uint8_t const *buffer, int size) {
    int value = 0;
    for (int i = 0; i < size; i++) {
        value |= buffer[i] << (8 * (size - i - 1));
    }
    return value;
}
