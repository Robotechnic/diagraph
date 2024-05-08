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

float decode_float(uint8_t *buffer) {
	int value = big_endian_decode(buffer, TYPST_INT_SIZE);
	int sign = (value >> 31) & 1;
	int exponent = (value >> 23) & 0xFF;
	int mantissa = value & 0x7FFFFF;
	return (1.0f - 2.0f * sign) * (1.0f + mantissa / 0x800000) * (float)pow(2, exponent - 127);
}

void encode_float(float value, uint8_t *buffer) {
	if (value == 0.0f) {
		big_endian_encode(0, buffer, TYPST_INT_SIZE);
	} else {
		union FloatBuffer {
			float f;
			int i;
		} float_buffer;
		float_buffer.f = value;
		big_endian_encode(float_buffer.i, buffer, TYPST_INT_SIZE);
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

void free_LabelInfo(LabelInfo *s) {
    if (s->label) {
        free(s->label);
    }
    if (s->fontName) {
        free(s->fontName);
    }
}
size_t LabelInfo_size(const void *s){
	return 1 + 1 + strlen(((LabelInfo*)s)->label) + 1 + 1 + TYPST_INT_SIZE + strlen(((LabelInfo*)s)->fontName) + 1 + TYPST_INT_SIZE;
}
int encode_LabelInfo(const LabelInfo *s, uint8_t *__input_buffer, size_t *buffer_len, size_t *buffer_offset) {
    size_t __buffer_offset = 0;    size_t s_size = LabelInfo_size(s);
    if (s_size > *buffer_len) {
        return 2;
    }
    int err;
	(void)err;
    CHAR_PACK(s->native)
    CHAR_PACK(s->html)
    STR_PACK(s->label)
    CHAR_PACK(s->mathMode)
    INT_PACK(s->color)
    STR_PACK(s->fontName)
    FLOAT_PACK(s->fontSize)

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
void free_SizedLabel(SizedLabel *s) {
}
int decode_SizedLabel(uint8_t *__input_buffer, size_t buffer_len, SizedLabel *out, size_t *buffer_offset) {
    size_t __buffer_offset = 0;
    int err;
    (void)err;
    NEXT_CHAR(out->override)
    NEXT_FLOAT(out->width)
    NEXT_FLOAT(out->height)
    *buffer_offset += __buffer_offset;
    return 0;
}
void free_LabelsInfos(LabelsInfos *s) {
    for (size_t i = 0; i < s->labels_len; i++) {
    free_LabelInfo(&s->labels[i]);
    }
    free(s->labels);
}
size_t LabelsInfos_size(const void *s){
	return TYPST_INT_SIZE + list_size(((LabelsInfos*)s)->labels, ((LabelsInfos*)s)->labels_len, LabelInfo_size, sizeof(*((LabelsInfos*)s)->labels));
}
int encode_LabelsInfos(const LabelsInfos *s) {
    size_t buffer_len = LabelsInfos_size(s);
    INIT_BUFFER_PACK(buffer_len)
    int err;
	(void)err;
    INT_PACK(s->labels_len)
    for (size_t i = 0; i < s->labels_len; i++) {
        if ((err = encode_LabelInfo(&s->labels[i], __input_buffer + __buffer_offset, &buffer_len, &__buffer_offset))) {
            return err;
        }
    }

    wasm_minimal_protocol_send_result_to_host(__input_buffer, buffer_len);
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
    for (size_t i = 0; i < s->labels_len; i++) {
    free_Coordinates(&s->labels[i]);
    }
    free(s->labels);
    if (s->svg) {
        free(s->svg);
    }
}
size_t graphInfo_size(const void *s){
	return 1 + TYPST_INT_SIZE + list_size(((graphInfo*)s)->labels, ((graphInfo*)s)->labels_len, Coordinates_size, sizeof(*((graphInfo*)s)->labels)) + strlen(((graphInfo*)s)->svg) + 1;
}
int encode_graphInfo(const graphInfo *s) {
    size_t buffer_len = graphInfo_size(s);
    INIT_BUFFER_PACK(buffer_len)
    int err;
	(void)err;
    CHAR_PACK(s->error)
    INT_PACK(s->labels_len)
    for (size_t i = 0; i < s->labels_len; i++) {
        if ((err = encode_Coordinates(&s->labels[i], __input_buffer + __buffer_offset, &buffer_len, &__buffer_offset))) {
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
    for (size_t i = 0; i < s->labels_len; i++) {
    free_SizedLabel(&s->labels[i]);
    }
    free(s->labels);
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
    NEXT_INT(out->labels_len)
    if (out->labels_len == 0) {
        out->labels = NULL;
    } else {
        out->labels = malloc(out->labels_len * sizeof(SizedLabel));
        if (!out->labels){
            return 1;
        }
        for (size_t i = 0; i < out->labels_len; i++) {
    if ((err = decode_SizedLabel(__input_buffer + __buffer_offset, buffer_len - __buffer_offset, &out->labels[i], &__buffer_offset))){return err;}
        }
    }
    NEXT_STR(out->engine)
    FREE_BUFFER()
    return 0;
}
