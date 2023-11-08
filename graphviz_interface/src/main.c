#include "emscripten.h"

#include "protocol/protocol.h"
#include "utils/utils.h"
#include <graphviz/gvc.h>
#include <graphviz/gvplugin.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define DOUBLE_PRECISION 1000

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
    int __str_len = strlen((char *) __input_buffer + __buffer_offset);                             \
    (dst) = malloc(__str_len + 1);                                                                 \
    strcpy((dst), (char *) __input_buffer + __buffer_offset);                                      \
    __buffer_offset += __str_len + 1;                                                              \
}

#define NEXT_INT(dst)                                                                              \
    (dst) = big_endian_decode(__input_buffer + __buffer_offset, TYPST_INT_SIZE);                   \
    __buffer_offset += TYPST_INT_SIZE;

#define NEXT_DOUBLE(dst) {                                                                         \
    int __encoded_value;                                                                           \
    NEXT_INT(__encoded_value);                                                                     \
    (dst) = ((double) __encoded_value) / DOUBLE_PRECISION;                                         \
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
 * @brief Tests if the name of a node can be interpreted as math by Typst.
 *
 * Note that this function has false negatives, but should not have any false positive.
 */
bool is_name_valid_math(char *name) {
    bool was_identifier_character = false;
    for (int i = 0; name[i]; i++) {
        if (name[i] < 0) {
            // Non-ASCII characters are too complicated to handle properly.
            return false;
        }
        if ('A' <= name[i] && name[i] <= 'Z' || 'a' <= name[i] && name[i] <= 'z') {
            if (was_identifier_character) {
                return false;
            } else {
                was_identifier_character = true;
            }
        } else {
            was_identifier_character = false;
        }
    }
    return true;
}

/**
 * @brief Return a list of all labels.
 */
EMSCRIPTEN_KEEPALIVE
int get_labels(size_t dot_len) {
    // Set error report to custom function. Lets us get error messages from Graphviz.
    agseterr(AGERR);
    agseterrf(vizErrorf);

    INIT_BUFFER_UNPACK(dot_len);
    char dot[dot_len + 1];
    NEXT_SIZED_STR(dot, dot_len);
    FREE_BUFFER();

    GVC_t *gvc = gvContextPlugins(lt_preloaded_symbols, false);
    graph_t *g = agmemread(dot);

    if (!g) {
        gvFreeLayout(gvc, g);
        agclose(g);
        gvFinalize(gvc);
        gvFreeContext(gvc);
        wasm_minimal_protocol_send_result_to_host((uint8_t *)errBuff, strlen(errBuff));
        return 1;
    }

    int label_count = 0;
    int total_size = 0;
    char *labels[agnnodes(g) + agnedges(g) + agnsubg(g)];

    // Get node labels.
    for (Agnode_t *n = agfstnode(g); n; n = agnxtnode(g, n)) {
        char *label = agget(n, "label");
        if (!aghtmlstr(label)) {
            // If there is no explicitely set label for a node, so Graphviz intuitively sets its "label" attribute to "\\N".
            bool has_label = strcmp(label, "\\N") != 0;
            if (!has_label) {
                label = agnameof(n);
            }
            int label_len = strlen(label);
            labels[label_count] = malloc(label_len + 2);
            labels[label_count][0] = has_label || !is_name_valid_math(label) ? 't' : 'm';
            strcpy(labels[label_count] + 1, label);
            total_size += label_len + 2;
            label_count++;
        }
    }

    uint8_t *output = malloc(total_size);
    int offset = 0;
    for (int i = 0; i < label_count; i++) {
        int length = strlen(labels[i]);
        memcpy(output + offset, labels[i], length + 1);
        offset += length + 1;
    }
    wasm_minimal_protocol_send_result_to_host(output, total_size);

    for (int i = 0; i < label_count; i++) {
        free(labels[i]);
    }
    gvFreeLayout(gvc, g);
    agclose(g);
    gvFinalize(gvc);
    gvFreeContext(gvc);
    return 0;
}

/**
 * Creates and returns a Graphviz label that has the specified dimensions.
 */
char *create_label_for_dimension(graph_t *g, double width, double height) {
    char label[2048];
    snprintf(
        label,
        sizeof(label),
        "<table border=\"0\" fixedsize=\"true\" width=\"%lf\" height=\"%lf\"><tr><td></td></tr></table>",
        width,
        height
    );
    return agstrdup_html(g, label);
}

/**
 * Replaces non-HTML labels in a graph with invisible labels that have specific dimensions. For each
 * label that is overridden, write `true` in the corresponding cell in `is_label_overridden`. Write
  * `false` for other labels.
 */
void override_labels(graph_t *g, double *label_widths, double *label_heights, bool *is_label_overridden) {
    int label_count = 0;
    int overridden_count = 0;

    for (Agnode_t *n = agfstnode(g); n; n = agnxtnode(g, n)) {
        char *label = agget(n, "label");
        if (!aghtmlstr(label)) {
            agset(n, "label", create_label_for_dimension(g, label_widths[overridden_count], label_heights[overridden_count]));
            is_label_overridden[label_count] = true;
            overridden_count++;
        } else {
            is_label_overridden[label_count] = false;
        }
        label_count++;
    }
}

