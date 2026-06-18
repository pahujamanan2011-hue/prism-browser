/*
 *  Duktape public API header.
 *
 *  See the API documentation for documentation on the API calls.
 *
 *  Copyright (c) 2013-2017 by Duktape authors (see AUTHORS)
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */

#ifndef DUKTAPE_H_INCLUDED
#define DUKTAPE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* External duk_config.h provides platform specific includes and defines. */
#include "duk_config.h"

/* Duktape version detection. */
#define DUK_VERSION                       20000L
#define DUK_VERSION_MAJOR                 2
#define DUK_VERSION_MINOR                 0
#define DUK_VERSION_PATCH                 0

/* Duktape public API defines and typedefs. */
typedef struct duk_hthread duk_context;
typedef struct duk_memory_functions duk_memory_functions;

/* Memory functions. */
struct duk_memory_functions {
    void *(*alloc_func)(void *udata, size_t size);
    void *(*realloc_func)(void *udata, void *ptr, size_t size);
    void (*free_func)(void *udata, void *ptr);
    void *udata;
};

/* Error codes. */
#define DUK_ERR_NONE                      0
#define DUK_ERR_ERROR                     1
#define DUK_ERR_EVAL_ERROR                2
#define DUK_ERR_RANGE_ERROR               3
#define DUK_ERR_REFERENCE_ERROR           4
#define DUK_ERR_SYNTAX_ERROR              5
#define DUK_ERR_TYPE_ERROR                6
#define DUK_ERR_URI_ERROR                 7

/* Heap creation flags. */
#define DUK_HEAP_FLAG_DEBUG               0x0001

/* Context creation flags. */
#define DUK_CTX_FLAG_DEBUG                0x0001

/* Type masks. */
#define DUK_TYPE_MASK_NONE                (1U << 0)
#define DUK_TYPE_MASK_UNDEFINED           (1U << 1)
#define DUK_TYPE_MASK_NULL                (1U << 2)
#define DUK_TYPE_MASK_BOOLEAN             (1U << 3)
#define DUK_TYPE_MASK_NUMBER              (1U << 4)
#define DUK_TYPE_MASK_STRING              (1U << 5)
#define DUK_TYPE_MASK_OBJECT              (1U << 6)
#define DUK_TYPE_MASK_BUFFER              (1U << 7)
#define DUK_TYPE_MASK_POINTER             (1U << 8)
#define DUK_TYPE_MASK_LIGHTFUNC           (1U << 9)

#define DUK_TYPE_MASK_THROW               0x8000U

/* API prototypes. */
duk_context *duk_create_heap(duk_memory_functions *mem_funcs, void *heap_udata);
duk_context *duk_create_heap_default(void);
void duk_destroy_heap(duk_context *ctx);

void duk_push_undefined(duk_context *ctx);
void duk_push_null(duk_context *ctx);
void duk_push_boolean(duk_context *ctx, duk_bool_t val);
void duk_push_true(duk_context *ctx);
void duk_push_false(duk_context *ctx);
void duk_push_number(duk_context *ctx, duk_double_t val);
void duk_push_int(duk_context *ctx, duk_int_t val);
void duk_push_uint(duk_context *ctx, duk_uint_t val);
void duk_push_string(duk_context *ctx, const char *str);
void duk_push_lstring(duk_context *ctx, const char *str, duk_size_t len);
void duk_push_pointer(duk_context *ctx, void *p);
void duk_push_object(duk_context *ctx);
void duk_push_array(duk_context *ctx);
void duk_push_c_function(duk_context *ctx, duk_c_function func, duk_int_t nargs);
void duk_push_c_lightfunc(duk_context *ctx, duk_c_function func, duk_int_t nargs, duk_int_t length, duk_int_t magic);
void duk_push_thread(duk_context *ctx);
void duk_push_thread_new_globalenv(duk_context *ctx);
void duk_push_global_object(duk_context *ctx);
void duk_push_current_function(duk_context *ctx);
void duk_push_current_thread(duk_context *ctx);
void duk_push_heapptr(duk_context *ctx, void *ptr);

