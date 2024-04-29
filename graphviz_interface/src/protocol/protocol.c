#include "protocol.h"
int big_endian_decode(uint8_t const *buffer, int size){
    int value = 0;
    for (int i = 0; i < size; i++) {
        value |= buffer[i] << (8 * (size - i - 1));
    }
    return value;
}

void big_endian_encode(int value, uint8_t *buffer, int size) {
    for (int i = 0; i < sizeof(int); i++) {
        buffer[i] = (value >> (8 * (sizeof(int) - i - 1))) & 0xFF;
    }
}

size_t list_size(void *list, size_t size, size_t (*sf)(const void*), size_t element_size) {
    size_t result = 0;
    for (int i = 0; i < size; i++) {
        result += sf(list + i * element_size);
    }
    return result;
}

size_t int_size(const void* elem) {
    return TYPST_INT_SIZE;
}
size_t float_size(const void *elem) {
    return TYPST_INT_SIZE;
}
size_t bool_size(const void *elem) {
    return TYPST_INT_SIZE;
}
size_t char_size(const void *elem) {
    return 1;
}
size_t string_size(const void *elem) {
    return strlen((char *)elem) + 1;
}

void free_NativeLabel(NativeLabel *s) {
    if (s->label) {
        free(s->label);
    }
}
size_t NativeLabel_size(const void *s){
	return strlen(((NativeLabel*)s)->label) + 1 + 1;
}
int encode_NativeLabel(const NativeLabel *s, uint8_t *__input_buffer, size_t *buffer_len, size_t *buffer_offset) {
    size_t __buffer_offset = 0;    size_t s_size = NativeLabel_size(s);
    if (s_size > *buffer_len) {
        return 2;
    }
    int err;
	(void)err;
    STR_PACK(s->label)
    CHAR_PACK(s->mathMode)

    *buffer_offset += __buffer_offset;
    return 0;
}
void free_SizedLabel(SizedLabel *s) {
    if (s->label) {
        free(s->label);
    }
}
int decode_SizedLabel(uint8_t *__input_buffer, size_t buffer_len, SizedLabel *out, size_t *buffer_offset) {
    size_t __buffer_offset = 0;
    int err;
    (void)err;
    NEXT_STR(out->label)
    NEXT_FLOAT(out->width)
    NEXT_FLOAT(out->height)
    *buffer_offset += __buffer_offset;
    return 0;
}
void free_Coordinates(Coordinates *s) {
}
size_t Coordinates_size(const void *s){
	return TYPST_INT_SIZE + TYPST_INT_SIZE;
}
int encode_Coordinates(const Coordinates *s, uint8_t *__input_buffer, size_t *buffer_len, size_t *buffer_offset) {
    size_t __buffer_offset = 0;    size_t s_size = Coordinates_size(s);
    if (s_size > *buffer_len) {
        return 2;
    }
    int err;
	(void)err;
    FLOAT_PACK(s->x)
    FLOAT_PACK(s->y)

    *buffer_offset += __buffer_offset;
    return 0;
}
void free_overriddenLabels(overriddenLabels *s) {
    for (size_t i = 0; i < s->labels_len; i++) {
    if (s->labels[i]) {
        free(s->labels[i]);
    }
    }
    free(s->labels);
    if (s->dot) {
        free(s->dot);
    }
}
int decode_overriddenLabels(size_t buffer_len, overriddenLabels *out) {
    INIT_BUFFER_UNPACK(buffer_len)
    int err;
    (void)err;
    NEXT_INT(out->labels_len)
    if (out->labels_len == 0) {
        out->labels = NULL;
    } else {
        out->labels = malloc(out->labels_len * sizeof(char*));
        if (!out->labels){
            return 1;
        }
        for (size_t i = 0; i < out->labels_len; i++) {
    NEXT_STR(out->labels[i])
        }
    }
    NEXT_STR(out->dot)
    FREE_BUFFER()
    return 0;
}
void free_graphInfo(graphInfo *s) {
    for (size_t i = 0; i < s->manualLabels_len; i++) {
    free_Coordinates(&s->manualLabels[i]);
    }
    free(s->manualLabels);
    for (size_t i = 0; i < s->nativeLabels_len; i++) {
    free_Coordinates(&s->nativeLabels[i]);
    }
    free(s->nativeLabels);
    if (s->svg) {
        free(s->svg);
    }
}
size_t graphInfo_size(const void *s){
	return 1 + TYPST_INT_SIZE + list_size(((graphInfo*)s)->manualLabels, ((graphInfo*)s)->manualLabels_len, Coordinates_size, sizeof(*((graphInfo*)s)->manualLabels)) + TYPST_INT_SIZE + list_size(((graphInfo*)s)->nativeLabels, ((graphInfo*)s)->nativeLabels_len, Coordinates_size, sizeof(*((graphInfo*)s)->nativeLabels)) + strlen(((graphInfo*)s)->svg) + 1;
}
int encode_graphInfo(const graphInfo *s) {
    size_t buffer_len = graphInfo_size(s);
    INIT_BUFFER_PACK(buffer_len)
    int err;
	(void)err;
    CHAR_PACK(s->error)
    INT_PACK(s->manualLabels_len)
    for (size_t i = 0; i < s->manualLabels_len; i++) {
        if ((err = encode_Coordinates(&s->manualLabels[i], __input_buffer + __buffer_offset, &buffer_len, &__buffer_offset))) {
            return err;
        }
    }
    INT_PACK(s->nativeLabels_len)
    for (size_t i = 0; i < s->nativeLabels_len; i++) {
        if ((err = encode_Coordinates(&s->nativeLabels[i], __input_buffer + __buffer_offset, &buffer_len, &__buffer_offset))) {
            return err;
        }
    }
    STR_PACK(s->svg)

    wasm_minimal_protocol_send_result_to_host(__input_buffer, buffer_len);
    return 0;
}
void free_renderGraph(renderGraph *s) {
    if (s->dot) {
        free(s->dot);
    }
    for (size_t i = 0; i < s->nativeLabels_len; i++) {
    free_SizedLabel(&s->nativeLabels[i]);
    }
    free(s->nativeLabels);
    for (size_t i = 0; i < s->manualLabels_len; i++) {
    free_SizedLabel(&s->manualLabels[i]);
    }
    free(s->manualLabels);
    if (s->engine) {
        free(s->engine);
    }
}
int decode_renderGraph(size_t buffer_len, renderGraph *out) {
    INIT_BUFFER_UNPACK(buffer_len)
    int err;
    (void)err;
    NEXT_FLOAT(out->fontSize)
    NEXT_STR(out->dot)
    NEXT_INT(out->nativeLabels_len)
    if (out->nativeLabels_len == 0) {
        out->nativeLabels = NULL;
    } else {
        out->nativeLabels = malloc(out->nativeLabels_len * sizeof(SizedLabel));
        if (!out->nativeLabels){
            return 1;
        }
        for (size_t i = 0; i < out->nativeLabels_len; i++) {
    if ((err = decode_SizedLabel(__input_buffer + __buffer_offset, buffer_len - __buffer_offset, &out->nativeLabels[i], &__buffer_offset))){return err;}
        }
    }
    NEXT_INT(out->manualLabels_len)
    if (out->manualLabels_len == 0) {
        out->manualLabels = NULL;
    } else {
        out->manualLabels = malloc(out->manualLabels_len * sizeof(SizedLabel));
        if (!out->manualLabels){
            return 1;
        }
        for (size_t i = 0; i < out->manualLabels_len; i++) {
    if ((err = decode_SizedLabel(__input_buffer + __buffer_offset, buffer_len - __buffer_offset, &out->manualLabels[i], &__buffer_offset))){return err;}
        }
    }
    NEXT_STR(out->engine)
    FREE_BUFFER()
    return 0;
}
void free_nativeLabels(nativeLabels *s) {
    for (size_t i = 0; i < s->nativeLabels_len; i++) {
    free_NativeLabel(&s->nativeLabels[i]);
    }
    free(s->nativeLabels);
}
size_t nativeLabels_size(const void *s){
	return TYPST_INT_SIZE + list_size(((nativeLabels*)s)->nativeLabels, ((nativeLabels*)s)->nativeLabels_len, NativeLabel_size, sizeof(*((nativeLabels*)s)->nativeLabels));
}
int encode_nativeLabels(const nativeLabels *s) {
    size_t buffer_len = nativeLabels_size(s);
    INIT_BUFFER_PACK(buffer_len)
    int err;
	(void)err;
    INT_PACK(s->nativeLabels_len)
    for (size_t i = 0; i < s->nativeLabels_len; i++) {
        if ((err = encode_NativeLabel(&s->nativeLabels[i], __input_buffer + __buffer_offset, &buffer_len, &__buffer_offset))) {
            return err;
        }
    }

    wasm_minimal_protocol_send_result_to_host(__input_buffer, buffer_len);
    return 0;
}
