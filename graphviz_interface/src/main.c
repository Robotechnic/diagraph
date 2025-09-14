#ifdef TEST
#define EMSCRIPTEN_KEEPALIVE
#define PACKAGE_VERSION "2.47.0"
#define DEBUG(...) printf(__VA_ARGS__)
#define ERROR(str) printf("Error: %s\n", str)
#define DEBUG_BLOCK(block)                                                                                   \
    do {                                                                                                     \
        block                                                                                                \
    } while (0)
#else
#define DEBUG(...)
#define DEBUG_BLOCK(block)
#define ERROR(str) write_error_message(str)
#include "protocol/plugin.h"
#endif

#define GRAPHVIZ_ERROR wasm_minimal_protocol_send_result_to_host((uint8_t *)errBuff, strlen(errBuff))

/// Show debug boxes around labels and cluster labels.
// #define SHOW_DEBUG_BOXES

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
    const char *intro = "Diagraph error: ";
    size_t intro_len = strlen(intro);
    strncpy(errBuff + intro_len, str, sizeof(errBuff) - intro_len);
    memcpy(errBuff, intro, intro_len);
    return 0;
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
void default_node_label_values(NodeLabelInfo *label) {
    memset(label, 0, sizeof(NodeLabelInfo));
}

int safe_strncmp(const char *s1, const char *s2, size_t n) {
    if (s1 == NULL && s2 == NULL) {
        return 0;
    }
    if (s1 == NULL) {
        return -1;
    }
    if (s2 == NULL) {
        return 1;
    }
    return strncmp(s1, s2, n);
}

int safe_strcmp(const char *s1, const char *s2) {
    if (s1 == NULL && s2 == NULL) {
        return 0;
    }
    if (s1 == NULL) {
        return -1;
    }
    if (s2 == NULL) {
        return 1;
    }
    return strcmp(s1, s2);
}


bool node_math_mode(void *obj, const char *label, const char *math_attribute) {
    char *math = agget(obj, math_attribute);
    bool math_true = safe_strcmp(math, "true") == 0;
    bool math_false = safe_strcmp(math, "false") == 0;
    return (is_math(label) || math_true) && !math_false;
}

/**
 * @brief Extract labels, font names, color and font sizes from a node.
 *
 * @param n the node
 * @param label_infos the label infos
 * @param name the name of the node
 * @param label the label of the node
 * @return int 0 on success, 1 on failure
 */