duk_bool_t duk_is_undefined(duk_context *ctx, duk_idx_t idx);
duk_bool_t duk_is_null(duk_context *ctx, duk_idx_t idx);
duk_bool_t duk_is_boolean(duk_context *ctx, duk_idx_t idx);
duk_bool_t duk_is_number(duk_context *ctx, duk_idx_t idx);
duk_bool_t duk_is_string(duk_context *ctx, duk_idx_t idx);
duk_bool_t duk_is_object(duk_context *ctx, duk_idx_t idx);
duk_bool_t duk_is_array(duk_context *ctx, duk_idx_t idx);
duk_bool_t duk_is_function(duk_context *ctx, duk_idx_t idx);
duk_bool_t duk_is_c_function(duk_context *ctx, duk_idx_t idx);
duk_bool_t duk_is_ecmascript_function(duk_context *ctx, duk_idx_t idx);
duk_bool_t duk_is_bound_function(duk_context *ctx, duk_idx_t idx);
duk_bool_t duk_is_thread(duk_context *ctx, duk_idx_t idx);
duk_bool_t duk_is_buffer(duk_context *ctx, duk_idx_t idx);
duk_bool_t duk_is_pointer(duk_context *ctx, duk_idx_t idx);
duk_bool_t duk_is_lightfunc(duk_context *ctx, duk_idx_t idx);

duk_bool_t duk_to_boolean(duk_context *ctx, duk_idx_t idx);
duk_double_t duk_to_number(duk_context *ctx, duk_idx_t idx);
duk_int_t duk_to_int(duk_context *ctx, duk_idx_t idx);
duk_uint_t duk_to_uint(duk_context *ctx, duk_idx_t idx);
duk_int32_t duk_to_int32(duk_context *ctx, duk_idx_t idx);
duk_uint32_t duk_to_uint32(duk_context *ctx, duk_idx_t idx);
duk_double_t duk_to_double(duk_context *ctx, duk_idx_t idx);
const char *duk_to_string(duk_context *ctx, duk_idx_t idx);
const char *duk_to_lstring(duk_context *ctx, duk_idx_t idx, duk_size_t *out_len);
void *duk_to_buffer(duk_context *ctx, duk_idx_t idx, duk_size_t *out_len);
void *duk_to_pointer(duk_context *ctx, duk_idx_t idx);
const char *duk_safe_to_lstring(duk_context *ctx, duk_idx_t idx, duk_size_t *out_len);
const char *duk_safe_to_string(duk_context *ctx, duk_idx_t idx);

duk_idx_t duk_get_top(duk_context *ctx);
void duk_set_top(duk_context *ctx, duk_idx_t idx);
duk_idx_t duk_get_top_index(duk_context *ctx);
duk_idx_t duk_require_top_index(duk_context *ctx);
duk_bool_t duk_check_stack(duk_context *ctx, duk_idx_t extra);
void duk_swap(duk_context *ctx, duk_idx_t idx1, duk_idx_t idx2);
void duk_swap_top(duk_context *ctx, duk_idx_t idx);
void duk_dup(duk_context *ctx, duk_idx_t idx);
void duk_dup_top(duk_context *ctx);
void duk_insert(duk_context *ctx, duk_idx_t idx);
void duk_replace(duk_context *ctx, duk_idx_t idx);
void duk_copy(duk_context *ctx, duk_idx_t from_idx, duk_idx_t to_idx);
void duk_remove(duk_context *ctx, duk_idx_t idx);
void duk_xcopytop(duk_context *dst_ctx, duk_context *src_ctx, duk_idx_t count);
void duk_xmove_top(duk_context *dst_ctx, duk_context *src_ctx, duk_idx_t count);

duk_idx_t duk_require_stack(duk_context *ctx, duk_idx_t extra);

duk_int_t duk_get_type(duk_context *ctx, duk_idx_t idx);
duk_uint_t duk_get_type_mask(duk_context *ctx, duk_idx_t idx);
duk_bool_t duk_check_type_mask(duk_context *ctx, duk_idx_t idx, duk_uint_t mask);

duk_bool_t duk_get_boolean(duk_context *ctx, duk_idx_t idx);
duk_double_t duk_get_number(duk_context *ctx, duk_idx_t idx);
const char *duk_get_string(duk_context *ctx, duk_idx_t idx);
const char *duk_get_lstring(duk_context *ctx, duk_idx_t idx, duk_size_t *out_len);
void *duk_get_buffer(duk_context *ctx, duk_idx_t idx, duk_size_t *out_len);
void *duk_get_pointer(duk_context *ctx, duk_idx_t idx);
void *duk_get_heapptr(duk_context *ctx, duk_idx_t idx);

void duk_require_undefined(duk_context *ctx, duk_idx_t idx);
void duk_require_null(duk_context *ctx, duk_idx_t idx);
duk_bool_t duk_require_boolean(duk_context *ctx, duk_idx_t idx);
duk_double_t duk_require_number(duk_context *ctx, duk_idx_t idx);
const char *duk_require_string(duk_context *ctx, duk_idx_t idx);
const char *duk_require_lstring(duk_context *ctx, duk_idx_t idx, duk_size_t *out_len);
void *duk_require_buffer(duk_context *ctx, duk_idx_t idx, duk_size_t *out_len);
void *duk_require_pointer(duk_context *ctx, duk_idx_t idx);

