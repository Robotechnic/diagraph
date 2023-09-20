#include "emscripten.h"

#include <string.h>
#include <graphviz/gvc.h>


#define PROTOCOL_FUNCTION __attribute__((import_module("typst_env"))) extern

PROTOCOL_FUNCTION void wasm_minimal_protocol_send_result_to_host(const uint8_t *ptr, size_t len);
PROTOCOL_FUNCTION void wasm_minimal_protocol_write_args_to_buffer(uint8_t *ptr);

/**
 * @brief Render a graphviz graph to svg from a dot string
 * 
 * @param arg1_len the length of the buffer containing the dot string
 * @return int 0 on success, 1 on failure
 */
EMSCRIPTEN_KEEPALIVE
int render(size_t arg1_len) {
    uint8_t *arg1 = malloc(arg1_len);
    if (!arg1) {
        return 1;
    }
    wasm_minimal_protocol_write_args_to_buffer(arg1);
    
    char errbuf[4096] = "error: ";
    setbuf(stderr, errbuf + 7);
    GVC_t *gvc = gvContext();
    graph_t *g = agmemread((char *)arg1);
    free(arg1);
    if (!g) {
        wasm_minimal_protocol_send_result_to_host((uint8_t *)errbuf, strlen(errbuf));
        return 1;
    }

    if (gvLayout(gvc, g, "dot")) {
        wasm_minimal_protocol_send_result_to_host((uint8_t *)errbuf, strlen(errbuf));
		return 1;
	}

    char* data;
    unsigned int length;

    if (gvRenderData(gvc, g, "svg", &data, &length)) {
        gvFreeRenderData(data);
        return 1;
    }
    if (g) {
	    gvFreeLayout(gvc, g);
    }
	gvFinalize(gvc);
    gvFreeContext(gvc);
    agclose(g);


    wasm_minimal_protocol_send_result_to_host((uint8_t *)data, length);
    return 0;
}