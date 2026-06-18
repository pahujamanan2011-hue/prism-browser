#ifndef PRISM_JS_RUNTIME_H
#define PRISM_JS_RUNTIME_H

#include <stdbool.h>
#include <stdint.h>
#include "parser.h"

bool js_runtime_init(void);
void js_runtime_cleanup(void);
bool js_runtime_execute(const char* code);
bool js_runtime_execute_page(ParsedPage* page);
void* js_runtime_get_context(void);

#endif /* PRISM_JS_RUNTIME_H */