void duk_pop(duk_context *ctx);
void duk_pop_n(duk_context *ctx, duk_idx_t count);
void duk_pop_2(duk_context *ctx);
void duk_pop_3(duk_context *ctx);

void duk_get_prop_string(duk_context *ctx, duk_idx_t obj_idx, const char *key);
void duk_get_prop_lstring(duk_context *ctx, duk_idx_t obj_idx, const char *key, duk_size_t key_len);
void duk_get_prop_index(duk_context *ctx, duk_idx_t obj_idx, duk_uarridx_t arr_idx);
void duk_get_prop(duk_context *ctx, duk_idx_t obj_idx);
duk_bool_t duk_get_prop_string_double(duk_context *ctx, duk_idx_t obj_idx, const char *key, duk_double_t *out_val);
duk_bool_t duk_get_prop_string_bool(duk_context *ctx, duk_idx_t obj_idx, const char *key, duk_bool_t *out_val);

void duk_put_prop_string(duk_context *ctx, duk_idx_t obj_idx, const char *key);
void duk_put_prop_lstring(duk_context *ctx, duk_idx_t obj_idx, const char *key, duk_size_t key_len);
void duk_put_prop_index(duk_context *ctx, duk_idx_t obj_idx, duk_uarridx_t arr_idx);
void duk_put_prop(duk_context *ctx, duk_idx_t obj_idx);

void duk_del_prop_string(duk_context *ctx, duk_idx_t obj_idx, const char *key);
void duk_del_prop_lstring(duk_context *ctx, duk_idx_t obj_idx, const char *key, duk_size_t key_len);
void duk_del_prop_index(duk_context *ctx, duk_idx_t obj_idx, duk_uarridx_t arr_idx);
void duk_del_prop(duk_context *ctx, duk_idx_t obj_idx);

duk_bool_t duk_has_prop_string(duk_context *ctx, duk_idx_t obj_idx, const char *key);
duk_bool_t duk_has_prop_lstring(duk_context *ctx, duk_idx_t obj_idx, const char *key, duk_size_t key_len);
duk_bool_t duk_has_prop_index(duk_context *ctx, duk_idx_t obj_idx, duk_uarridx_t arr_idx);
duk_bool_t duk_has_prop(duk_context *ctx, duk_idx_t obj_idx);

void duk_get_global_string(duk_context *ctx, const char *key);
void duk_get_global_lstring(duk_context *ctx, const char *key, duk_size_t key_len);
void duk_put_global_string(duk_context *ctx, const char *key);
void duk_put_global_lstring(duk_context *ctx, const char *key, duk_size_t key_len);

duk_int_t duk_pcall(duk_context *ctx, duk_idx_t nargs);
duk_int_t duk_pcall_method(duk_context *ctx, duk_idx_t nargs);
duk_int_t duk_pcall_prop(duk_context *ctx, duk_idx_t obj_idx, duk_idx_t nargs);
duk_int_t duk_peval(duk_context *ctx);
duk_int_t duk_peval_string(duk_context *ctx, const char *str);
duk_int_t duk_peval_lstring(duk_context *ctx, const char *str, duk_size_t len);
duk_int_t duk_pnew(duk_context *ctx, duk_idx_t nargs);

void duk_call(duk_context *ctx, duk_idx_t nargs);
void duk_call_method(duk_context *ctx, duk_idx_t nargs);
void duk_call_prop(duk_context *ctx, duk_idx_t obj_idx, duk_idx_t nargs);

duk_int_t duk_safe_call(duk_context *ctx, duk_safe_call_function func, void *udata, duk_idx_t nargs, duk_idx_t nrets);

const char *duk_get_error_message(duk_context *ctx, duk_idx_t idx);
void duk_throw(duk_context *ctx);
void duk_fatal(duk_context *ctx, int err_code, const char *err_msg);

void duk_gc(duk_context *ctx, duk_uint_t flags);

void duk_debugger_attach(duk_context *ctx,
                         duk_debug_read_function read_cb,
                         duk_debug_write_function write_cb,
                         duk_debug_peek_function peek_cb,
                         duk_debug_read_flush_function read_flush_cb,
                         duk_debug_write_flush_function write_flush_cb,
                         duk_debug_detach_function detach_cb,
                         void *udata);
void duk_debugger_detach(duk_context *ctx);
void duk_debugger_cooperate(duk_context *ctx);
duk_bool_t duk_debugger_notify(duk_context *ctx, duk_idx_t nvalues);

#ifdef __cplusplus
}
#endif

#endif /* DUKTAPE_H_INCLUDED */