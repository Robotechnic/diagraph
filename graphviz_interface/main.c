#include "emscripten.h"

#include <string.h>
#include <graphviz/gvc.h>


#define PROTOCOL_FUNCTION __attribute__((import_module("typst_env"))) extern

PROTOCOL_FUNCTION void wasm_minimal_protocol_send_result_to_host(const uint8_t *ptr, size_t len);
PROTOCOL_FUNCTION void wasm_minimal_protocol_write_args_to_buffer(uint8_t *ptr);

EMSCRIPTEN_KEEPALIVE
int render(size_t arg1_len) {
    uint8_t *arg1 = malloc(arg1_len);
    if (!arg1) {
        return 1;
    }
    wasm_minimal_protocol_write_args_to_buffer(arg1);
    
    char errbuf[4096];
    GVC_t *gvc = gvContext();
    graph_t *g = agmemread((char *)arg1);
    if (!g) {
        char *err = malloc(strlen(errbuf) + 7);
        err = strcat(err, "error: ");
        err = strcat(err, errbuf);
        wasm_minimal_protocol_send_result_to_host((uint8_t *)err, strlen(err));
        return 1;
    }

    if (gvLayout(gvc, g, "dot")) {
        char *err = malloc(strlen(errbuf) + 7);
        err = strcat(err, "error: ");
        err = strcat(err, errbuf);
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
    free(arg1);
    return 0;
}