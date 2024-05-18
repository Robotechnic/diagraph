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

#include "color/color.h"
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
 * @brief Default values for a label.
 *
 * @param label the label to set default values to
 */
void default_label_values(LabelInfo *label) {
    label->label = NULL;
    label->math_mode = false;
    label->font_size = 0;
    label->font_name = NULL;
    label->color = 0;
    label->override_xlabel = false;
    label->xlabel = NULL;
}

/**
 * @brief Check if the the node label is overridden and if not if the label is a math expression.
 *
 * @param n the node
 * @param label_infos the label infos
 * @param name the name of the node
 * @param label the label of the node
 * @param is_manually_overridden whether the label is manually overridden
 * @return int 0 on success, 1 on failure
 */
int process_node_label(Agnode_t *n, LabelInfo *label_infos, const char *name, const char *label,
                       bool is_manually_overridden) {
    if (!is_manually_overridden) {
        // If there is no explicitely set label for a node, so Graphviz intuitively sets its
        // "label" attribute to "\\N".
        bool has_label = strcmp(label, "\\N") != 0;
        if (!has_label) {
            label = agnameof(n);
        }
        label_infos->label = malloc(strlen(label) + 1);
        if (!label_infos->label) {
            ERROR("Failed to allocate memory for native labels");
            return 1;
        }
        strcpy(label_infos->label, label);
        label_infos->label[strlen(label)] = '\0';
        label_infos->math_mode = !has_label && is_math(label);
    } else {
		label_infos->label = NULL;
        label_infos->math_mode = false;
    }
    label_infos->name = malloc(strlen(name) + 1);
    if (!label_infos->name) {
        ERROR("Failed to allocate memory for label name");
        return 1;
    }
    strcpy(label_infos->name, name);
    label_infos->name[strlen(name)] = '\0';
    return 0;
}

/**
 * @brief Process the font name for a node label.
 *
 * @param n the node
 * @param label_infos the label infos
 */
void process_font_name(Agnode_t *n, LabelInfo *label_infos) {
    char *fn = agget(n, "fontname");
    if (fn) {
        label_infos->font_name = malloc(strlen(fn) + 1);
        strcpy(label_infos->font_name, fn);
    } else {
        label_infos->font_name = NULL;
    }
}

/**
 * @brief Check if the the xlabel label is overridden and if not if the label is a math expression.
 *
 * @param n the node
 * @param label_infos the label infos
 * @param is_xlabel_overridden whether the xlabel label is manually overridden
 *
 * @return int 0 on success, 1 on failure
 */
int process_xlabel_label(Agnode_t *n, LabelInfo *label_infos, bool is_xlabel_overridden) {
    const char *xlabel = agget(n, "xlabel");
	label_infos->override_xlabel = is_xlabel_overridden;
    if (!is_xlabel_overridden && xlabel) {
        label_infos->xlabel = malloc(strlen(xlabel) + 1);
        if (!label_infos->xlabel) {
            ERROR("Failed to allocate memory for xlabel");
            return 1;
        }
        strcpy(label_infos->xlabel, xlabel);
        label_infos->xlabel[strlen(xlabel)] = '\0';
        label_infos->xlabel_math_mode = is_math(xlabel);
    } else {
        label_infos->xlabel = NULL;
        label_infos->xlabel_math_mode = false;
    }
    return 0;
}

/**
 * @brief Return a list of all labels.
 */
EMSCRIPTEN_KEEPALIVE int get_labels(size_t buffer_len) {
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

    LabelsInfos nLabels = {0};
    nLabels.labels_len = get_total_label_count(g);
    nLabels.labels = malloc(nLabels.labels_len * sizeof(LabelInfo));
    if (!nLabels.labels) {
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
        bool is_xlabel_overridden = false;
        for (int i = 0; i < labels.labels_len; i++) {
            if (strcmp(labels.labels[i].label, name) == 0) {
                is_manually_overridden = labels.labels[i].content;
                is_xlabel_overridden = labels.labels[i].xlabel;
                break;
            }
        }

        const char *label = agget(n, "label");
        if (aghtmlstr(label)) {
            default_label_values(&nLabels.labels[label_index]);
            nLabels.labels[label_index].html = true;
        } else {
            nLabels.labels[label_index].html = false;

            if (process_node_label(n, &nLabels.labels[label_index], name, label, is_manually_overridden) ||
                process_xlabel_label(n, &nLabels.labels[label_index], is_xlabel_overridden)) {
                free_overriddenLabels(&labels);
                free_LabelsInfos(&nLabels);
                gvFreeLayout(gvc, g);
                agclose(g);
                return 1;
            }

            process_font_name(n, &nLabels.labels[label_index]);
            nLabels.labels[label_index].native = !is_manually_overridden;
            nLabels.labels[label_index].color = color_to_int(agget(n, "fontcolor"));
            nLabels.labels[label_index].font_size = atof(agget(n, "fontsize"));
        }
        label_index++;
    }

    gvFreeLayout(gvc, g);
    agclose(g);
    gvFinalize(gvc);

    if ((err = encode_LabelsInfos(&nLabels))) {
        if (err == 1) {
            ERROR("Failed to allocate memory for native labels");
        } else if (err == 2) {
            ERROR("Failed to encode native labels");
        } else {
            ERROR("Failled to encode native labels");
        }
        free_overriddenLabels(&labels);
        free_LabelsInfos(&nLabels);
        return 1;
    }

    free_overriddenLabels(&labels);
    free_LabelsInfos(&nLabels);
    return 0;
}