/**
 * Encode the position of considered labels in the passed buffer.
 */
void get_label_positions(graph_t *g, double margin, bool *consider, uint8_t *buffer) {
    int label_count = 0;
    int considered_count = 0;

    for (Agnode_t *n = agfstnode(g); n; n = agnxtnode(g, n)) {
        if (consider[label_count++]) {
            int x = floor((ND_coord(n).x + margin) * DOUBLE_PRECISION);
            int y = floor((ND_coord(n).y + margin) * DOUBLE_PRECISION);
            big_endian_encode(buffer + considered_count * 2 * sizeof(int), x);
            big_endian_encode(buffer + (considered_count * 2 + 1) * sizeof(int), y);
            considered_count++;
        }
    }
}

/**
 * @brief Render a graphviz graph to svg from a dot string.
 *
 * // TODO: Update that.
 * Output buffer layout is:
 * - 1 byte for result (0: OK, 1: error);
 * - 1 byte for sizeof(int);
 * - coordinates of overridden labels: for each such label,
 *   - sizeof(int) bytes for x,
 *   - sizeof(int) bytes for y;
 * - sizeof(int) bytes for SVG width;
 * - sizeof(int) bytes for SVG height;
 * - the rest is SVG data.
 * All integers are encoded in in big-endian.
 *
 * @param font_size_len the length of the integer encoding the font size
 * @param dot_len the length of the buffer containing the dot string
 * @param label_dimensions_len the length of the buffer containing the dimensions of the labels generated by Typst
 * @param engine_len the length of the buffer containing the engine string
 * @param background_len the length of the buffer containing the background color string
 * @return int 0 on success, 1 on failure
 */
EMSCRIPTEN_KEEPALIVE
int render(size_t font_size_len, size_t dot_len, size_t label_dimensions_len, size_t engine_len, size_t background_len) {
    // Set error report to custom function. Lets us get error messages from Graphviz.
    agseterr(AGERR);
    agseterrf(vizErrorf);

    INIT_BUFFER_UNPACK(font_size_len + dot_len + label_dimensions_len + engine_len + background_len);
    double font_size;
    NEXT_DOUBLE(font_size);
    char dot[dot_len + 1];
    NEXT_SIZED_STR(dot, dot_len);
    int label_count = label_dimensions_len / TYPST_INT_SIZE / 2;
    double label_widths[label_count];
    double label_heights[label_count];
    for (int i = 0; i < label_count; i++) {
        NEXT_DOUBLE(label_widths[i]);
        NEXT_DOUBLE(label_heights[i]);
    }
    char engine[engine_len + 1];
    NEXT_SIZED_STR(engine, engine_len);
    char background[background_len + 1];
    NEXT_SIZED_STR(background, background_len);
    FREE_BUFFER();

    char *render_data = NULL;

    GVC_t *gvc = gvContextPlugins(lt_preloaded_symbols, false);
    graph_t *g = agmemread(dot);

    #define FREE_EVERYTHING() {                                                                    \
        gvFreeRenderData(render_data);                                                             \
        gvFreeLayout(gvc, g);                                                                      \
        agclose(g);                                                                                \
        gvFinalize(gvc);                                                                           \
        gvFreeContext(gvc);                                                                        \
    }

    if (!g) {
        FREE_EVERYTHING();
        wasm_minimal_protocol_send_result_to_host((uint8_t *)errBuff, strlen(errBuff));
        return 0;
    }

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

    // Override labels
    bool is_label_overridden[agnnodes(g) + agnedges(g) + agnsubg(g)];
    override_labels(g, label_widths, label_heights, is_label_overridden);

    if (gvLayout(gvc, g, engine) == -1) {
        FREE_EVERYTHING();
        wasm_minimal_protocol_send_result_to_host((uint8_t *)errBuff, strlen(errBuff));
        return 0;
    }

    unsigned int svg_chunk_size;
    int result = gvRenderData(gvc, g, "svg", &render_data, &svg_chunk_size);
    if (result == -1) {
        FREE_EVERYTHING();
        char *err = "Diagraph error: failed to render graph to svg\0";
        wasm_minimal_protocol_send_result_to_host((uint8_t *)err, strlen(err));
        return 1;
    }

    uint8_t label_positions[label_count * 2 * sizeof(int)];
    get_label_positions(g, margin, is_label_overridden, label_positions);

    int svg_width = (int) floor((GD_bb(g).UR.x - GD_bb(g).LL.x + 2.0 * margin) * DOUBLE_PRECISION);
    int svg_height = (int) floor((GD_bb(g).UR.y - GD_bb(g).LL.y + 2.0 * margin) * DOUBLE_PRECISION);

    size_t output_buffer_len = 2 + sizeof(label_positions) + sizeof(svg_width) + sizeof(svg_height) + svg_chunk_size;
    uint8_t *output_buffer = malloc(output_buffer_len);
    size_t offset = 0;
    output_buffer[offset++] = 0;
    output_buffer[offset++] = sizeof(int);
    memcpy(output_buffer + offset, label_positions, sizeof(label_positions));
    offset += sizeof(label_positions);
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
