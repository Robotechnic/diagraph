#include "emscripten.h"

#include "protocol/protocol.h"
#include "utils/utils.h"
#include <graphviz/gvc.h>
#include <graphviz/gvplugin.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define INIT_BUFFER_UNPACK(buffer_len)                                                             \
    size_t __buffer_offset = 0;                                                                    \
    uint8_t *__input_buffer = malloc((buffer_len));                                                  \
    if (!__input_buffer) {                                                                         \
        return 1;                                                                                  \
    }                                                                                              \
    wasm_minimal_protocol_write_args_to_buffer(__input_buffer);

#define NEXT_CHAR(dst, len)                                                                       \
    (dst) = malloc((len) + 1);                                                                        \
    memcpy((dst), __input_buffer + __buffer_offset, (len));                                           \
    (dst)[(len)] = '\0';                                                                              \
    __buffer_offset += (len);

#define NEXT_STR(dst) {                                                                           \
        int len = strlen(__input_buffer + __buffer_offset);                                        \
        (dst) = malloc(len + 1);                                                                    \
        strcpy((dst), __input_buffer + __buffer_offset);                                            \
        __buffer_offset += len + 1;                                                                \
    }

#define NEXT_INT(dst, int_size)                                                                   \
    (dst) = big_endian_decode(__input_buffer + __buffer_offset, (int_size));                          \
    __buffer_offset += (int_size);

#define FREE_BUFFER()                                                                              \
    free(__input_buffer);                                                                          \
    __input_buffer = NULL;

char errBuff[1024];
int vizErrorf(char *str) {
    char *intro = "\1Diagraph error: ";
    int intro_len = strlen(intro);
    strncpy(errBuff + intro_len, str, sizeof(errBuff) - intro_len);
    memcpy(errBuff, intro, intro_len);
    return 0;
}

/**
 * @brief Render a graphviz graph to svg from a dot string
 *
 * Output buffer layout is:
 * - 1 byte for result (0: OK, 1: error);
 * - 1 byte for sizeof(int);
 * - coordinates of nodes with overridden labels: for each such node
 *   - sizeof(int) bytes for x,
 *   - sizeof(int) bytes for y;
 * - sizeof(int) bytes for SVG width;
 * - sizeof(int) bytes for SVG height;
 * - the rest is SVG data.
 * All integers are encoded in in big-endian.
 *
 * @param font_size_len the length of the integer encoding the font size
 * @param dot_len the length of the buffer containing the dot string
 * @param overridden_labels_len the length of the buffer containing the identifiers of the nodes whose labels are overridden
 * @param engine_len the length of the buffer containing the engine string
 * @param background_len the length of the buffer containing the background color string
 * @return int 0 on success, 1 on failure
 */