/**
 * Creates and returns a Graphviz label that has the specified dimensions.
 */
char *create_label_for_dimension(graph_t *g, double width, double height) {
    char label[2048];
    snprintf(label, sizeof(label),
             "<TABLE BORDER=\"0\"  FIXEDSIZE=\"true\" WIDTH=\"%lf\" "
             "HEIGHT=\"%lf\"><TR><TD></TD></TR></TABLE>",
             width, height);
    return agstrdup_html(g, label);
}

/**
 * Replaces labels for each nodes with invisible labels that have specific dimensions.
 */
void override_labels(graph_t *g, const renderGraph *renderInfo) {
    int label_index = 0;

    for (Agnode_t *n = agfstnode(g); n; n = agnxtnode(g, n)) {
        if (renderInfo->labels[label_index].override) {
            agset(n, "label",
                  create_label_for_dimension(g, renderInfo->labels[label_index].width,
                                             renderInfo->labels[label_index].height));
        }
        if (renderInfo->labels[label_index].xoverride) {
            agset(n, "xlabel",
				  create_label_for_dimension(g, renderInfo->labels[label_index].xwidth,
											 renderInfo->labels[label_index].xheight));
        }
        label_index++;
    }
}

/**
 * Encodes the position of considered labels in the output buffer.
 */
void get_label_coordinates(graph_t *g, double pad, const SizedLabel *labels, Coordinates *output) {
    int label_index = 0;

    for (Agnode_t *n = agfstnode(g); n; n = agnxtnode(g, n)) {
        output[label_index].x = (float)ND_coord(n).x + pad;
        output[label_index].y = (float)ND_coord(n).y + pad;
		if (labels[label_index].xoverride) {
			pointf pos = ND_xlabel(n)->pos;
			output[label_index].xx = pos.x + pad;
			output[label_index].xy = pos.y + pad;
		} else {
			output[label_index].xx = 0;
			output[label_index].xy = 0;
		}
        label_index++;
    }
}

#define FREE_EVERYTHING()                                                                                    \
    {                                                                                                        \
        free_renderGraph(&renderInfo);                                                                       \
        free_graphInfo(&g_render);                                                                           \
        if (g) {                                                                                             \
            gvFreeLayout(gvc, g);                                                                            \
            agclose(g);                                                                                      \
        }                                                                                                    \
        gvFinalize(gvc);                                                                                     \
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

    DEBUG("Got %d labels\n", renderInfo.labels_len);

    g_render.labels_len = renderInfo.labels_len;
    g_render.labels = calloc(g_render.labels_len, sizeof(LabelInfo));
    if (!g_render.labels) {
        ERROR("Failed to allocate memory for native labels");
        free_renderGraph(&renderInfo);
        return 1;
    }

#ifdef TEST
    GVC_t *gvc = gvContext();
#else
    GVC_t *gvc = gvContextPlugins(lt_preloaded_symbols, false);
#endif

    graph_t *g = agmemread(renderInfo.dot);

    // Passing `NULL` sets a default value for the next graphs.
    agattr(g, AGRAPH, "pad", "0.0555"); // 4pt, Graphviz default

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
        snprintf(font_size_string, 128, "%fpt", renderInfo.font_size);
        agattr(g, AGRAPH, "fontsize", font_size_string);
        agattr(g, AGNODE, "fontsize", font_size_string);
        agattr(g, AGEDGE, "fontsize", font_size_string);
    }

    DEBUG("Total label count: %d\n", get_total_label_count(g));

    // Override labels with invisible labels that have specific dimensions.
    override_labels(g, &renderInfo);

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
    get_label_coordinates(g, pad, renderInfo.labels, g_render.labels);

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
