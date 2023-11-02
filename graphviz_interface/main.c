#include "emscripten.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <graphviz/gvc.h>
#include <graphviz/gvplugin.h>

#define PROTOCOL_FUNCTION __attribute__((import_module("typst_env"))) extern

PROTOCOL_FUNCTION void wasm_minimal_protocol_send_result_to_host(const uint8_t *ptr, size_t len);
PROTOCOL_FUNCTION void wasm_minimal_protocol_write_args_to_buffer(uint8_t *ptr);

extern gvplugin_library_t gvplugin_dot_layout_LTX_library;
extern gvplugin_library_t gvplugin_neato_layout_LTX_library;

extern gvplugin_library_t gvplugin_core_LTX_library;

lt_symlist_t lt_preloaded_symbols[] = {
    {"gvplugin_dot_layout_LTX_library", &gvplugin_dot_layout_LTX_library},
    {"gvplugin_neato_layout_LTX_library", &gvplugin_neato_layout_LTX_library},
    {"gvplugin_core_LTX_library", &gvplugin_core_LTX_library},
    {0, 0}
};

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

void big_endian_encode(uint8_t *buffer, int value) {
    for (int i = 0; i < sizeof(int); i++) {
        buffer[i] = (value >> (8 * (sizeof(int) - i - 1))) & 0xFF;
    }
}

/**
 * @brief Render a graphviz graph to svg from a dot string
 * 
 * @param dot_len the length of the buffer containing the dot string
 * @return int 0 on success, 1 on failure
 */
EMSCRIPTEN_KEEPALIVE
int render(size_t dot_len, size_t engine_len, size_t background_len) {
    // set error report to custom function
    // it allow us to get the error message from graphviz
    agseterr(AGERR);
    agseterrf(vizErrorf);

    uint8_t *buffer = malloc(dot_len + engine_len + background_len);

    if (!buffer) {
      return 1;
    }
    wasm_minimal_protocol_write_args_to_buffer(buffer);

    // this copy is needed because the buffer
    // is not null terminated and because
    // the buffer doesn't reset properly when
    // watch mode is enabled
    char *dot = malloc(dot_len + 1);
    char *engine = malloc(engine_len + 1);
    char *background = malloc(background_len + 1);
    memcpy(dot, buffer, dot_len);
    dot[dot_len] = '\0';
    memcpy(engine, buffer + dot_len, engine_len);
    engine[engine_len] = '\0';
    memcpy(background, buffer + dot_len + engine_len, background_len);
    background[background_len] = '\0';
    free(buffer);

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
      wasm_minimal_protocol_send_result_to_host((uint8_t *)errBuff,
                                                strlen(errBuff));
      return 0;
    }

    if (gvLayout(gvc, g, engine) == -1) {
      free(engine);
      wasm_minimal_protocol_send_result_to_host((uint8_t *)errBuff,
                                                strlen(errBuff));
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

    // display bounding box of the svg render in the console
    // this is useful to debug the svg render
    int width = (int)floor(GD_bb(g).UR.x - GD_bb(g).LL.x);
    int height = (int)floor(GD_bb(g).UR.y - GD_bb(g).LL.y);

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