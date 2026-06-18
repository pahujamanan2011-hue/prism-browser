#include "js_runtime.h"
#include "config.h"
#include <duktape.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static duk_context* duk_ctx = NULL;
static bool initialized = false;

static duk_ret_t js_console_log(duk_context* ctx) {
    int n = duk_get_top(ctx);
    for (int i = 0; i < n; i++) {
        const char* msg = duk_to_string(ctx, i);
        if (msg) {
            fprintf(stdout, "[JS] %s\n", msg);
        }
    }
    return 0;
}

static duk_ret_t js_console_error(duk_context* ctx) {
    int n = duk_get_top(ctx);
    for (int i = 0; i < n; i++) {
        const char* msg = duk_to_string(ctx, i);
        if (msg) {
            fprintf(stderr, "[JS ERROR] %s\n", msg);
        }
    }
    return 0;
}

static duk_ret_t js_storage_set(duk_context* ctx) {
    const char* key = duk_to_string(ctx, 0);
    const char* value = duk_to_string(ctx, 1);
    
    if (key && value) {
        char* storage_key = malloc(strlen("prism_storage_") + strlen(key) + 1);
        if (storage_key) {
            sprintf(storage_key, "prism_storage_%s", key);
            /* In production, this would use a persistent storage backend */
            duk_put_global_string(ctx, storage_key);
            free(storage_key);
        }
    }
    return 0;
}

static duk_ret_t js_storage_get(duk_context* ctx) {
    const char* key = duk_to_string(ctx, 0);
    if (key) {
        char* storage_key = malloc(strlen("prism_storage_") + strlen(key) + 1);
        if (storage_key) {
            sprintf(storage_key, "prism_storage_%s", key);
            duk_get_global_string(ctx, storage_key);
            free(storage_key);
            return 1;
        }
    }
    duk_push_null(ctx);
    return 1;
}

static duk_ret_t js_fetch(duk_context* ctx) {
    const char* url = duk_to_string(ctx, 0);
    if (url) {
        duk_push_string(ctx, "Prism Browser only supports basic JS execution");
    } else {
        duk_push_null(ctx);
    }
    return 1;
}

static void register_global_functions(duk_context* ctx) {
    duk_push_c_function(ctx, js_console_log, DUK_VARARGS);
    duk_put_global_string(ctx, "console.log");
    
    duk_push_c_function(ctx, js_console_error, DUK_VARARGS);
    duk_put_global_string(ctx, "console.error");
    
    duk_push_c_function(ctx, js_storage_set, 2);
    duk_put_global_string(ctx, "localStorage.setItem");
    
    duk_push_c_function(ctx, js_storage_get, 1);
    duk_put_global_string(ctx, "localStorage.getItem");
    
    duk_push_c_function(ctx, js_fetch, 1);
    duk_put_global_string(ctx, "fetch");
}

bool js_runtime_init(void) {
    if (initialized) return true;
    
    duk_ctx = duk_create_heap_default();
    if (!duk_ctx) return false;
    
    register_global_functions(duk_ctx);
    
    const char* common_js = 
        "window = this;\n"
        "document = { getElementById: function(id) { return null; } };\n"
        "console.log('Prism Browser JavaScript Runtime initialized');\n";
    
    if (duk_peval_string(duk_ctx, common_js) != 0) {
        fprintf(stderr, "JS init error: %s\n", duk_safe_to_string(duk_ctx, -1));
        duk_pop(duk_ctx);
        duk_destroy_heap(duk_ctx);
        duk_ctx = NULL;
        return false;
    }
    duk_pop(duk_ctx);
    
    initialized = true;
    return true;
}

void js_runtime_cleanup(void) {
    if (duk_ctx) {
        duk_destroy_heap(duk_ctx);
        duk_ctx = NULL;
    }
    initialized = false;
}

bool js_runtime_execute(const char* code) {
    if (!initialized || !duk_ctx || !code) return false;
    
    if (duk_peval_string(duk_ctx, code) != 0) {
        fprintf(stderr, "JS execution error: %s\n", duk_safe_to_string(duk_ctx, -1));
        duk_pop(duk_ctx);
        return false;
    }
    duk_pop(duk_ctx);
    return true;
}

bool js_runtime_execute_page(ParsedPage* page) {
    if (!initialized || !duk_ctx || !page) return false;
    
    /* Extract any JavaScript from the page */
    for (int i = 0; i < page->element_count; i++) {
        if (page->elements[i].type == ELEMENT_SCRIPT && page->elements[i].content) {
            js_runtime_execute(page->elements[i].content);
        }
    }
    return true;
}

void* js_runtime_get_context(void) {
    return duk_ctx;
}