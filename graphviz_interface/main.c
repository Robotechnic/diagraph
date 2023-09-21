#include "emscripten.h"

#include <string.h>
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
    strncpy(errBuff + 7, str, sizeof(errBuff) - 7);
    errBuff[0] = 'e';
    errBuff[1] = 'r';
    errBuff[2] = 'r';
    errBuff[3] = 'o';
    errBuff[4] = 'r';
    errBuff[5] = ':';
    errBuff[6] = ' ';
    return 0;
}

/**
 * @brief Render a graphviz graph to svg from a dot string
 * 
 * @param dot_len the length of the buffer containing the dot string
 * @return int 0 on success, 1 on failure
 */
EMSCRIPTEN_KEEPALIVE
int render(size_t dot_len, size_t engine_len) {
    // set error report to custom function
    // it allow us to get the error message from graphviz
    agseterr(AGERR);
    agseterrf(vizErrorf);

    uint8_t *buffer = malloc(dot_len + engine_len + 1);

    if (!buffer) {
        return 1;
    }
    wasm_minimal_protocol_write_args_to_buffer(buffer);
    // to make sure the buffer is null terminated
    buffer[dot_len + engine_len] = '\0';

    char const* dot = (char *)buffer;
    char const* engine = (char *)buffer + dot_len;

    GVC_t *gvc = gvContextPlugins(lt_preloaded_symbols, true);
    graph_t *g = agmemread(dot);
    char* data = NULL;
    unsigned int length;

    if (!g) {
        wasm_minimal_protocol_send_result_to_host((uint8_t *)errBuff, strlen(errBuff));
        free(buffer);
        return 0;
    }

    if (gvLayout(gvc, g, engine) == -1) {
        wasm_minimal_protocol_send_result_to_host((uint8_t *)errBuff, strlen(errBuff));
        free(buffer);
		return 0;
	}

    int result = gvRenderData(gvc, g, "svg", &data, &length);
    if (result == -1) {
        gvFreeRenderData(data);
        free(buffer);
        char* err = "error: failed to render graph to svg\0";
        wasm_minimal_protocol_send_result_to_host((uint8_t *)err, strlen(err));
        return 1;
    }

    wasm_minimal_protocol_send_result_to_host((uint8_t *)data, length);
    gvFreeRenderData(data);
    gvFreeLayout(gvc, g);
    agclose(g);
	gvFinalize(gvc);
    gvFreeContext(gvc);
    free(buffer);
    return 0;
}

EMSCRIPTEN_KEEPALIVE
int get_version() {
    wasm_minimal_protocol_send_result_to_host((uint8_t *)PACKAGE_VERSION, strlen(PACKAGE_VERSION));
    return 0;
}