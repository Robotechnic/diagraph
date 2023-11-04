#include "emscripten.h"

#include "protocol/protocol.h"
#include "utils/utils.h"
#include <graphviz/gvc.h>
#include <graphviz/gvplugin.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define INIT_BUFFER_UNPACK(buffer_len)                                                             \
    size_t offset = 0;                                                                             \
    uint8_t *buffer = malloc(buffer_len);                                                          \
    if (!buffer) {                                                                                 \
        return 1;                                                                                  \
    }                                                                                              \
    wasm_minimal_protocol_write_args_to_buffer(buffer);

#define NEXT_CHAR(name, len)                                                                       \
    char *name = malloc(len + 1);                                                                  \
    memcpy(name, buffer + offset, len);                                                            \
    name[len] = '\0';                                                                              \
    offset += len;

#define NEXT_INT(name)                                                                             \
    int name = big_endian_decode(buffer + offset);                                                 \
    offset += sizeof(int);

#define FREE_BUFFER()                                                                              \
    free(buffer);                                                                                  \
    buffer = NULL;

char errBuff[1024];
int vizErrorf(char *str) {
    strncpy(errBuff + 8, str, sizeof(errBuff) - 8);
    errBuff[0] = 1;
    errBuff[1] = 'e';
    errBuff[2] = 'r';
    errBuff[3] = 'r';
    errBuff[4] = 'o';
    errBuff[5] = 'r';
    errBuff[6] = ':';
    errBuff[7] = ' ';
    return 0;
}

/**
 * @brief Render a graphviz graph to svg from a dot string
 *
 * @param dot_len the length of the buffer containing the dot string
 * @param engine_len the length of the buffer containing the engine string
 * @param background_len the length of the buffer containing the background color string
 * @return int 0 on success, 1 on failure
 */
EMSCRIPTEN_KEEPALIVE
int render(size_t dot_len, size_t engine_len, size_t background_len) {
    // set error report to custom function
    // it allow us to get the error message from graphviz
    agseterr(AGERR);
    agseterrf(vizErrorf);

    INIT_BUFFER_UNPACK(dot_len + engine_len + background_len)
    NEXT_CHAR(dot, dot_len)
    NEXT_CHAR(engine, engine_len)
    NEXT_CHAR(background, background_len)
    FREE_BUFFER()

    GVC_t *gvc = gvContextPlugins(lt_preloaded_symbols, false);
    graph_t *g = agmemread(dot);

    if (strlen(background) > 0) {
        agattr(g, AGRAPH, "bgcolor", background);
    }

    agattr(g, AGRAPH, "center", "true");
    agattr(g, AGRAPH, "margin", "0");

    free(background);
    free(dot);
    char *data = NULL;
    unsigned int length;

    if (!g) {
        free(engine);
        wasm_minimal_protocol_send_result_to_host((uint8_t *)errBuff, strlen(errBuff));
        return 0;
    }

    if (gvLayout(gvc, g, engine) == -1) {
        free(engine);
        wasm_minimal_protocol_send_result_to_host((uint8_t *)errBuff, strlen(errBuff));
        return 0;
    }

    int result = gvRenderData(gvc, g, "svg", &data, &length);
    free(engine);
    if (result == -1) {
        gvFreeRenderData(data);
        char *err = "error: failed to render graph to svg\0";
        wasm_minimal_protocol_send_result_to_host((uint8_t *)err, strlen(err));
        return 1;
    }

    int width = (int)floor(GD_bb(g).UR.x - GD_bb(g).LL.x) + 8;
    int height = (int)floor(GD_bb(g).UR.y - GD_bb(g).LL.y) + 8;

    size_t result_buffer_len = 0;
    result_buffer_len = sizeof(width) + sizeof(height) + length + 2;
    uint8_t *result_buffer = malloc(result_buffer_len);
    result_buffer[0] = 0;
    result_buffer[1] = sizeof(width);
    big_endian_encode(result_buffer + 2, width);
    big_endian_encode(result_buffer + 2 + sizeof(width), height);
    memcpy(result_buffer + 2 + sizeof(width) + sizeof(height), data, length);

    wasm_minimal_protocol_send_result_to_host(result_buffer, result_buffer_len);
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