int process_node_label(Agnode_t *n, NodeLabelInfo *label_infos, const char *name, const char *label) {
    if (safe_strncmp(agget(n, "shape"), "point", 5) == 0) {
        ERROR("Node shape is 'point', no label will be set");
        label_infos->label = NULL;
        label_infos->math_mode = false;
    } else {
        // If there is no explicitly set label for a node, so Graphviz intuitively sets its
        // "label" attribute to "\\N".
        bool has_label = safe_strcmp(label, "\\N") != 0;
        if (!has_label) {
            label = name;
        }
        size_t len = strlen(label);
        label_infos->label = malloc(len + 1);
        if (!label_infos->label) {
            ERROR("Failed to allocate memory for native labels");
            return 1;
        }
        strcpy(label_infos->label, label);
        label_infos->label[len] = '\0';
        label_infos->math_mode = node_math_mode(n, label, "math");
        DEBUG("Node %s: label='%s' math_mode=%d\n", name, label_infos->label, label_infos->math_mode);
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
void process_font_name(Agnode_t *n, NodeLabelInfo *label_infos) {
    const char *fn = agget(n, "fontname");
    if (fn) {
        label_infos->font_name = malloc(strlen(fn) + 1);
        strcpy(label_infos->font_name, fn);
        label_infos->font_name[strlen(fn)] = '\0';
    } else {
        label_infos->font_name = NULL;
    }
}

/**
 * @brief Process the font size for a node label.
 *
 * @param n the node
 * @param label_infos the label infos
 */
void process_fontsize(Agnode_t *n, NodeLabelInfo *label_infos) {
    const char *fontsize = agget(n, "fontsize");
    if (fontsize) {
        label_infos->font_size = (float)atof(fontsize);
    } else {
        label_infos->font_size = 0;
    }
}

void process_edge_font_name(Agedge_t *e, EdgeLabelInfo *label_infos) {
    const char *fn = agget(e, "fontname");
    if (fn) {
        label_infos->font_name = malloc(strlen(fn) + 1);
        strcpy(label_infos->font_name, fn);
    } else {
        label_infos->font_name = NULL;
    }
}

void process_edge_fontsize(Agedge_t *e, EdgeLabelInfo *label_infos) {
    const char *fontsize = agget(e, "fontsize");
    if (fontsize) {
        label_infos->font_size = (float)atof(fontsize);
    } else {
        label_infos->font_size = 0;
    }
}

/**
 * @brief Process the xlabel label for a node
 *
 * @param n the node
 * @param label_infos the label infos
 * @return int 0 on success, 1 on failure
 */
int process_xlabel_label(Agnode_t *n, NodeLabelInfo *label_infos) {
    const char *xlabel = agget(n, "xlabel");
    if (xlabel) {
        label_infos->xlabel = malloc(strlen(xlabel) + 1);
        if (!label_infos->xlabel) {
            ERROR("Failed to allocate memory for xlabel");
            return 1;
        }
        strcpy(label_infos->xlabel, xlabel);
        label_infos->xlabel[strlen(xlabel)] = '\0';
        label_infos->xlabel_math_mode = node_math_mode(n, xlabel, "xmath");
    } else {
        label_infos->xlabel = NULL;
        label_infos->xlabel_math_mode = false;
    }
    return 0;
}

int copy_label(Agedge_t *e, char *name, char **label, bool *math_mode) {
    const char *l = agget(e, name);
    if (!l || aghtmlstr(l)) {
        *label = NULL;
        *math_mode = false;
    } else {
        *label = malloc(strlen(l) + 1);
        if (!*label) {
            ERROR("Failed to allocate memory for label");
            return 1;
        }
        strcpy(*label, l);
        (*label)[strlen(l)] = '\0';
		char label[] = {name[0], 'm', 'a', 't', 'h', '\0'};
        *math_mode = node_math_mode(e, l, label);
    }
    return 0;
}

/**
 * @brief fill the label infos with edges labels
 *
 * @param g the graph we are working on
 * @param n the node we are working on
 * @param[out] labels the description of which labels are overwriten
 * @param[out] len the number of edges
 * @return int 1 on error, 0 otherwise
 */
int get_node_edges_labels(graph_t *g, Agnode_t *n, EdgeLabelInfo **labels, size_t *len) {
    *len = agdegree(g, n, false, true);
    *labels = malloc(*len * sizeof(EdgeLabelInfo));
    if (!*labels) {
        ERROR("Failed to allocate memory for edge labels");
        return 1;
    }
    int i = 0;
    for (Agedge_t *e = agfstout(g, n); e; e = agnxtout(g, e)) {
        if (copy_label(e, "label", &(*labels)[i].label, &(*labels)[i].label_math_mode) ||
            copy_label(e, "xlabel", &(*labels)[i].xlabel, &(*labels)[i].xlabel_math_mode) ||
            copy_label(e, "headlabel", &(*labels)[i].headlabel, &(*labels)[i].headlabel_math_mode) ||
            copy_label(e, "taillabel", &(*labels)[i].taillabel, &(*labels)[i].taillabel_math_mode)) {
            return 1;
        }
        process_edge_font_name(e, &(*labels)[i]);
        process_edge_fontsize(e, &(*labels)[i]);
        (*labels)[i].color = color_to_int(agget(e, "fontcolor"));
        const char *to = agnameof(aghead(e));
        (*labels)[i].to = malloc(strlen(to) + 1);
        strcpy((*labels)[i].to, to);
        (*labels)[i].to[strlen(to)] = '\0';
        if (i > 0 && safe_strcmp((*labels)[i].to, (*labels)[i - 1].to) == 0) {
            (*labels)[i].index = (*labels)[i - 1].index + 1;
        } else {
            (*labels)[i].index = 0;
        }
        i++;
    }

    return 0;
}

/**
 * @brief fill the label infos with nodes labels and xlabels data
 *
 * @param g the graph we are working on
 * @param labels the description of which labels are overwriten
 * @param nLabels the labels and xlabel infos
 * @return int 1 on error, 0 otherwise
 */
int get_nodes_labels(graph_t *g, const GetGraphInfo *labels, GraphInfo *nLabels) {
    int label_index = 0;

    // Get node labels.
    for (Agnode_t *n = agfstnode(g); n; n = agnxtnode(g, n)) {
        default_node_label_values(&nLabels->labels[label_index]);
        const char *name = agnameof(n);
        const char *label = agget(n, "label");
        DEBUG("Processing node %s with label %s\n", name, label ? label : "NULL");
        if (aghtmlstr(label)) {
            DEBUG("Node %s: HTML label detected\n", name);
            size_t name_len = strlen(name);
            nLabels->labels[label_index].html_mode = true;
            nLabels->labels[label_index].name = malloc(name_len + 1);
            if (!nLabels->labels[label_index].name) {
                ERROR("Failed to allocate memory for node name");
                return 1;
            }
            strcpy(nLabels->labels[label_index].name, name);
            nLabels->labels[label_index].name[name_len] = '\0';

            size_t label_len = strlen(label);
            nLabels->labels[label_index].label = malloc(label_len+ 1);
            if (!nLabels->labels[label_index].label) {
                ERROR("Failed to allocate memory for node label");
                free(nLabels->labels[label_index].name);
                return 1;
            }
            strcpy(nLabels->labels[label_index].label, label);
            nLabels->labels[label_index].label[label_len] = '\0';
        } else {
            DEBUG("Node %s: Text label detected\n", name);
            if (process_node_label(n, &nLabels->labels[label_index], name, label) ||
                process_xlabel_label(n, &nLabels->labels[label_index])) {
                return 1;
            }

            process_font_name(n, &nLabels->labels[label_index]);
            process_fontsize(n, &nLabels->labels[label_index]);
            nLabels->labels[label_index].color = color_to_int(agget(n, "fontcolor"));
        }

        DEBUG("Node %s: label='%s' math_mode=%d html_mode=%d xlabel='%s' xlabel_math_mode=%d xlabel_html_mode=%d "
              "font_name='%s' font_size=%f color=%d\n",
              nLabels->labels[label_index].name,
              nLabels->labels[label_index].label ? nLabels->labels[label_index].label : "NULL",
              nLabels->labels[label_index].math_mode, nLabels->labels[label_index].html_mode,
              nLabels->labels[label_index].xlabel ? nLabels->labels[label_index].xlabel : "NULL",
              nLabels->labels[label_index].xlabel_math_mode, nLabels->labels[label_index].xlabel_html_mode,
              nLabels->labels[label_index].font_name ? nLabels->labels[label_index].font_name : "NULL",
              nLabels->labels[label_index].font_size, nLabels->labels[label_index].color);
        get_node_edges_labels(g, n, &nLabels->labels[label_index].edges_infos,
                              &nLabels->labels[label_index].edges_infos_len);

        label_index++;
    }
    return 0;
}

/**
 * @brief Default values for a cluster label.
 *
 * @param ClusterLabelInfo the default values
 */
void default_cluster_label_values(ClusterLabelInfo *label) {
    memset(label, 0, sizeof(ClusterLabelInfo));
}

/**
 * @brief Process the font name for a subgrah label
 *
 * @param g the cluster
 * @param label_infos the label infos
 */
void process_cluster_font_name(Agraph_t *g, ClusterLabelInfo *label_infos) {
    const char *fn = agget(g, "fontname");
    if (fn) {
        label_infos->font_name = malloc(strlen(fn) + 1);
        strcpy(label_infos->font_name, fn);
    } else {
        label_infos->font_name = NULL;
    }
}

int process_cluster_label(Agraph_t *sg, const char *name, const char *label, ClusterLabelInfo *label_infos) {
    label_infos->name = malloc(strlen(name) + 1);
    if (!label_infos->name) {
        ERROR("Failed to allocate memory for cluster name");
        return 1;
    }
    strcpy(label_infos->name, name);
    label_infos->name[strlen(name)] = '\0';
    if (label == NULL) {
        label_infos->label = NULL;
        label_infos->math_mode = false;
    } else {
        label_infos->label = malloc(strlen(label) + 1);
        if (!label_infos->label) {
            ERROR("Failed to allocate memory for cluster label");
            return 1;
        }
        strcpy(label_infos->label, label);
        label_infos->label[strlen(label)] = '\0';
        label_infos->math_mode = node_math_mode(sg, label, "math");
    }
    label_infos->color = color_to_int(agget(sg, "fontcolor"));
    const char *fontsize = agget(sg, "fontsize");
    if (fontsize) {
        label_infos->font_size = (float)atof(agget(sg, "fontsize"));
    } else {
        label_infos->font_size = 0;
    }
    process_cluster_font_name(sg, label_infos);
    return 0;
}

/**
 * @brief fill the label infos with cluster labels
 *
 * @param g the graph we are working on
 * @param labels the description of which labels are overwriten
 * @param sgLabels the clusters labels infos
 * @return int 1 on error, 0 otherwise
 */
int get_cluster_labels(graph_t *g, const GetGraphInfo *labels, GraphInfo *sgLabels, int *label_index) {
    int cluster_count = agnsubg(g);
    if (cluster_count == 0) {
        return 0;
    }
    bool clean = false;
    if (sgLabels->cluster_labels == NULL) {
        sgLabels->cluster_labels_len = cluster_count;
        sgLabels->cluster_labels = malloc(sizeof(ClusterLabelInfo) * cluster_count);
        clean = true;
    } else {
        sgLabels->cluster_labels_len += cluster_count;
        sgLabels->cluster_labels =
            realloc(sgLabels->cluster_labels, sizeof(ClusterLabelInfo) * sgLabels->cluster_labels_len);
    }
    if (sgLabels->cluster_labels == NULL) {
        ERROR("Failed to allocate memory for cluster labels");
        return 1;
    }
    for (Agraph_t *sg = agfstsubg(g); sg; sg = agnxtsubg(sg)) {
        const char *name = agnameof(sg);
        // the name must be "cluster_([0-9]+)"
        if (safe_strncmp(name, "cluster_", 8) != 0 || strlen(name) < 9) {
            get_cluster_labels(sg, labels, sgLabels, label_index);
            continue;
        }

        const char *label = agget(sg, "label");
        if (aghtmlstr(label)) {
            default_cluster_label_values(&sgLabels->cluster_labels[*label_index]);
        } else if (process_cluster_label(sg, name, label, &sgLabels->cluster_labels[*label_index])) {
            return 1;
        }
        (*label_index)++;
        get_cluster_labels(sg, labels, sgLabels, label_index);
    }

    if (clean) {
        sgLabels->cluster_labels_len = *label_index;
        // this resize the buffer to a size lower or equal to the previous one
        // no need to check for errors because the pointer won't be NULL
        sgLabels->cluster_labels =
            realloc(sgLabels->cluster_labels, sizeof(ClusterLabelInfo) * sgLabels->cluster_labels_len);
    }

    return 0;
}

/**
 * @brief Return a list of all labels.
 */
EMSCRIPTEN_KEEPALIVE
int get_labels(size_t buffer_len) {
    // Set error report to custom function. Lets us get error messages from Graphviz.
    agseterr(AGERR);
    agseterrf(vizErrorf);

    GetGraphInfo labels = {0};
    int err;
    if ((err = decode_GetGraphInfo(buffer_len, &labels))) {
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
    if (!gvc) {
        ERROR("Failed to create Graphviz context");
        return 1;
    }
    graph_t *g = agmemread(labels.dot);

    if (!g) {
        free_GetGraphInfo(&labels);
        gvFinalize(gvc);
        gvFreeContext(gvc);

        GRAPHVIZ_ERROR;
        return 1;
    }

    agattr_text(g, AGNODE, "xlabel", "");

    GraphInfo nLabels = {0};
    nLabels.labels_len = agnnodes(g);
    nLabels.labels = malloc(nLabels.labels_len * sizeof(NodeLabelInfo));
    if (!nLabels.labels) {
        ERROR("Failed to allocate memory for native labels");
        free_GetGraphInfo(&labels);
        gvFinalize(gvc);
        gvFreeContext(gvc);
        return 1;
    }
    DEBUG("Init done\n");

    int index = 0;
    err = get_nodes_labels(g, &labels, &nLabels) || get_cluster_labels(g, &labels, &nLabels, &index);
    gvFreeLayout(gvc, g);
    agclose(g);
    gvFinalize(gvc);
    gvFreeContext(gvc);

    if (err) {
        free_GetGraphInfo(&labels);
        free_GraphInfo(&nLabels);
        return 1;
    }

    DEBUG("Encoding labels\n");
    if ((err = encode_GraphInfo(&nLabels))) {
        if (err == 1) {
            ERROR("Failed to allocate memory for native labels");
        } else if (err == 2) {
            ERROR("Failed to encode native labels");
        } else {
            ERROR("Failled to encode native labels");
        }
        free_GetGraphInfo(&labels);
        free_GraphInfo(&nLabels);
        return 1;
    }

    free_GetGraphInfo(&labels);
    free_GraphInfo(&nLabels);
    return 0;
}

/**
 * Creates and returns a Graphviz label that has the specified dimensions.
 */
char *create_label_for_dimension(graph_t *g, double width, double height) {
    if (width < 1e-5 && height < 1e-5) {
        return agstrdup_text(g, "");
    }
    char label[2048];
    snprintf(label, sizeof(label),
             "<TABLE "
#ifndef SHOW_DEBUG_BOXES
             "BORDER=\"0\" "
#endif
             "FIXEDSIZE=\"true\" WIDTH=\"%lf\" "
             "HEIGHT=\"%lf\"><TR><TD></TD></TR></TABLE>",
             width, height);
    return agstrdup_html(g, label);
}

/**
 * Replaces labels for each nodes with invisible labels that have specific dimensions.
 */
void overwrite_labels(graph_t *g, const renderGraph *renderInfo) {
    int label_index = 0;

    for (Agnode_t *n = agfstnode(g); n; n = agnxtnode(g, n)) {
        if (renderInfo->labels[label_index].overwrite) {
            agset_html(n, "label",
                       create_label_for_dimension(g, renderInfo->labels[label_index].width,
                                                  renderInfo->labels[label_index].height));
        }
        if (renderInfo->labels[label_index].xoverwrite) {
            agset_html(n, "xlabel",
                       create_label_for_dimension(g, renderInfo->labels[label_index].xwidth,
                                                  renderInfo->labels[label_index].xheight));
        }
        int edge_label_index = 0;
        for (Agedge_t *e = agfstout(g, n); e; e = agnxtout(g, e)) {
            SizedEdgeLabel *edges_infos = &renderInfo->labels[label_index].edges_size[edge_label_index];
            if (edges_infos->overwrite) {
                agset_html(e, "label", create_label_for_dimension(g, edges_infos->width, edges_infos->height));
            }
            if (edges_infos->xoverwrite) {
                agset_html(e, "xlabel", create_label_for_dimension(g, edges_infos->xwidth, edges_infos->xheight));
            }
            if (edges_infos->headoverwrite) {
                agset_html(e, "headlabel",
                      create_label_for_dimension(g, edges_infos->headwidth, edges_infos->headheight));
            }
            if (edges_infos->tailoverwrite) {
                agset_html(e, "taillabel",
                      create_label_for_dimension(g, edges_infos->tailwidth, edges_infos->tailheight));
            }
            edge_label_index++;
        }
        label_index++;
    }
}

/**
 * @brief Replaces labels for each cluster with invisible labels that have specific dimensions.
 * This function also set the cluster margins correctly
 *
 * @param g the current graph we are working on
 * @param renderInfo structure containing the cluster labels sizes
 * @param label_index the current index of the cluster label
 */
void overwrite_cluster_labels(graph_t *g, const renderGraph *renderInfo, int *label_index) {
    for (Agraph_t *sg = agfstsubg(g); sg; sg = agnxtsubg(sg)) {
        if (agget(sg, "margin") == NULL) {
            agset_text(sg, "margin", "8");
        }

        const char *name = agnameof(sg);
        // the name must be "cluster_([0-9]+)"
        if (safe_strncmp(name, "cluster_", 8) != 0 || strlen(name) < 9) {
            overwrite_cluster_labels(sg, renderInfo, label_index);
            continue;
        }
        agset_html(sg, "label",
              create_label_for_dimension(g, renderInfo->cluster_labels[*label_index].width,
                                         renderInfo->cluster_labels[*label_index].height));

        (*label_index)++;
        overwrite_cluster_labels(sg, renderInfo, label_index);
    }
}

/**
 * @brief Encodes the position of considered edges labels in the output buffer.
 *
 * @param g the current graph we are working on
 * @param n the current node we are working on
 * @param output the output coordinates
 * @param label_index the current index of the node
 * @param labels the labels to encode
 * @param pad the padding to apply to the labels
 */
void get_edges_labels_coordinates(graph_t *g, Agnode_t *n, NodeCoordinates *output, int label_index,
                                  const SizedNodeLabel *labels, float pad) {
    int edge_label_index = 0;
    for (Agedge_t *e = agfstout(g, n); e; e = agnxtout(g, e)) {
        EdgeCoordinates *edge_coords = &output[label_index].edges[edge_label_index];
        SizedEdgeLabel *edges_infos = &labels[label_index].edges_size[edge_label_index];
        if (edges_infos->overwrite) {
            pointf pos = ED_label(e)->pos;
            edge_coords->x = (float)pos.x + pad;
            edge_coords->y = (float)pos.y + pad;
        } else {
            edge_coords->x = 0;
            edge_coords->y = 0;
        }
        if (edges_infos->xoverwrite) {
            pointf pos = ED_xlabel(e)->pos;
            edge_coords->xx = (float)pos.x + pad;
            edge_coords->xy = (float)pos.y + pad;
        } else {
            edge_coords->xx = 0;
            edge_coords->xy = 0;
        }
        if (edges_infos->headoverwrite) {
            pointf pos = ED_head_label(e)->pos;
            edge_coords->headx = (float)pos.x + pad;
            edge_coords->heady = (float)pos.y + pad;
        } else {
            edge_coords->headx = 0;
            edge_coords->heady = 0;
        }
        if (edges_infos->tailoverwrite) {
            pointf pos = ED_tail_label(e)->pos;
            edge_coords->tailx = (float)pos.x + pad;
            edge_coords->taily = (float)pos.y + pad;
        } else {
            edge_coords->tailx = 0;
            edge_coords->taily = 0;
        }
        edge_label_index++;
    }
}

/**
 * @brief Encodes the position of considered labels in the output buffer.
 *
 * @param g the current graph we are working on
 * @param pad the padding to apply to the labels
 * @param labels the labels to encode
 * @param output the output coordinates
 */
void get_label_coordinates(graph_t *g, float pad, const SizedNodeLabel *labels, NodeCoordinates *output) {
    int label_index = 0;

    for (Agnode_t *n = agfstnode(g); n; n = agnxtnode(g, n)) {
        output[label_index].x = (float)ND_coord(n).x + pad;
        output[label_index].y = (float)ND_coord(n).y + pad;
        if (labels[label_index].xoverwrite) {
            pointf pos = ND_xlabel(n)->pos;
            output[label_index].xx = (float)pos.x + pad;
            output[label_index].xy = (float)pos.y + pad;
        } else {
            output[label_index].xx = 0;
            output[label_index].xy = 0;
        }
        output[label_index].edges_len = labels[label_index].edges_size_len;
        output[label_index].edges = malloc(output[label_index].edges_len * sizeof(EdgeCoordinates));
        get_edges_labels_coordinates(g, n, output, label_index, labels, pad);
        label_index++;
    }
}

/**
 * @brief Encodes the position of considered cluster labels in the output buffer.
 *
 * @param g the current graph we are working on
 * @param pad the padding to apply to the labels
 * @param labels the labels to encode
 * @param output the output coordinates
 * @param index the current index of the cluster label
 */
void get_cluster_label_coordinates(Agraph_t *g, float pad, const SizedClusterLabel *labels,
                                   ClusterCoordinates *output, int *index) {
    for (Agraph_t *sg = agfstsubg(g); sg; sg = agnxtsubg(sg)) {
        const char *name = agnameof(sg);
        // the name must be "cluster_([0-9]+)"
        if (safe_strncmp(name, "cluster_", 8) != 0 || strlen(name) < 9) {
            get_cluster_label_coordinates(sg, pad, labels, output, index);
            continue;
        }
        pointf pos = GD_label(sg)->pos;
        output[*index].x = (float)pos.x + pad;
        output[*index].y = (float)pos.y + pad;
        (*index)++;
        get_cluster_label_coordinates(sg, pad, labels, output, index);
    }
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

    int err;
    if ((err = decode_renderGraph(buffer_len, &renderInfo))) {
        if (err == 1) {
            ERROR("Failed to allocate memory for render graph");
        } else if (err == 2) {
            ERROR("Buffer length is too small (should not happen)");
        } else {
            ERROR("Failed to decode render graph");
        }
        return 1;
    }

    DEBUG("Got %d labels\n", renderInfo.labels_len);

    g_render.labels_len = renderInfo.labels_len;
    if (g_render.labels_len > 0) {
        g_render.labels = malloc(g_render.labels_len * sizeof(NodeLabelInfo));
        if (!g_render.labels) {
            ERROR("Failed to allocate memory for native labels");
            free_renderGraph(&renderInfo);
            return 1;
        }
    }

    g_render.cluster_labels_len = renderInfo.cluster_labels_len;
    if (g_render.cluster_labels_len > 0) {
        g_render.cluster_labels = malloc(g_render.cluster_labels_len * sizeof(ClusterLabelInfo));
        if (!g_render.cluster_labels) {
            ERROR("Failed to allocate memory for cluster labels");
            free_renderGraph(&renderInfo);
            free_graphInfo(&g_render);
            return 1;
        }
    }

#ifdef TEST
    GVC_t *gvc = gvContext();
#else
    GVC_t *gvc = gvContextPlugins(lt_preloaded_symbols, false);
#endif
    if (!gvc) {
        ERROR("Failed to create Graphviz context");
        free_renderGraph(&renderInfo);
        return 1;
    }

    graph_t *g = agmemread(renderInfo.dot);
    if (!g) {
        free_renderGraph(&renderInfo);
        free_graphInfo(&g_render);
        gvFinalize(gvc);
        gvFreeContext(gvc);
        GRAPHVIZ_ERROR;
        return 1;
    }

    agattr_text(g, AGRAPH, "label", "");
    agattr_text(g, AGRAPH, "pad", "0.0555"); // 4pt, Graphviz default
    agattr_text(g, AGNODE, "xlabel", "");
    agattr_text(g, AGEDGE, "label", "");
    agattr_text(g, AGEDGE, "xlabel", "");
    agattr_text(g, AGEDGE, "taillabel", "");
    agattr_text(g, AGEDGE, "headlabel", "");

    // Passing a graph sets the value for the graph.
    agattr_text(g, AGRAPH, "bgcolor", "transparent");
    agattr_text(g, AGRAPH, "margin", "0");
    
    agset_text(g, "size", NULL);

    {
        char font_size_string[128];
        snprintf(font_size_string, 128, "%fpt", renderInfo.font_size);
        agattr_text(g, AGRAPH, "fontsize", font_size_string);
        agattr_text(g, AGNODE, "fontsize", font_size_string);
        agattr_text(g, AGEDGE, "fontsize", font_size_string);
    }

    DEBUG("Total label count: %d\n", agnnodes(g));

    // overwrite labels with invisible labels that have specific dimensions.
    overwrite_labels(g, &renderInfo);
    int index = 0;
    overwrite_cluster_labels(g, &renderInfo, &index);

    char* rotate = agget(g, "rotate");
    if (!rotate || rotate[0] == '\0') {
        char *orientation = agget(g, "orientation");
        if (orientation && orientation[0] == 'l' || orientation[0] == 'L') {
            g_render.landscape = true;
            agset_text(g, "orientation", "\0");
        }
        char *landscape = agget(g, "landscape");
        if (safe_strcmp(landscape, "true") == 0) {
            g_render.landscape = true;
        } else {
            g_render.landscape = false;
        }
        agset_text(g, "landscape", "false");
    } else if (safe_strcmp(rotate, "90") == 0) {
        g_render.landscape = true;
        agset_text(g, "rotate", "0");
    }
    
    // Layout graph.
    // FIXME: The call to `gvLayout` causes to reach an `unreachable` instruction if a (user-made)
    //  label uses invalid HTML tags, like <span>.
    if (gvLayout(gvc, g, renderInfo.engine) == -1) {
        free_renderGraph(&renderInfo);
        free_graphInfo(&g_render);
        gvFinalize(gvc);
        gvFreeContext(gvc);
        free(g);
        GRAPHVIZ_ERROR;
        return 1;
    }

    // Render SVG.
    size_t svg_chunk_size;
    err = gvRenderData(gvc, g, "svg", &g_render.svg, &svg_chunk_size);
    if (err == -1) {
        free_renderGraph(&renderInfo);
        free_graphInfo(&g_render);
        agclose(g);
        gvFinalize(gvc);
        gvFreeContext(gvc);
        ERROR("Diagraph error: failed to render graph to svg\0");
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

    // Get label positions.
    get_label_coordinates(g, (float)pad, renderInfo.labels, g_render.labels);
    index = 0;
    get_cluster_label_coordinates(g, (float)pad, renderInfo.cluster_labels, g_render.cluster_labels, &index);


    gvFreeLayout(gvc, g);
    agclose(g);
    gvFinalize(gvc);
    gvFreeContext(gvc);
    free_renderGraph(&renderInfo);

    // Generate output.
    if ((err = encode_graphInfo(&g_render))) {
        free_graphInfo(&g_render);

        if (err == 1) {
            ERROR("Failed to allocate memory for graph info");
        } else if (err == 2) {
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

EMSCRIPTEN_KEEPALIVE
int engine_list() {
#ifdef TEST
    GVC_t *gvc = gvContext();
#else
    GVC_t *gvc = gvContextPlugins(lt_preloaded_symbols, false);
#endif
    if (!gvc) {
        ERROR("Failed to create Graphviz context");
        return 1;
    }
    char *kind = "layout";
    int size = 0;
    char **engines_list = gvPluginList(gvc, kind, &size);
    if (!engines_list) {
        ERROR("Failed to get engines list");
        return 1;
    }
#ifdef TEST
    for (int i = 0; i < size; i++) {
        DEBUG("Engine: %s\n", *(engines_list + i));
    }
#endif
    Engines engines;
    engines.engines_len = size;
    engines.engines = engines_list;
    if (encode_Engines(&engines)) {
        ERROR("Failed to encode engines list");
        return 1;
    }
    free_Engines(&engines);
    return 0;
}

EMSCRIPTEN_KEEPALIVE
int convert_color(const char *color) {
    if (!color) {
        ERROR("Color is null");
        return 1;
    }
    int color_int = color_to_int(color);
    if (color_int == -1) {
        ERROR("Failed to convert color");
        return 1;
    }
    char color_str[11];
    snprintf(color_str, sizeof(color_str), "#%08x", color_int);
    wasm_minimal_protocol_send_result_to_host((uint8_t *)color_str, strlen(color_str));
    return 0;
}
