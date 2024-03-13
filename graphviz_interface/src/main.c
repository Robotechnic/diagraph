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

char errBuff[1024];
int vizErrorf(char *str) {
    char *intro = "\1Diagraph error: ";
    int intro_len = strlen(intro);
    strncpy(errBuff + intro_len, str, sizeof(errBuff) - intro_len);
    memcpy(errBuff, intro, intro_len);
    return 0;
}

/**
 * Returns the total number of labels.
 */
int get_total_label_count(graph_t *g) {
    return agnnodes(g);
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
        if (was_identifier_character &&
            ('A' <= name[i] && name[i] <= 'Z' || 'a' <= name[i] && name[i] <= 'z' ||
             '0' <= name[i] && name[i] <= '9')) {
            return false;
        } else if ('A' <= name[i] && name[i] <= 'Z' || 'a' <= name[i] && name[i] <= 'z') {
            was_identifier_character = true;
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
int get_labels(size_t manual_label_count_len, size_t manual_label_names_len, size_t dot_len) {
    // Set error report to custom function. Lets us get error messages from Graphviz.
    agseterr(AGERR);
    agseterrf(vizErrorf);

    INIT_BUFFER_UNPACK(manual_label_count_len + manual_label_names_len + dot_len);
    int manual_label_count;
    NEXT_INT(manual_label_count);
    char *manual_label_names[manual_label_count];
    for (int i = 0; i < manual_label_count; i++) {
        NEXT_STR(manual_label_names[i]);
    }
    char dot[dot_len + 1];
    NEXT_SIZED_STR(dot, dot_len);
    FREE_BUFFER();

    GVC_t *gvc = gvContextPlugins(lt_preloaded_symbols, false);
    graph_t *g = agmemread(dot);

    if (!g) {
        for (int i = 0; i < manual_label_count; i++) {
            free(manual_label_names[i]);
        }
        gvFinalize(gvc);
        gvFreeContext(gvc);
        wasm_minimal_protocol_send_result_to_host((uint8_t *)errBuff, strlen(errBuff));
        return 1;
    }

    int label_index = 0;
    int total_size = 0;
    char *labels[get_total_label_count(g)];

    // Get node labels.
    for (Agnode_t *n = agfstnode(g); n; n = agnxtnode(g, n)) {
        char *name = agnameof(n);
        bool is_manually_overridden = false;
        for (int i = 0; i < manual_label_count; i++) {
            if (strcmp(manual_label_names[i], name) == 0) {
                is_manually_overridden = true;
            }
        }
        if (is_manually_overridden) {
            continue;
        }
        char *label = agget(n, "label");
        if (!aghtmlstr(label)) {
            // If there is no explicitely set label for a node, so Graphviz intuitively sets its
            // "label" attribute to "\\N".
            bool has_label = strcmp(label, "\\N") != 0;
            if (!has_label) {
                label = agnameof(n);
            }
            int label_len = strlen(label);
            labels[label_index] = malloc(label_len + 2);
            labels[label_index][0] = has_label || !is_name_valid_math(label) ? 't' : 'm';
            strcpy(labels[label_index] + 1, label);
            total_size += label_len + 2;
            label_index++;
        }
    }

    int label_count = label_index;

    uint8_t *output = malloc(total_size);
    int offset = 0;
    for (int i = 0; i < label_count; i++) {
        int length = strlen(labels[i]);
        memcpy(output + offset, labels[i], length + 1);
        offset += length + 1;
    }
    wasm_minimal_protocol_send_result_to_host(output, total_size);

    for (int i = 0; i < manual_label_count; i++) {
        free(manual_label_names[i]);
    }
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
    snprintf(label, sizeof(label),
             "<table border=\"0\" fixedsize=\"true\" width=\"%lf\" "
             "height=\"%lf\"><tr><td></td></tr></table>",
             width, height);
    return agstrdup_html(g, label);
}

/**
 * Replaces labels for nodes whose name corresponds to a label name with invisible labels that have
 * specific dimensions. For each node whose label is manually overridden, writes the index of the
 * new label in the corresponding cell in `manual_override_indices`. Writes `-1` in
 * `manual_override_indices` for other nodes.
 */
void manually_override_labels(graph_t *g, double manual_label_count, const double *label_widths,
                              const double *label_heights, const char **label_names,
                              int *manual_override_indices) {
    int label_index = 0;

    for (Agnode_t *n = agfstnode(g); n; n = agnxtnode(g, n)) {
        manual_override_indices[label_index] = -1;
        for (int i = 0; i < manual_label_count; i++) {
            if (strcmp(label_names[i], agnameof(n)) == 0) {
                agset(n, "label", create_label_for_dimension(g, label_widths[i], label_heights[i]));
                manual_override_indices[label_index] = i;
                break;
            }
        }
        label_index++;
    }
}

/**
 * Replaces non-HTML and non-already overridden labels in a graph with invisible labels that have
 * specific dimensions. For each label that is overridden, writes `true` in the corresponding cell
 * in `is_label_overridden`. Writes `false` for other labels.
 */
void override_native_labels(graph_t *g, const double *label_widths, const double *label_heights,
                            bool *is_label_overridden, const int *manual_override_indices) {
    int label_index = 0;
    int native_override_chain_index = 0;

    for (Agnode_t *n = agfstnode(g); n; n = agnxtnode(g, n)) {
        is_label_overridden[label_index] = false;
        if (manual_override_indices[label_index] < 0) {
            if (!aghtmlstr(agget(n, "label"))) {
                agset(n, "label",
                      create_label_for_dimension(g, label_widths[native_override_chain_index],
                                                 label_heights[native_override_chain_index]));
                is_label_overridden[label_index] = true;
                native_override_chain_index++;
            }
        }
        label_index++;
    }
}

/**
 * Encodes the position of considered labels in the output buffer.
 */
void get_label_positions(graph_t *g, double pad, const bool *consider, uint8_t *output) {
    int label_index = 0;
    int considered_count = 0;

    for (Agnode_t *n = agfstnode(g); n; n = agnxtnode(g, n)) {
        if (consider[label_index]) {
            int x = floor((ND_coord(n).x + pad) * DOUBLE_PRECISION);
            int y = floor((ND_coord(n).y + pad) * DOUBLE_PRECISION);
            big_endian_encode(output + considered_count * 2 * sizeof(int), x);
            big_endian_encode(output + (considered_count * 2 + 1) * sizeof(int), y);
            considered_count++;
        }
        label_index++;
    }
}

#define FREE_EVERYTHING()                                                                          \
    {                                                                                              \
        for (int i = 0; i < manual_label_count; i++) {                                             \
            free(manual_label_names[i]);                                                           \
        }                                                                                          \
        gvFreeRenderData(render_data);                                                             \
        if (g) {                                                                                   \
            gvFreeLayout(gvc, g);                                                                  \
            agclose(g);                                                                            \
        }                                                                                          \
        gvFinalize(gvc);                                                                           \
        gvFreeContext(gvc);                                                                        \
    }

/**
 * @brief Render a graphviz graph to svg from a dot string.
 *
 * Output buffer layout is:
 * - 1 byte for result (0: OK, 1: error);
 * - 1 byte for sizeof(int);
 * - coordinates of overridden native labels: for each such label,
 *   - sizeof(int) bytes for x,
 *   - sizeof(int) bytes for y;
 * - coordinates of manually overridden labels: for each such label,
 *   - sizeof(int) bytes for the number of times it is used,
 *   - for each use:
 *     - sizeof(int) bytes for x,
 *     - sizeof(int) bytes for y;
 * - the rest is SVG data.
 * All integers are encoded in in big-endian.
 *
 * @param font_size_len the length of the integer encoding the font size
 * @param dot_len the length of the buffer containing the dot string
 * @param native_label_dimensions_len the length of the buffer containing the dimensions of the
 * native labels generated by Typst
 * @param manual_label_dimensions_len the length of the buffer containing the dimensions of the
 * manual labels generated by Typst
 * @param manual_label_names_len the length of the buffer containing the names of the manual labels
 * generated by Typst
 * @param engine_len the length of the buffer containing the engine string
 * @return int 0 on success, 1 on failure
 */
EMSCRIPTEN_KEEPALIVE
int render(size_t font_size_len, size_t dot_len, size_t native_label_dimensions_len,
           size_t manual_label_dimensions_len, size_t manual_label_names_len, size_t engine_len) {
    // Set error report to custom function. Lets us get error messages from Graphviz.
    agseterr(AGERR);
    agseterrf(vizErrorf);

    INIT_BUFFER_UNPACK(font_size_len + dot_len + native_label_dimensions_len +
                       manual_label_dimensions_len + manual_label_names_len + engine_len);
    double font_size;
    NEXT_DOUBLE(font_size);
    char dot[dot_len + 1];
    NEXT_SIZED_STR(dot, dot_len);
    int native_label_count = native_label_dimensions_len / TYPST_INT_SIZE / 2;
    double native_label_widths[native_label_count];
    double native_label_heights[native_label_count];
    for (int i = 0; i < native_label_count; i++) {
        NEXT_DOUBLE(native_label_widths[i]);
        NEXT_DOUBLE(native_label_heights[i]);
    }
    int manual_label_count = manual_label_dimensions_len / TYPST_INT_SIZE / 2;
    double manual_label_widths[manual_label_count];
    double manual_label_heights[manual_label_count];
    for (int i = 0; i < manual_label_count; i++) {
        NEXT_DOUBLE(manual_label_widths[i]);
        NEXT_DOUBLE(manual_label_heights[i]);
    }
    char *manual_label_names[manual_label_count];
    for (int i = 0; i < manual_label_count; i++) {
        NEXT_STR(manual_label_names[i]);
    }
    char engine[engine_len + 1];
    NEXT_SIZED_STR(engine, engine_len);
    FREE_BUFFER();

    char *render_data = NULL;

    // Passing `NULL` sets a default value for the next graphs.
    agattr(NULL, AGRAPH, "pad", "0.0555"); // 4pt, Graphviz default

    GVC_t *gvc = gvContextPlugins(lt_preloaded_symbols, false);
    graph_t *g = agmemread(dot);

    if (!g) {
        FREE_EVERYTHING();
        wasm_minimal_protocol_send_result_to_host((uint8_t *)errBuff, strlen(errBuff));
        return 0;
    }

    // Passing a graph sets the value for the graph.
    agattr(g, AGRAPH, "bgcolor", "transparent");
    agattr(g, AGRAPH, "margin", "0");

    {
        char font_size_string[128];
        snprintf(font_size_string, 128, "%fpt", font_size);
        agattr(NULL, AGRAPH, "fontsize", font_size_string);
        agattr(NULL, AGNODE, "fontsize", font_size_string);
        agattr(NULL, AGEDGE, "fontsize", font_size_string);
    }

    int total_label_count = get_total_label_count(g);

    // Handle manually overridden labels.
    int manual_override_indices[total_label_count];
    manually_override_labels(g, manual_label_count, manual_label_widths, manual_label_heights,
                             manual_label_names, manual_override_indices);

    // Override native labels.
    bool is_native_label_overridden[total_label_count];
    override_native_labels(g, native_label_widths, native_label_heights, is_native_label_overridden,
                           manual_override_indices);

    // Layout graph.
    // FIXME: The call to `gvLayout` causes to reach an `unreachable` instruction if a (user-made)
    //  label uses invalid HTML tags, like <span>.
    if (gvLayout(gvc, g, engine) == -1) {
        FREE_EVERYTHING();
        wasm_minimal_protocol_send_result_to_host((uint8_t *)errBuff, strlen(errBuff));
        return 0;
    }

    // Render SVG.
    unsigned int svg_chunk_size;
    int result = gvRenderData(gvc, g, "svg", &render_data, &svg_chunk_size);
    if (result == -1) {
        FREE_EVERYTHING();
        char *err = "Diagraph error: failed to render graph to svg\0";
        wasm_minimal_protocol_send_result_to_host((uint8_t *)err, strlen(err));
        return 1;
    }

    double pad;
    {
        double pad_inches;
        if (sscanf(agget(g, "pad"), "%lf", &pad_inches) != 1) {
            // This case should never happen since we set a default value at the beginning.
            pad_inches = 0.0555; // Graphviz default
        }
        pad = pad_inches * 72.0; // 1in = 72pt
    }

    // Get native label positions.
    uint8_t native_label_positions[native_label_count * 2 * sizeof(int)];
    get_label_positions(g, pad, is_native_label_overridden, native_label_positions);

    // Get manual label positions.
    int total_manual_label_uses = 0;
    int manual_label_usage_count[manual_label_count];
    uint8_t manual_label_positions[manual_label_count][total_label_count * 2 * sizeof(int)];
    for (int manual_label_index = 0; manual_label_index < manual_label_count;
         manual_label_index++) {
        manual_label_usage_count[manual_label_index] = 0;
        bool is_manually_overridden_by_this_label[total_label_count];
        for (int i = 0; i < total_label_count; i++) {
            bool overridden = manual_override_indices[i] == manual_label_index;
            is_manually_overridden_by_this_label[i] = overridden;
            if (overridden) {
                manual_label_usage_count[manual_label_index]++;
                total_manual_label_uses++;
            }
        }
        get_label_positions(g, pad, is_manually_overridden_by_this_label,
                            manual_label_positions[manual_label_index]);
    }

    // Generate output.
    size_t output_buffer_len = 2 + sizeof(native_label_positions) +
                               sizeof(int) * manual_label_count +
                               sizeof(int) * 2 * total_manual_label_uses + svg_chunk_size;
    uint8_t *output_buffer = malloc(output_buffer_len);
    size_t offset = 0;
    output_buffer[offset++] = 0;
    output_buffer[offset++] = sizeof(int);
    memcpy(output_buffer + offset, native_label_positions, sizeof(native_label_positions));
    offset += sizeof(native_label_positions);
    for (int manual_label_index = 0; manual_label_index < manual_label_count;
         manual_label_index++) {
        int count = manual_label_usage_count[manual_label_index];
        big_endian_encode(output_buffer + offset, count);
        offset += sizeof(int);
        memcpy(output_buffer + offset, manual_label_positions[manual_label_index],
               count * 2 * sizeof(int));
        offset += count * 2 * sizeof(int);
    }
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
