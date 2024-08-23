#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "emscripten.h"

#define PROTOCOL_FUNCTION __attribute__((import_module("typst_env"))) extern

PROTOCOL_FUNCTION void wasm_minimal_protocol_send_result_to_host(const uint8_t *ptr, size_t len);
PROTOCOL_FUNCTION void wasm_minimal_protocol_write_args_to_buffer(uint8_t *ptr);


#define TYPST_INT_SIZE 4

#define INIT_BUFFER_UNPACK(buffer_len)                                                             \
    size_t __buffer_offset = 0;                                                                    \
    uint8_t *__input_buffer = malloc((buffer_len));                                                \
    if (!__input_buffer) {                                                                         \
        return 1;                                                                                  \
    }                                                                                              \
    wasm_minimal_protocol_write_args_to_buffer(__input_buffer);

#define CHECK_BUFFER()                                                                             \
	if (__buffer_offset >= buffer_len) {                                                           \
		return 2;                                                                                  \
	}

#define NEXT_STR(dst)                                                                              \
	CHECK_BUFFER()                                                                                 \
    {                                                                                              \
		if (__input_buffer[__buffer_offset] == '\0') {                                            \
			(dst) = malloc(1);                                                                     \
			if (!(dst)) {                                                                          \
				return 1;                                                                          \
			}                                                                                      \
			(dst)[0] = '\0';                                                                      \
			__buffer_offset++;                                                                     \
		} else {                                                                                   \
			int __str_len = strlen((char *)__input_buffer + __buffer_offset);                      \
			(dst) = malloc(__str_len + 1);                                                         \
			if (!(dst)) {                                                                          \
				return 1;                                                                          \
			}                                                                                      \
			strcpy((dst), (char *)__input_buffer + __buffer_offset);                               \
			__buffer_offset += __str_len + 1;                                                      \
		}                                                                                          \
    }

#define NEXT_INT(dst)                                                                              \
	CHECK_BUFFER()                                                                                 \
    (dst) = big_endian_decode(__input_buffer + __buffer_offset, TYPST_INT_SIZE);                   \
    __buffer_offset += TYPST_INT_SIZE;

#define NEXT_CHAR(dst)                                                                             \
	CHECK_BUFFER()                                                                                 \
    (dst) = __input_buffer[__buffer_offset++];

#define NEXT_FLOAT(dst)                                                                            \
	CHECK_BUFFER()                                                                                 \
    (dst) = decode_float(__input_buffer + __buffer_offset);                                        \
	__buffer_offset += TYPST_INT_SIZE;
    
#define FREE_BUFFER()                                                                              \
    free(__input_buffer);                                                                          \
    __input_buffer = NULL;

#define INIT_BUFFER_PACK(buffer_len)                                                               \
    size_t __buffer_offset = 0;                                                                    \
    uint8_t *__input_buffer = malloc((buffer_len));                                                \
    if (!__input_buffer) {                                                                         \
        return 1;                                                                                  \
    }

#define FLOAT_PACK(fp)                                                                             \
    {                                                                                              \
		if (fp == 0.0f) {  																	       \
			big_endian_encode(0, __input_buffer + __buffer_offset, TYPST_INT_SIZE);                \
		} else {                                                                                   \
			union FloatBuffer { 																   \
				float f;   																	       \
				int i;   																	       \
			} __float_buffer;                                                                      \
			__float_buffer.f = (fp);                                                               \
			big_endian_encode(__float_buffer.i, __input_buffer + __buffer_offset, TYPST_INT_SIZE); \
		}                                                                                          \
		__buffer_offset += TYPST_INT_SIZE;                                                         \
	}

#define INT_PACK(i)                                                                                \
    big_endian_encode((i), __input_buffer + __buffer_offset, TYPST_INT_SIZE);                      \
    __buffer_offset += TYPST_INT_SIZE;

#define CHAR_PACK(c)                                                                               \
    __input_buffer[__buffer_offset++] = (c);

