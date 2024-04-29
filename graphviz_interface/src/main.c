#ifdef TEST
#define EMSCRIPTEN_KEEPALIVE
#define PACKAGE_VERSION "2.47.0"
#define DEBUG(...) printf(__VA_ARGS__)
#define ERROR(str) printf("Error: %s\n", str)
#else
#define DEBUG(...)
#define ERROR(str) write_error_message(str)
#include "protocol/plugin.h"
#endif
#include "protocol/protocol.h"

#include "math/math.h"
#include <graphviz/gvc.h>
#include <graphviz/gvplugin.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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
 * @brief Write an error message to the host.
 *
 * @param message the message to write
 */
void write_error_message(char *message) {
    wasm_minimal_protocol_send_result_to_host((uint8_t *)message, strlen(message));
}

/**
 * @brief Return a list of all labels.
 */
EMSCRIPTEN_KEEPALIVE
int get_labels(size_t buffer_len) {
    // Set error report to custom function. Lets us get error messages from Graphviz.
    agseterr(AGERR);
    agseterrf(vizErrorf);

    overriddenLabels labels = {0};
    int err;
    if ((err = decode_overriddenLabels(buffer_len, &labels))) {
        if (err == 1) {
            ERROR("Failled to allocate memory for overridden labels");
        } else if (err == 2) {
            ERROR("Buffer length is too small");
        } else {
            ERROR("Failed to decode overridden labels");
        }
        return 1;
    }

#ifdef TEST
        GVC_t *gvc = gvContext();
#else
        GVC_t *gvc = gvContextPlugins(lt_preloaded_symbols, false);
#endif
    graph_t *g = agmemread(labels.dot);

    if (!g) {
        free_overriddenLabels(&labels);
        gvFinalize(gvc);
		gvFreeContext(gvc);

        wasm_minimal_protocol_send_result_to_host((uint8_t *)errBuff, strlen(errBuff));
        return 1;
    }

    nativeLabels nLabels = {0};
    nLabels.nativeLabels_len = get_total_label_count(g);
    nLabels.nativeLabels = malloc(nLabels.nativeLabels_len * sizeof(NativeLabel));
    if (!nLabels.nativeLabels) {
        ERROR("Failed to allocate memory for native labels");
        free_overriddenLabels(&labels);
        gvFinalize(gvc);
        gvFreeContext(gvc);
        return 1;
    }

    int label_index = 0;

    // Get node labels.
    for (Agnode_t *n = agfstnode(g); n; n = agnxtnode(g, n)) {
        const char *name = agnameof(n);
        bool is_manually_overridden = false;
        for (int i = 0; i < labels.labels_len; i++) {
            if (strcmp(labels.labels[i], name) == 0) {
                is_manually_overridden = true;
            }
        }
        char *label = agget(n, "label");
        if (!aghtmlstr(label) && !is_manually_overridden) {
            // If there is no explicitely set label for a node, so Graphviz intuitively sets its
            // "label" attribute to "\\N".
            bool has_label = strcmp(label, "\\N") != 0;
            if (!has_label) {
                label = agnameof(n);
            }
            nLabels.nativeLabels[label_index].label = malloc(strlen(label) + 1);
            if (!nLabels.nativeLabels[label_index].label) {
                ERROR("Failed to allocate memory for native labels");
                free_overriddenLabels(&labels);
                free_nativeLabels(&nLabels);
                gvFreeLayout(gvc, g);
                agclose(g);
                return 1;
            }
            strcpy(nLabels.nativeLabels[label_index].label, label);
            nLabels.nativeLabels[label_index].label[strlen(label)] = '\0';
            nLabels.nativeLabels[label_index].mathMode = !has_label && is_math(label);
            label_index++;
        } else {
            nLabels.nativeLabels[label_index].label = NULL;
            nLabels.nativeLabels[label_index].mathMode = false;
            label_index++;
        }
    }

    gvFreeLayout(gvc, g);
    agclose(g);
    gvFinalize(gvc);

    if ((err = encode_nativeLabels(&nLabels))) {
        if (err == 1) {
            ERROR("Failed to allocate memory for native labels");
        } else if (err == 2) {
            ERROR("Failed to encode native labels");
        } else {
            ERROR("Failled to encode native labels");
        }
        free_overriddenLabels(&labels);
        free_nativeLabels(&nLabels);
        return 1;
    }

    free_overriddenLabels(&labels);
    free_nativeLabels(&nLabels);
    return 0;
}

