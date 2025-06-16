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
    if (!elem || !((char *)elem)[0]) {
        return 1;
    }
    return strlen((char *)elem) + 1;
}
size_t string_list_size(char **list, size_t size) {
	size_t result = 0;
	for (size_t i = 0; i < size; i++) {
		result += string_size(list[i]);
	}
	return result;
}

void free_EdgeLabelInfo(EdgeLabelInfo *s) {
    if (s->to) {
        free(s->to);
    }
    if (s->label) {
        free(s->label);
    }
    if (s->xlabel) {
        free(s->xlabel);
    }
    if (s->headlabel) {
        free(s->headlabel);
    }
    if (s->taillabel) {
        free(s->taillabel);
    }
    if (s->font_name) {
        free(s->font_name);
    }
}
size_t EdgeLabelInfo_size(const void *s){
	return string_size(((EdgeLabelInfo*)s)->to) + TYPST_INT_SIZE + string_size(((EdgeLabelInfo*)s)->label) + 1 + 1 + string_size(((EdgeLabelInfo*)s)->xlabel) + 1 + 1 + string_size(((EdgeLabelInfo*)s)->headlabel) + 1 + 1 + string_size(((EdgeLabelInfo*)s)->taillabel) + 1 + 1 + TYPST_INT_SIZE + string_size(((EdgeLabelInfo*)s)->font_name) + TYPST_INT_SIZE;
}
int encode_EdgeLabelInfo(const EdgeLabelInfo *s, uint8_t *__input_buffer, size_t *buffer_len, size_t *buffer_offset) {
    size_t __buffer_offset = 0;    size_t s_size = EdgeLabelInfo_size(s);
    if (s_size > *buffer_len) {
        return 2;
    }
    int err;
	(void)err;
    STR_PACK(s->to)
    INT_PACK(s->index)
    STR_PACK(s->label)
    CHAR_PACK(s->label_math_mode)
    CHAR_PACK(s->label_html_mode)
    STR_PACK(s->xlabel)
    CHAR_PACK(s->xlabel_math_mode)
    CHAR_PACK(s->xlabel_html_mode)
    STR_PACK(s->headlabel)
    CHAR_PACK(s->headlabel_math_mode)
    CHAR_PACK(s->headlabel_html_mode)
    STR_PACK(s->taillabel)
    CHAR_PACK(s->taillabel_math_mode)
    CHAR_PACK(s->taillabel_html_mode)
    INT_PACK(s->color)
    STR_PACK(s->font_name)
    FLOAT_PACK(s->font_size)

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
    for (size_t i = 0; i < s->edges_infos_len; i++) {
    free_EdgeLabelInfo(&s->edges_infos[i]);
    }
    free(s->edges_infos);
}
size_t NodeLabelInfo_size(const void *s){
	return string_size(((NodeLabelInfo*)s)->name) + string_size(((NodeLabelInfo*)s)->label) + 1 + 1 + string_size(((NodeLabelInfo*)s)->xlabel) + 1 + 1 + TYPST_INT_SIZE + string_size(((NodeLabelInfo*)s)->font_name) + TYPST_INT_SIZE + TYPST_INT_SIZE + list_size(((NodeLabelInfo*)s)->edges_infos, ((NodeLabelInfo*)s)->edges_infos_len, EdgeLabelInfo_size, sizeof(*((NodeLabelInfo*)s)->edges_infos));
}
int encode_NodeLabelInfo(const NodeLabelInfo *s, uint8_t *__input_buffer, size_t *buffer_len, size_t *buffer_offset) {
    size_t __buffer_offset = 0;    size_t s_size = NodeLabelInfo_size(s);
    if (s_size > *buffer_len) {
        return 2;
    }
    int err;
	(void)err;
    STR_PACK(s->name)
    STR_PACK(s->label)
    CHAR_PACK(s->math_mode)
    CHAR_PACK(s->html_mode)
    STR_PACK(s->xlabel)
    CHAR_PACK(s->xlabel_math_mode)
    CHAR_PACK(s->xlabel_html_mode)
    INT_PACK(s->color)
    STR_PACK(s->font_name)
    FLOAT_PACK(s->font_size)
    INT_PACK(s->edges_infos_len)
    for (size_t i = 0; i < s->edges_infos_len; i++) {
        if ((err = encode_EdgeLabelInfo(&s->edges_infos[i], __input_buffer + __buffer_offset, buffer_len, &__buffer_offset))) {
            return err;
        }
    }

    *buffer_offset += __buffer_offset;
    return 0;
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
	return string_size(((ClusterLabelInfo*)s)->name) + string_size(((ClusterLabelInfo*)s)->label) + 1 + 1 + TYPST_INT_SIZE + string_size(((ClusterLabelInfo*)s)->font_name) + TYPST_INT_SIZE;
}
int encode_ClusterLabelInfo(const ClusterLabelInfo *s, uint8_t *__input_buffer, size_t *buffer_len, size_t *buffer_offset) {
    size_t __buffer_offset = 0;    size_t s_size = ClusterLabelInfo_size(s);
    if (s_size > *buffer_len) {
        return 2;
    }
    int err;
	(void)err;
    STR_PACK(s->name)
    STR_PACK(s->label)
    CHAR_PACK(s->math_mode)
    CHAR_PACK(s->html_mode)
    INT_PACK(s->color)
    STR_PACK(s->font_name)
    FLOAT_PACK(s->font_size)

    *buffer_offset += __buffer_offset;
    return 0;
}
void free_SizedEdgeLabel(SizedEdgeLabel *s) {
}
int decode_SizedEdgeLabel(uint8_t *__input_buffer, size_t buffer_len, SizedEdgeLabel *out, size_t *buffer_offset) {
    size_t __buffer_offset = 0;
    int err;
    (void)err;
    NEXT_CHAR(out->overwrite)
    NEXT_FLOAT(out->width)
    NEXT_FLOAT(out->height)
    NEXT_CHAR(out->xoverwrite)
    NEXT_FLOAT(out->xwidth)
    NEXT_FLOAT(out->xheight)
    NEXT_CHAR(out->headoverwrite)
    NEXT_FLOAT(out->headwidth)
    NEXT_FLOAT(out->headheight)
    NEXT_CHAR(out->tailoverwrite)
    NEXT_FLOAT(out->tailwidth)
    NEXT_FLOAT(out->tailheight)
    *buffer_offset += __buffer_offset;
    return 0;
}
void free_SizedNodeLabel(SizedNodeLabel *s) {
    for (size_t i = 0; i < s->edges_size_len; i++) {
    free_SizedEdgeLabel(&s->edges_size[i]);
    }
    free(s->edges_size);
}
int decode_SizedNodeLabel(uint8_t *__input_buffer, size_t buffer_len, SizedNodeLabel *out, size_t *buffer_offset) {
    size_t __buffer_offset = 0;
    int err;
    (void)err;
    NEXT_CHAR(out->overwrite)
    NEXT_CHAR(out->xoverwrite)
    NEXT_FLOAT(out->width)
    NEXT_FLOAT(out->height)
    NEXT_FLOAT(out->xwidth)
    NEXT_FLOAT(out->xheight)
    NEXT_INT(out->edges_size_len)
    if (out->edges_size_len == 0) {
        out->edges_size = NULL;
    } else {
        out->edges_size = malloc(out->edges_size_len * sizeof(SizedEdgeLabel));
        if (!out->edges_size){
            return 1;
        }
        for (size_t i = 0; i < out->edges_size_len; i++) {
    if ((err = decode_SizedEdgeLabel(__input_buffer + __buffer_offset, buffer_len - __buffer_offset, &out->edges_size[i], &__buffer_offset))){return err;}
        }
    }
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
void free_EdgeCoordinates(EdgeCoordinates *s) {
}
size_t EdgeCoordinates_size(const void *s){
	return TYPST_INT_SIZE + TYPST_INT_SIZE + TYPST_INT_SIZE + TYPST_INT_SIZE + TYPST_INT_SIZE + TYPST_INT_SIZE + TYPST_INT_SIZE + TYPST_INT_SIZE;
}
int encode_EdgeCoordinates(const EdgeCoordinates *s, uint8_t *__input_buffer, size_t *buffer_len, size_t *buffer_offset) {
    size_t __buffer_offset = 0;    size_t s_size = EdgeCoordinates_size(s);
    if (s_size > *buffer_len) {
        return 2;
    }
    int err;
	(void)err;
    FLOAT_PACK(s->x)
    FLOAT_PACK(s->y)
    FLOAT_PACK(s->xx)
    FLOAT_PACK(s->xy)
    FLOAT_PACK(s->headx)
    FLOAT_PACK(s->heady)
    FLOAT_PACK(s->tailx)
    FLOAT_PACK(s->taily)

    *buffer_offset += __buffer_offset;
    return 0;
}
void free_NodeCoordinates(NodeCoordinates *s) {
    for (size_t i = 0; i < s->edges_len; i++) {
    free_EdgeCoordinates(&s->edges[i]);
    }
    free(s->edges);
}
size_t NodeCoordinates_size(const void *s){
	return TYPST_INT_SIZE + TYPST_INT_SIZE + TYPST_INT_SIZE + TYPST_INT_SIZE + TYPST_INT_SIZE + list_size(((NodeCoordinates*)s)->edges, ((NodeCoordinates*)s)->edges_len, EdgeCoordinates_size, sizeof(*((NodeCoordinates*)s)->edges));
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
    INT_PACK(s->edges_len)
    for (size_t i = 0; i < s->edges_len; i++) {
        if ((err = encode_EdgeCoordinates(&s->edges[i], __input_buffer + __buffer_offset, buffer_len, &__buffer_offset))) {
            return err;
        }
    }

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
void free_GetGraphInfo(GetGraphInfo *s) {
    if (s->dot) {
        free(s->dot);
    }
}
int decode_GetGraphInfo(size_t buffer_len, GetGraphInfo *out) {
    INIT_BUFFER_UNPACK(buffer_len)
    int err;
    (void)err;
    NEXT_STR(out->dot)
    FREE_BUFFER()
    return 0;
}
void free_Engines(Engines *s) {
    for (size_t i = 0; i < s->engines_len; i++) {
    if (s->engines[i]) {
        free(s->engines[i]);
    }
    }
    free(s->engines);
}
size_t Engines_size(const void *s){
	return TYPST_INT_SIZE + string_list_size(((Engines*)s)->engines, ((Engines*)s)->engines_len);
}
int encode_Engines(const Engines *s) {
    size_t buffer_len = Engines_size(s);
    INIT_BUFFER_PACK(buffer_len)
    int err;
	(void)err;
    INT_PACK(s->engines_len)
    for (size_t i = 0; i < s->engines_len; i++) {
    STR_PACK(s->engines[i])
    }

    wasm_minimal_protocol_send_result_to_host(__input_buffer, buffer_len);
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
	return 1 + 1 + TYPST_INT_SIZE + list_size(((graphInfo*)s)->labels, ((graphInfo*)s)->labels_len, NodeCoordinates_size, sizeof(*((graphInfo*)s)->labels)) + TYPST_INT_SIZE + list_size(((graphInfo*)s)->cluster_labels, ((graphInfo*)s)->cluster_labels_len, ClusterCoordinates_size, sizeof(*((graphInfo*)s)->cluster_labels)) + string_size(((graphInfo*)s)->svg);
}
int encode_graphInfo(const graphInfo *s) {
    size_t buffer_len = graphInfo_size(s);
    INIT_BUFFER_PACK(buffer_len)
    int err;
	(void)err;
    CHAR_PACK(s->error)
    CHAR_PACK(s->landscape)
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
void free_GraphInfo(GraphInfo *s) {
    for (size_t i = 0; i < s->labels_len; i++) {
    free_NodeLabelInfo(&s->labels[i]);
    }
    free(s->labels);
    for (size_t i = 0; i < s->cluster_labels_len; i++) {
    free_ClusterLabelInfo(&s->cluster_labels[i]);
    }
    free(s->cluster_labels);
}
size_t GraphInfo_size(const void *s){
	return TYPST_INT_SIZE + list_size(((GraphInfo*)s)->labels, ((GraphInfo*)s)->labels_len, NodeLabelInfo_size, sizeof(*((GraphInfo*)s)->labels)) + TYPST_INT_SIZE + list_size(((GraphInfo*)s)->cluster_labels, ((GraphInfo*)s)->cluster_labels_len, ClusterLabelInfo_size, sizeof(*((GraphInfo*)s)->cluster_labels));
}
int encode_GraphInfo(const GraphInfo *s) {
    size_t buffer_len = GraphInfo_size(s);
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
