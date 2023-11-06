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
    uint8_t *__input_buffer = malloc((buffer_len));                                                \
    if (!__input_buffer) {                                                                         \
        return 1;                                                                                  \
    }                                                                                              \
    wasm_minimal_protocol_write_args_to_buffer(__input_buffer);

#define NEXT_SIZED_STR(dst, len)                                                                   \
    memcpy((dst), __input_buffer + __buffer_offset, (len));                                        \
    (dst)[(len)] = '\0';                                                                           \
    __buffer_offset += (len);

#define NEXT_STR(dst) {                                                                            \
    int __str_len = strlen(__input_buffer + __buffer_offset);                                      \
    (dst) = malloc(__str_len + 1);                                                                 \
    strcpy((dst), __input_buffer + __buffer_offset);                                               \
    __buffer_offset += __str_len + 1;                                                              \
}

#define NEXT_INT(dst, int_size)                                                                    \
    (dst) = big_endian_decode(__input_buffer + __buffer_offset, (int_size));                       \
    __buffer_offset += (int_size);

#define NEXT_DOUBLE(dst, int_size, precision) {                                                    \
    int __encoded_value = big_endian_decode(__input_buffer + __buffer_offset, (int_size));         \
    (dst) = ((double) __encoded_value) / precision;                                                \
    __buffer_offset += (int_size);                                                                 \
}

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
 * @brief Render a graphviz graph to svg from a dot string.
 *
 * Output buffer layout is:
 * - 1 byte for result (0: OK, 1: error);
 * - 1 byte for sizeof(int);
 * - coordinates of nodes with overridden labels: for each such node,
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
    // Set error report to custom function. Lets us get error messages from Graphviz.
    agseterr(AGERR);
    agseterrf(vizErrorf);

    INIT_BUFFER_UNPACK(font_size_len + dot_len + overridden_labels_len + engine_len + background_len);
    double font_size;
    NEXT_DOUBLE(font_size, font_size_len, 100.0);
    char dot[dot_len + 1];
    NEXT_SIZED_STR(dot, dot_len);
    int overridden_label_count;
    NEXT_INT(overridden_label_count, 4);
    char *overridden_labels[overridden_label_count];
    double label_widths[overridden_label_count];
    double label_heights[overridden_label_count];
    for (int i = 0; i < overridden_label_count; i++) {
        NEXT_STR(overridden_labels[i]);
        NEXT_DOUBLE(label_widths[i], 4, 100.0);
        NEXT_DOUBLE(label_heights[i], 4, 100.0);
    }
    char engine[engine_len + 1];
    NEXT_SIZED_STR(engine, engine_len);
    char background[background_len + 1];
    NEXT_SIZED_STR(background, background_len);
    FREE_BUFFER();

    char *render_data = NULL;

    #define FREE_EVERYTHING() {                                                                    \
        for (int i = 0; i < overridden_label_count; i++) {                                         \
            free(overridden_labels[i]);                                                            \
        }                                                                                          \
        gvFreeRenderData(render_data);                                                             \
        gvFreeLayout(gvc, g);                                                                      \
        agclose(g);                                                                                \
        gvFinalize(gvc);                                                                           \
        gvFreeContext(gvc);                                                                        \
    }

    GVC_t *gvc = gvContextPlugins(lt_preloaded_symbols, false);
    graph_t *g = agmemread(dot);

    if (strlen(background) > 0) {
        agattr(g, AGRAPH, "bgcolor", background);
    }

    agattr(g, AGRAPH, "center", "true");
    agattr(g, AGRAPH, "margin", "0");

    // This is the default margin around the graph (even though we just set it to 0).
    double margin = 4.0;

    {
        char font_size_string[128];
        snprintf(font_size_string, 128, "%fpt", font_size);
        agattr(g, AGRAPH, "fontsize", font_size_string);
        agattr(g, AGNODE, "fontsize", font_size_string);
        agattr(g, AGEDGE, "fontsize", font_size_string);
    }

    if (!g) {
        for (int i = 0; i < overridden_label_count; i++) {
            free(overridden_labels[i]);
        }
        wasm_minimal_protocol_send_result_to_host((uint8_t *)errBuff, strlen(errBuff));
        return 0;
    }

    // Remove labels for nodes whose label is overridden
    for (int i = 0; i < overridden_label_count; i++) {
        // TODO: Optimize using `agattr` (see page 8 of https://graphviz.org/pdf/cgraph.pdf).
        Agnode_t *n = agnode(g, overridden_labels[i], FALSE);
        if (n != NULL) {
            char label[2048];
            snprintf(label, 2048, "<table border=\"0\" fixedsize=\"true\" width=\"%lf\" height=\"%lf\"><tr><td></td></tr></table>", label_widths[i], label_heights[i]);
            agset(n, "label", agstrdup_html(g, label));
        }
    }

    if (gvLayout(gvc, g, engine) == -1) {
        FREE_EVERYTHING();
        wasm_minimal_protocol_send_result_to_host((uint8_t *)errBuff, strlen(errBuff));
        return 0;
    }

    unsigned int svg_chunk_size;
    int result = gvRenderData(gvc, g, "svg", &render_data, &svg_chunk_size);
    free(engine);
    if (result == -1) {
        FREE_EVERYTHING();
        char *err = "Diagraph error: failed to render graph to svg\0";
        wasm_minimal_protocol_send_result_to_host((uint8_t *)err, strlen(err));
        return 1;
    }

    int node_positions[overridden_label_count * 2];
    for (int i = 0; i < overridden_label_count; i++) {
        Agnode_t *n = agnode(g, overridden_labels[i], FALSE);
        if (n != NULL) {
            int label_x = floor(ND_coord(n).x + margin);
            int label_y = floor(ND_coord(n).y + margin);
            big_endian_encode(node_positions + i * 2, label_x);
            big_endian_encode(node_positions + (i * 2 + 1), label_y);
        } else {
            FREE_EVERYTHING();
            errBuff[0] = 1;
            char message[512];
            snprintf(message, sizeof(message), "Unable to override node label: node %s does not exist", overridden_labels[i]);
            strcpy(errBuff + 1, message);
            char err[256] = "Unable to get all node positions.";
            wasm_minimal_protocol_send_result_to_host((uint8_t *)errBuff, strlen(errBuff));
            return 0;
        }
    }

    int svg_width = (int) floor(GD_bb(g).UR.x - GD_bb(g).LL.x + 2.0 * margin);
    int svg_height = (int) floor(GD_bb(g).UR.y - GD_bb(g).LL.y + 2.0 * margin);

    size_t output_buffer_len = 2 + sizeof(node_positions) + sizeof(svg_width) + sizeof(svg_height) + svg_chunk_size;
    uint8_t *output_buffer = malloc(output_buffer_len);
    size_t offset = 0;
    output_buffer[offset++] = 0;
    output_buffer[offset++] = sizeof(svg_width);
    // TODO: We could write the positions in the right buffer to start with.
    memcpy(output_buffer + offset, node_positions, sizeof(node_positions));
    offset += sizeof(node_positions);
    big_endian_encode(output_buffer + offset, svg_width);
    offset += sizeof(svg_width);
    big_endian_encode(output_buffer + offset, svg_height);
    offset += sizeof(svg_height);
    memcpy(output_buffer + offset, render_data, svg_chunk_size);

    wasm_minimal_protocol_send_result_to_host(output_buffer, output_buffer_len);
    FREE_EVERYTHING();
    return 0;
}

EMSCRIPTEN_KEEPALIVE
int get_version() {
    wasm_minimal_protocol_send_result_to_host((uint8_t *)PACKAGE_VERSION, strlen(PACKAGE_VERSION));
    return 0;
}
