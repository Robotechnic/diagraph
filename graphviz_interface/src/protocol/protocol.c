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
	if (value == 0) {
		return 0.0f;
	}
	union FloatBuffer {
		float f;
		int i;
	} float_buffer;
	float_buffer.i = value;
	return float_buffer.f;
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

void free_ClusterLabelInfo(ClusterLabelInfo *s) {
    if (s->name) {
        free(s->name);
    }
    if (s->label) {
        free(s->label);
    }
    if (s->font_name) {
        free(s->font_name);
    }
}
size_t ClusterLabelInfo_size(const void *s){
	return 1 + 1 + strlen(((ClusterLabelInfo*)s)->name) + 1 + strlen(((ClusterLabelInfo*)s)->label) + 1 + 1 + TYPST_INT_SIZE + strlen(((ClusterLabelInfo*)s)->font_name) + 1 + TYPST_INT_SIZE;
}
int encode_ClusterLabelInfo(const ClusterLabelInfo *s, uint8_t *__input_buffer, size_t *buffer_len, size_t *buffer_offset) {
    size_t __buffer_offset = 0;    size_t s_size = ClusterLabelInfo_size(s);
    if (s_size > *buffer_len) {
        return 2;
    }
    int err;
	(void)err;
    CHAR_PACK(s->native)
    CHAR_PACK(s->html)
    STR_PACK(s->name)
    STR_PACK(s->label)
    CHAR_PACK(s->math_mode)
    INT_PACK(s->color)
    STR_PACK(s->font_name)
    FLOAT_PACK(s->font_size)

    *buffer_offset += __buffer_offset;
    return 0;
}
void free_SizedClusterLabel(SizedClusterLabel *s) {
}
int decode_SizedClusterLabel(uint8_t *__input_buffer, size_t buffer_len, SizedClusterLabel *out, size_t *buffer_offset) {
    size_t __buffer_offset = 0;
    int err;
    (void)err;
    NEXT_FLOAT(out->width)
    NEXT_FLOAT(out->height)
    *buffer_offset += __buffer_offset;
    return 0;
}
void free_SizedNodeLabel(SizedNodeLabel *s) {
}
int decode_SizedNodeLabel(uint8_t *__input_buffer, size_t buffer_len, SizedNodeLabel *out, size_t *buffer_offset) {
    size_t __buffer_offset = 0;
    int err;
    (void)err;
    NEXT_CHAR(out->override)
    NEXT_CHAR(out->xoverride)
    NEXT_FLOAT(out->width)
    NEXT_FLOAT(out->height)
    NEXT_FLOAT(out->xwidth)
    NEXT_FLOAT(out->xheight)
    *buffer_offset += __buffer_offset;
    return 0;
}
void free_NodeLabelInfo(NodeLabelInfo *s) {
    if (s->name) {
        free(s->name);
    }
    if (s->label) {
        free(s->label);
    }
    if (s->xlabel) {
        free(s->xlabel);
    }
    if (s->font_name) {
        free(s->font_name);
    }
}
size_t NodeLabelInfo_size(const void *s){
	return 1 + 1 + strlen(((NodeLabelInfo*)s)->name) + 1 + strlen(((NodeLabelInfo*)s)->label) + 1 + 1 + 1 + strlen(((NodeLabelInfo*)s)->xlabel) + 1 + 1 + TYPST_INT_SIZE + strlen(((NodeLabelInfo*)s)->font_name) + 1 + TYPST_INT_SIZE;
}
int encode_NodeLabelInfo(const NodeLabelInfo *s, uint8_t *__input_buffer, size_t *buffer_len, size_t *buffer_offset) {
    size_t __buffer_offset = 0;    size_t s_size = NodeLabelInfo_size(s);
    if (s_size > *buffer_len) {
        return 2;
    }
    int err;
	(void)err;
    CHAR_PACK(s->native)
    CHAR_PACK(s->html)
    STR_PACK(s->name)
    STR_PACK(s->label)
    CHAR_PACK(s->math_mode)
    CHAR_PACK(s->override_xlabel)
    STR_PACK(s->xlabel)
    CHAR_PACK(s->xlabel_math_mode)
    INT_PACK(s->color)
    STR_PACK(s->font_name)
    FLOAT_PACK(s->font_size)

    *buffer_offset += __buffer_offset;
    return 0;
}
void free_OverrideLabel(OverrideLabel *s) {
    if (s->label) {
        free(s->label);
    }
}
int decode_OverrideLabel(uint8_t *__input_buffer, size_t buffer_len, OverrideLabel *out, size_t *buffer_offset) {
    size_t __buffer_offset = 0;
    int err;
    (void)err;
    NEXT_STR(out->label)
    NEXT_CHAR(out->content)
    NEXT_CHAR(out->xlabel)
    *buffer_offset += __buffer_offset;
    return 0;
}
void free_NodeCoordinates(NodeCoordinates *s) {
}
size_t NodeCoordinates_size(const void *s){
	return TYPST_INT_SIZE + TYPST_INT_SIZE + TYPST_INT_SIZE + TYPST_INT_SIZE;
}
int encode_NodeCoordinates(const NodeCoordinates *s, uint8_t *__input_buffer, size_t *buffer_len, size_t *buffer_offset) {
    size_t __buffer_offset = 0;    size_t s_size = NodeCoordinates_size(s);
    if (s_size > *buffer_len) {
        return 2;
    }
    int err;
	(void)err;
    FLOAT_PACK(s->x)
    FLOAT_PACK(s->y)
    FLOAT_PACK(s->xx)
    FLOAT_PACK(s->xy)

    *buffer_offset += __buffer_offset;
    return 0;
}
void free_ClusterCoordinates(ClusterCoordinates *s) {
}
size_t ClusterCoordinates_size(const void *s){
	return TYPST_INT_SIZE + TYPST_INT_SIZE;
}
int encode_ClusterCoordinates(const ClusterCoordinates *s, uint8_t *__input_buffer, size_t *buffer_len, size_t *buffer_offset) {
    size_t __buffer_offset = 0;    size_t s_size = ClusterCoordinates_size(s);
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
void free_graphInfo(graphInfo *s) {
    for (size_t i = 0; i < s->labels_len; i++) {
    free_NodeCoordinates(&s->labels[i]);
    }
    free(s->labels);
    for (size_t i = 0; i < s->cluster_labels_len; i++) {
    free_ClusterCoordinates(&s->cluster_labels[i]);
    }
    free(s->cluster_labels);
    if (s->svg) {
        free(s->svg);
    }
}
size_t graphInfo_size(const void *s){
	return 1 + TYPST_INT_SIZE + list_size(((graphInfo*)s)->labels, ((graphInfo*)s)->labels_len, NodeCoordinates_size, sizeof(*((graphInfo*)s)->labels)) + TYPST_INT_SIZE + list_size(((graphInfo*)s)->cluster_labels, ((graphInfo*)s)->cluster_labels_len, ClusterCoordinates_size, sizeof(*((graphInfo*)s)->cluster_labels)) + strlen(((graphInfo*)s)->svg) + 1;
}
int encode_graphInfo(const graphInfo *s) {
    size_t buffer_len = graphInfo_size(s);
    INIT_BUFFER_PACK(buffer_len)
    int err;
	(void)err;
    CHAR_PACK(s->error)
    INT_PACK(s->labels_len)
    for (size_t i = 0; i < s->labels_len; i++) {
        if ((err = encode_NodeCoordinates(&s->labels[i], __input_buffer + __buffer_offset, &buffer_len, &__buffer_offset))) {
            return err;
        }
    }
    INT_PACK(s->cluster_labels_len)
    for (size_t i = 0; i < s->cluster_labels_len; i++) {
        if ((err = encode_ClusterCoordinates(&s->cluster_labels[i], __input_buffer + __buffer_offset, &buffer_len, &__buffer_offset))) {
            return err;
        }
    }
    STR_PACK(s->svg)

    wasm_minimal_protocol_send_result_to_host(__input_buffer, buffer_len);
    return 0;
}
void free_overriddenLabels(overriddenLabels *s) {
    for (size_t i = 0; i < s->labels_len; i++) {
    free_OverrideLabel(&s->labels[i]);
    }
    free(s->labels);
    for (size_t i = 0; i < s->cluster_labels_len; i++) {
    if (s->cluster_labels[i]) {
        free(s->cluster_labels[i]);
    }
    }
    free(s->cluster_labels);
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
        out->labels = malloc(out->labels_len * sizeof(OverrideLabel));
        if (!out->labels){
            return 1;
        }
        for (size_t i = 0; i < out->labels_len; i++) {
    if ((err = decode_OverrideLabel(__input_buffer + __buffer_offset, buffer_len - __buffer_offset, &out->labels[i], &__buffer_offset))){return err;}
        }
    }
    NEXT_INT(out->cluster_labels_len)
    if (out->cluster_labels_len == 0) {
        out->cluster_labels = NULL;
    } else {
        out->cluster_labels = malloc(out->cluster_labels_len * sizeof(char*));
        if (!out->cluster_labels){
            return 1;
        }
        for (size_t i = 0; i < out->cluster_labels_len; i++) {
    NEXT_STR(out->cluster_labels[i])
        }
    }
    NEXT_STR(out->dot)
    FREE_BUFFER()
    return 0;
}
void free_LabelsInfos(LabelsInfos *s) {
    for (size_t i = 0; i < s->labels_len; i++) {
    free_NodeLabelInfo(&s->labels[i]);
    }
    free(s->labels);
    for (size_t i = 0; i < s->cluster_labels_len; i++) {
    free_ClusterLabelInfo(&s->cluster_labels[i]);
    }
    free(s->cluster_labels);
}
size_t LabelsInfos_size(const void *s){
	return TYPST_INT_SIZE + list_size(((LabelsInfos*)s)->labels, ((LabelsInfos*)s)->labels_len, NodeLabelInfo_size, sizeof(*((LabelsInfos*)s)->labels)) + TYPST_INT_SIZE + list_size(((LabelsInfos*)s)->cluster_labels, ((LabelsInfos*)s)->cluster_labels_len, ClusterLabelInfo_size, sizeof(*((LabelsInfos*)s)->cluster_labels));
}
int encode_LabelsInfos(const LabelsInfos *s) {
    size_t buffer_len = LabelsInfos_size(s);
    INIT_BUFFER_PACK(buffer_len)
    int err;
	(void)err;
    INT_PACK(s->labels_len)
    for (size_t i = 0; i < s->labels_len; i++) {
        if ((err = encode_NodeLabelInfo(&s->labels[i], __input_buffer + __buffer_offset, &buffer_len, &__buffer_offset))) {
            return err;
        }
    }
    INT_PACK(s->cluster_labels_len)
    for (size_t i = 0; i < s->cluster_labels_len; i++) {
        if ((err = encode_ClusterLabelInfo(&s->cluster_labels[i], __input_buffer + __buffer_offset, &buffer_len, &__buffer_offset))) {
            return err;
        }
    }

    wasm_minimal_protocol_send_result_to_host(__input_buffer, buffer_len);
    return 0;
}
void free_renderGraph(renderGraph *s) {
    if (s->dot) {
        free(s->dot);
    }
    for (size_t i = 0; i < s->labels_len; i++) {
    free_SizedNodeLabel(&s->labels[i]);
    }
    free(s->labels);
    for (size_t i = 0; i < s->cluster_labels_len; i++) {
    free_SizedClusterLabel(&s->cluster_labels[i]);
    }
    free(s->cluster_labels);
    if (s->engine) {
        free(s->engine);
    }
}
int decode_renderGraph(size_t buffer_len, renderGraph *out) {
    INIT_BUFFER_UNPACK(buffer_len)
    int err;
    (void)err;
    NEXT_FLOAT(out->font_size)
    NEXT_STR(out->dot)
    NEXT_INT(out->labels_len)
    if (out->labels_len == 0) {
        out->labels = NULL;
    } else {
        out->labels = malloc(out->labels_len * sizeof(SizedNodeLabel));
        if (!out->labels){
            return 1;
        }
        for (size_t i = 0; i < out->labels_len; i++) {
    if ((err = decode_SizedNodeLabel(__input_buffer + __buffer_offset, buffer_len - __buffer_offset, &out->labels[i], &__buffer_offset))){return err;}
        }
    }
    NEXT_INT(out->cluster_labels_len)
    if (out->cluster_labels_len == 0) {
        out->cluster_labels = NULL;
    } else {
        out->cluster_labels = malloc(out->cluster_labels_len * sizeof(SizedClusterLabel));
        if (!out->cluster_labels){
            return 1;
        }
        for (size_t i = 0; i < out->cluster_labels_len; i++) {
    if ((err = decode_SizedClusterLabel(__input_buffer + __buffer_offset, buffer_len - __buffer_offset, &out->cluster_labels[i], &__buffer_offset))){return err;}
        }
    }
    NEXT_STR(out->engine)
    FREE_BUFFER()
    return 0;
}
