#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "emscripten.h"
#include <graphviz/gvplugin.h>
#include <stdint.h>

#define PROTOCOL_FUNCTION __attribute__((import_module("typst_env"))) extern

PROTOCOL_FUNCTION void wasm_minimal_protocol_send_result_to_host(const uint8_t *ptr, size_t len);
PROTOCOL_FUNCTION void wasm_minimal_protocol_write_args_to_buffer(uint8_t *ptr);

extern gvplugin_library_t gvplugin_dot_layout_LTX_library;
extern gvplugin_library_t gvplugin_neato_layout_LTX_library;

extern gvplugin_library_t gvplugin_core_LTX_library;

lt_symlist_t lt_preloaded_symbols[] = {
    {"gvplugin_dot_layout_LTX_library", &gvplugin_dot_layout_LTX_library},
    {"gvplugin_neato_layout_LTX_library", &gvplugin_neato_layout_LTX_library},
    {"gvplugin_core_LTX_library", &gvplugin_core_LTX_library},
    {0, 0}};

#define INIT_BUFFER_UNPACK(buffer_len)                                                             \
    size_t __buffer_offset = 0;                                                                    \
    uint8_t *__input_buffer = malloc((buffer_len));                                                \
    if (!__input_buffer) {                                                                         \
        return 1;                                                                                  \
    }                                                                                              \
    wasm_minimal_protocol_write_args_to_buffer(__input_buffer);

#define NEXT_SIZED_STR(dst, len)                                                                   \
    memcpy((dst), __input_buffer + __buffer_offset, (len));                                        \
    (dst)[(len)] = '\0';                                                                           \
    __buffer_offset += (len);

#define NEXT_STR(dst)                                                                              \
    {                                                                                              \
        int __str_len = strlen((char *)__input_buffer + __buffer_offset);                          \
        (dst) = malloc(__str_len + 1);                                                             \
        strcpy((dst), (char *)__input_buffer + __buffer_offset);                                   \
        __buffer_offset += __str_len + 1;                                                          \
    }

#define NEXT_INT(dst)                                                                              \
    (dst) = big_endian_decode(__input_buffer + __buffer_offset, TYPST_INT_SIZE);                   \
    __buffer_offset += TYPST_INT_SIZE;

#define NEXT_DOUBLE(dst)                                                                           \
    {                                                                                              \
        int __encoded_value;                                                                       \
        NEXT_INT(__encoded_value);                                                                 \
        (dst) = ((double)__encoded_value) / DOUBLE_PRECISION;                                      \
    }

#define FREE_BUFFER()                                                                              \
    free(__input_buffer);                                                                          \
    __input_buffer = NULL;

#endif // PROTOCOL_H