EMSCRIPTEN_KEEPALIVE
int render(size_t font_size_len, size_t dot_len, size_t overridden_labels_len, size_t engine_len, size_t background_len) {
    // set error report to custom function
    // it allow us to get the error message from graphviz
    agseterr(AGERR);
    agseterrf(vizErrorf);

    INIT_BUFFER_UNPACK(font_size_len + dot_len + overridden_labels_len + engine_len + background_len);
    int font_size_100;
    NEXT_INT(font_size_100, font_size_len);
    char *dot;
    NEXT_CHAR(dot, dot_len);
    int overridden_label_count;
    NEXT_INT(overridden_label_count, 4);
    char *overridden_labels[overridden_label_count];
    int label_widths[overridden_label_count];
    int label_heights[overridden_label_count];
    for (int i = 0; i < overridden_label_count; i++) {
        NEXT_STR(overridden_labels[i]);
        NEXT_INT(label_widths[i], 4);
        NEXT_INT(label_heights[i], 4);
    }
    char *engine;
    NEXT_CHAR(engine, engine_len);
    char *background;
    NEXT_CHAR(background, background_len);
    FREE_BUFFER();

    GVC_t *gvc = gvContextPlugins(lt_preloaded_symbols, false);
    graph_t *g = agmemread(dot);

    if (strlen(background) > 0) {
        agattr(g, AGRAPH, "bgcolor", background);
    }

    agattr(g, AGRAPH, "center", "true");
    agattr(g, AGRAPH, "margin", "0");

    {
        double font_size = ((double) font_size_100) / 100.0;
        char font_size_string[128];
        snprintf(font_size_string, 128, "%fpt", font_size);
        // TODO: Apparently fontsize can also be set on clusters (see: https://graphviz.org/docs/attrs/fontsize/).
        agattr(g, AGRAPH, "fontsize", font_size_string);
        agattr(g, AGNODE, "fontsize", font_size_string);
        agattr(g, AGEDGE, "fontsize", font_size_string);
    }

    free(background);
    free(dot);

    if (!g) {
        for (int i = 0; i < overridden_label_count; i++) {
            free(overridden_labels[i]);
        }
        free(engine);
        wasm_minimal_protocol_send_result_to_host((uint8_t *)errBuff, strlen(errBuff));
        return 0;
    }

    // Remove labels for nodes whose label is overridden
    for (int i = 0; i < overridden_label_count; i++) {
        // TODO: Optimize using `agattr` (see page 8 of https://graphviz.org/pdf/cgraph.pdf).
        Agnode_t *n = agnode(g, overridden_labels[i], FALSE);
        if (n != NULL) {
            char label[2048];
            snprintf(label, 2048, "<table border=\"0\" fixedsize=\"true\" width=\"%d\" height=\"%d\"><tr><td></td></tr></table>", label_widths[i], label_heights[i]);
            agset(n, "label", agstrdup_html(g, label));
        }
    }

    if (gvLayout(gvc, g, engine) == -1) {
        for (int i = 0; i < overridden_label_count; i++) {
            free(overridden_labels[i]);
        }
        free(engine);
        wasm_minimal_protocol_send_result_to_host((uint8_t *)errBuff, strlen(errBuff));
        return 0;
    }

    char *data = NULL;
    unsigned int svg_chunk_size;

    int result = gvRenderData(gvc, g, "svg", &data, &svg_chunk_size);
    free(engine);
    if (result == -1) {
        for (int i = 0; i < overridden_label_count; i++) {
            free(overridden_labels[i]);
        }
        gvFreeRenderData(data);
        char *err = "error: failed to render graph to svg\0";
        wasm_minimal_protocol_send_result_to_host((uint8_t *)err, strlen(err));
        return 1;
    }

    size_t position_chunk_size = overridden_label_count * 2 * sizeof(int);
    int *node_positions = malloc(position_chunk_size);
    for (int i = 0; i < overridden_label_count; i++) {
        Agnode_t *n = agnode(g, overridden_labels[i], FALSE);
        if (n != NULL) {
            int label_x = floor(ND_coord(n).x + ND_width(n) / 2);
            int label_y = floor(ND_coord(n).y + ND_height(n) / 2);
            big_endian_encode(node_positions + i * 2, label_x);
            big_endian_encode(node_positions + (i * 2 + 1), label_y);
        } else {
            free(node_positions);
            for (int i = 0; i < overridden_label_count; i++) {
                free(overridden_labels[i]);
            }
            char *err = "Unable to get all node positions.";
            wasm_minimal_protocol_send_result_to_host((uint8_t *)err, strlen(err));
            return 1;
        }
    }
    for (int i = 0; i < overridden_label_count; i++) {
        free(overridden_labels[i]);
    }

    int width = (int)floor(GD_bb(g).UR.x - GD_bb(g).LL.x) + 8;
    int height = (int)floor(GD_bb(g).UR.y - GD_bb(g).LL.y) + 8;

    size_t output_buffer_len = 2 + position_chunk_size + sizeof(width) + sizeof(height) + svg_chunk_size;
    uint8_t *output_buffer = malloc(output_buffer_len);
    size_t offset = 0;
    output_buffer[offset++] = 0;
    output_buffer[offset++] = sizeof(width);
    // TODO: We could write the positions in the right buffer to start with.
    memcpy(output_buffer + offset, node_positions, position_chunk_size);
    free(node_positions);
    offset += position_chunk_size;
    big_endian_encode(output_buffer + offset, width);
    offset += sizeof(width);
    big_endian_encode(output_buffer + offset, height);
    offset += sizeof(height);
    memcpy(output_buffer + offset, data, svg_chunk_size);

    wasm_minimal_protocol_send_result_to_host(output_buffer, output_buffer_len);
    gvFreeRenderData(data);
    gvFreeLayout(gvc, g);
    agclose(g);
    gvFinalize(gvc);
    gvFreeContext(gvc);
    return 0;
}

EMSCRIPTEN_KEEPALIVE
int get_version() {
    wasm_minimal_protocol_send_result_to_host((uint8_t *)PACKAGE_VERSION, strlen(PACKAGE_VERSION));
    return 0;
}