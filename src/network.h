#ifndef PRISM_NETWORK_H
#define PRISM_NETWORK_H

#include <stdint.h>
#include <stdbool.h>
#include <curl/curl.h>

#define MAX_RESPONSE_DATA 10 * 1024 * 1024

typedef struct HttpResponse {
    uint8_t* data;
    size_t data_size;
    long status_code;
    char* content_type;
    char* location;
    int redirect_count;
    bool success;
    bool is_blocked;
} HttpResponse;

bool network_init(void);
void network_cleanup(void);

HttpResponse* http_get(const char* url);
HttpResponse* http_post(const char* url, const char* data, size_t data_len);
HttpResponse* http_head(const char* url);
void http_free_response(HttpResponse* response);
bool http_validate_content_type(HttpResponse* response, const char* expected_type);
char* http_url_encode(const char* url);
char* http_url_decode(const char* url);

#endif /* PRISM_NETWORK_H */