#define STR_PACK(s)                                                                                \
    if (s == NULL || s[0] == '\0') {                                                              \
        __input_buffer[__buffer_offset++] = '\0';                                                 \
    } else {                                                                                       \
        strcpy((char *)__input_buffer + __buffer_offset, (s));                                     \
        size_t __str_len = strlen((s));                                                            \
        __input_buffer[__buffer_offset + __str_len] = '\0';                                       \
        __buffer_offset += __str_len + 1;                                                          \
    }
typedef struct {
    char* label;
    bool content;
    bool xlabel;
} OverrideLabel;
void free_OverrideLabel(OverrideLabel *s);

typedef struct {
    char* label;
    bool math_mode;
    char* xlabel;
    bool xlabel_math_mode;
    char* headlabel;
    bool headlabel_math_mode;
    char* taillabel;
    bool taillabel_math_mode;
    int color;
    char* font_name;
    float font_size;
} EdgeLabelInfo;
void free_EdgeLabelInfo(EdgeLabelInfo *s);

typedef struct {
    char* name;
    bool native;
    char* label;
    bool math_mode;
    bool override_xlabel;
    char* xlabel;
    bool xlabel_math_mode;
    int color;
    char* font_name;
    float font_size;
    EdgeLabelInfo * edges_infos;
    size_t edges_infos_len;
} NodeLabelInfo;
void free_NodeLabelInfo(NodeLabelInfo *s);

typedef struct {
    bool native;
    char* name;
    char* label;
    bool math_mode;
    int color;
    char* font_name;
    float font_size;
} ClusterLabelInfo;
void free_ClusterLabelInfo(ClusterLabelInfo *s);

typedef struct {
    bool override;
    float width;
    float height;
    bool xoverride;
    float xwidth;
    float xheight;
    bool headoverride;
    float headwidth;
    float headheight;
    bool tailoverride;
    float tailwidth;
    float tailheight;
} SizedEdgeLabel;
void free_SizedEdgeLabel(SizedEdgeLabel *s);

typedef struct {
    bool override;
    bool xoverride;
    float width;
    float height;
    float xwidth;
    float xheight;
    SizedEdgeLabel * edges_size;
    size_t edges_size_len;
} SizedNodeLabel;
void free_SizedNodeLabel(SizedNodeLabel *s);

typedef struct {
    float width;
    float height;
} SizedClusterLabel;
void free_SizedClusterLabel(SizedClusterLabel *s);

typedef struct {
    float x;
    float y;
    float xx;
    float xy;
    float headx;
    float heady;
    float tailx;
    float taily;
} EdgeCoordinates;
void free_EdgeCoordinates(EdgeCoordinates *s);

typedef struct {
    float x;
    float y;
    float xx;
    float xy;
    EdgeCoordinates * edges;
    size_t edges_len;
} NodeCoordinates;
void free_NodeCoordinates(NodeCoordinates *s);

typedef struct {
    float x;
    float y;
} ClusterCoordinates;
void free_ClusterCoordinates(ClusterCoordinates *s);

typedef struct {
    bool error;
    NodeCoordinates * labels;
    size_t labels_len;
    ClusterCoordinates * cluster_labels;
    size_t cluster_labels_len;
    char* svg;
} graphInfo;
void free_graphInfo(graphInfo *s);
int encode_graphInfo(const graphInfo *s);

typedef struct {
    OverrideLabel * labels;
    size_t labels_len;
    char* * cluster_labels;
    size_t cluster_labels_len;
    char* dot;
} overriddenLabels;
void free_overriddenLabels(overriddenLabels *s);
int decode_overriddenLabels(size_t buffer_len, overriddenLabels *out);

typedef struct {
    NodeLabelInfo * labels;
    size_t labels_len;
    ClusterLabelInfo * cluster_labels;
    size_t cluster_labels_len;
} LabelsInfos;
void free_LabelsInfos(LabelsInfos *s);
int encode_LabelsInfos(const LabelsInfos *s);

typedef struct {
    float font_size;
    char* dot;
    SizedNodeLabel * labels;
    size_t labels_len;
    SizedClusterLabel * cluster_labels;
    size_t cluster_labels_len;
    char* engine;
} renderGraph;
void free_renderGraph(renderGraph *s);
int decode_renderGraph(size_t buffer_len, renderGraph *out);

#endif