/**
 * Creates and returns a Graphviz label that has the specified dimensions.
 */
char *create_label_for_dimension(graph_t *g, double width, double height) {
    char label[2048];
    snprintf(label, sizeof(label),
             "<TABLE BORDER=\"0\" FIXEDSIZE=\"true\" WIDTH=\"%lf\" "
             "HEIGHT=\"%lf\"><TR><TD></TD></TR></TABLE>",
             width, height);
    return agstrdup_html(g, label);
}

/**
 * Replaces labels for nodes whose name corresponds to a label name with invisible labels that have
 * specific dimensions. For each node whose label is manually overridden, writes the index of the
 * new label in the corresponding cell in `manual_override_indices`. Writes `-1` in
 * `manual_override_indices` for other nodes.
 */
void manually_override_labels(graph_t *g, const renderGraph *renderInfo,
                              int *manual_override_indices) {
    int label_index = 0;

    for (Agnode_t *n = agfstnode(g); n; n = agnxtnode(g, n)) {
        manual_override_indices[label_index] = -1;
        for (int i = 0; i < renderInfo->nativeLabels_len; i++) {
            if (strcmp(renderInfo->manualLabels[i].label, agnameof(n)) == 0) {
                agset(n, "label",
                      create_label_for_dimension(g, renderInfo->manualLabels[i].width,
                                                 renderInfo->manualLabels[i].height));
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
void override_native_labels(graph_t *g, const renderGraph *renderInfo, bool *is_label_overridden,
                            const int *manual_override_indices) {
    int label_index = 0;
    int native_override_chain_index = 0;

    for (Agnode_t *n = agfstnode(g); n; n = agnxtnode(g, n)) {
        is_label_overridden[label_index] = false;
        if (manual_override_indices[label_index] == -1 && !aghtmlstr(agget(n, "label"))) {
            agset(n, "label",
                  create_label_for_dimension(
                      g, renderInfo->nativeLabels[native_override_chain_index].width,
                      renderInfo->nativeLabels[native_override_chain_index].height));
            is_label_overridden[label_index] = true;
            native_override_chain_index++;
        }
        label_index++;
    }
}

/**
 * Encodes the position of considered labels in the output buffer.
 */
void get_label_positions(graph_t *g, double pad, const bool *consider, Coordinates *output) {
    int label_index = 0;
    int considered_count = 0;

    for (Agnode_t *n = agfstnode(g); n; n = agnxtnode(g, n)) {
        if (consider[label_index]) {
            output[considered_count].x = (float)ND_coord(n).x + pad;
            output[considered_count].y = (float)ND_coord(n).y + pad;
            considered_count++;
        }
        label_index++;
    }
}

#define FREE_EVERYTHING()                                                                          \
    {                                                                                              \
        free_renderGraph(&renderInfo);                                                             \
        free_graphInfo(&g_render);                                                                 \
        if (g) {                                                                                   \
            gvFreeLayout(gvc, g);                                                                  \
            agclose(g);                                                                            \
        }                                                                                          \
        gvFinalize(gvc);                                                                           \
    }

/**
 * @brief Render a graphviz graph to svg from a dot string.
 *
 * @param buffer_len length of the input buffer
 * @return int 0 on success, 1 on failure
 */
EMSCRIPTEN_KEEPALIVE
int render(size_t buffer_len) {
    // Set error report to custom function. Lets us get error messages from Graphviz.
    agseterr(AGERR);
    agseterrf(vizErrorf);

    renderGraph renderInfo = {0}; // input data
    graphInfo g_render = {0};     // output data

    int res;
    if ((res = decode_renderGraph(buffer_len, &renderInfo))) {
        if (res == 1) {
            ERROR("Failed to allocate memory for render graph");
        } else if (res == 2) {
            ERROR("Buffer length is too small (should not happen)");
        } else {
            ERROR("Failed to decode render graph");
        }
        return 1;
    }

    DEBUG("Got %d native labels and %d manual labels\n", renderInfo.nativeLabels_len,
          renderInfo.manualLabels_len);

    g_render.nativeLabels_len = renderInfo.nativeLabels_len;
    g_render.nativeLabels = calloc(g_render.nativeLabels_len, sizeof(Coordinates));
    if (!g_render.nativeLabels) {
        ERROR("Failed to allocate memory for native labels");
        free_renderGraph(&renderInfo);
        return 1;
    }
    g_render.manualLabels_len = renderInfo.manualLabels_len;
    g_render.manualLabels = calloc(g_render.manualLabels_len, sizeof(Coordinates));
    if (!g_render.manualLabels) {
        ERROR("Failed to allocate memory for manual labels");
        free_renderGraph(&renderInfo);
        return 1;
    }

    // Passing `NULL` sets a default value for the next graphs.
    agattr(NULL, AGRAPH, "pad", "0.0555"); // 4pt, Graphviz default

#ifdef TEST
        GVC_t *gvc = gvContext();
#else
        GVC_t *gvc = gvContextPlugins(lt_preloaded_symbols, false);
#endif
    
    graph_t *g = agmemread(renderInfo.dot);

    if (!g) {
        free_renderGraph(&renderInfo);
        free_graphInfo(&g_render);
        gvFinalize(gvc);
        wasm_minimal_protocol_send_result_to_host((uint8_t *)errBuff, strlen(errBuff));
        return 0;
    }

    // Passing a graph sets the value for the graph.
    agattr(g, AGRAPH, "bgcolor", "transparent");
    agattr(g, AGRAPH, "margin", "0");

    {
        char font_size_string[128];
        snprintf(font_size_string, 128, "%fpt", renderInfo.fontSize);
        agattr(NULL, AGRAPH, "fontsize", font_size_string);
        agattr(NULL, AGNODE, "fontsize", font_size_string);
        agattr(NULL, AGEDGE, "fontsize", font_size_string);
    }

    int total_label_count = get_total_label_count(g);

    DEBUG("Total label count: %d\n", total_label_count);

    // Handle manually overridden labels.
    int manual_override_indices[total_label_count];
    manually_override_labels(g, &renderInfo, manual_override_indices);

    // Override native labels.
    bool is_native_label_overridden[total_label_count];
    override_native_labels(g, &renderInfo, is_native_label_overridden, manual_override_indices);

    // Layout graph.
    // FIXME: The call to `gvLayout` causes to reach an `unreachable` instruction if a (user-made)
    //  label uses invalid HTML tags, like <span>.
    if (gvLayout(gvc, g, renderInfo.engine) == -1) {
        free_renderGraph(&renderInfo);
        free_graphInfo(&g_render);
        gvFinalize(gvc);
        wasm_minimal_protocol_send_result_to_host((uint8_t *)errBuff, strlen(errBuff));
        return 0;
    }

    // Render SVG.
    unsigned int svg_chunk_size;
    int result = gvRenderData(gvc, g, "svg", &g_render.svg, &svg_chunk_size);
    if (result == -1) {
        free_renderGraph(&renderInfo);
        free_graphInfo(&g_render);
        gvFinalize(gvc);
        ERROR("\0Diagraph error: failed to render graph to svg\0");
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
    get_label_positions(g, pad, is_native_label_overridden, g_render.nativeLabels);

    // Get manual label positions.
    for (int i = 0; i < total_label_count; i++) {
        is_native_label_overridden[i] = !is_native_label_overridden[i];
    }
    get_label_positions(g, pad, is_native_label_overridden, g_render.manualLabels);

    agclose(g);
    gvFinalize(gvc);
	free_renderGraph(&renderInfo);

    // Generate output.
    if ((res = encode_graphInfo(&g_render))) {
        free_graphInfo(&g_render);
		
        if (res == 1) {
            ERROR("Failed to allocate memory for graph info");
        } else if (res == 2) {
            ERROR("Buffer length is too small (should not happen)");
        } else {
            ERROR("Failed to encode graph info");
        }

        return 1;
    }

    free_graphInfo(&g_render);

    return 0;
}

EMSCRIPTEN_KEEPALIVE
int get_version() {
    wasm_minimal_protocol_send_result_to_host((uint8_t *)PACKAGE_VERSION, strlen(PACKAGE_VERSION));
    return 0;
}
