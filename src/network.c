#include "network.h"
#include "blackhole.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

static CURLM* multi_handle = NULL;
static bool initialized = false;

struct MemoryStruct {
    uint8_t* memory;
    size_t size;
};

static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;
    
    if (mem->size + realsize > MAX_RESPONSE_DATA) {
        return 0;
    }
    
    uint8_t* ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) return 0;
    
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    
    return realsize;
}

static size_t header_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    HttpResponse* response = (HttpResponse*)userp;
    char* header = (char*)contents;
    
    if (strncasecmp(header, "Content-Type:", 13) == 0) {
        char* start = header + 13;
        while (*start && isspace(*start)) start++;
        char* end = start;
        while (*end && *end != ';' && *end != '\r' && *end != '\n') end++;
        
        size_t len = end - start;
        if (response->content_type) free(response->content_type);
        response->content_type = malloc(len + 1);
        if (response->content_type) {
            strncpy(response->content_type, start, len);
            response->content_type[len] = '\0';
        }
    } else if (strncasecmp(header, "Location:", 9) == 0) {
        char* start = header + 9;
        while (*start && isspace(*start)) start++;
        char* end = start;
        while (*end && *end != '\r' && *end != '\n') end++;
        
        size_t len = end - start;
        if (response->location) free(response->location);
        response->location = malloc(len + 1);
        if (response->location) {
            strncpy(response->location, start, len);
            response->location[len] = '\0';
        }
    }
    
    return realsize;
}

bool network_init(void) {
    if (initialized) return true;
    
    multi_handle = curl_multi_init();
    if (!multi_handle) {
        return false;
    }
    
    initialized = true;
    return true;
}

void network_cleanup(void) {
    if (multi_handle) {
        curl_multi_cleanup(multi_handle);
        multi_handle = NULL;
    }
    initialized = false;
}

HttpResponse* http_get(const char* url) {
    if (!initialized || !url) return NULL;
    
    if (blackhole_is_blocked(url)) {
        HttpResponse* blocked_response = calloc(1, sizeof(HttpResponse));
        if (blocked_response) {
            blocked_response->is_blocked = true;
            blocked_response->success = false;
        }
        return blocked_response;
    }
    
    CURL* curl = curl_easy_init();
    if (!curl) return NULL;
    
    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    if (!chunk.memory) {
        curl_easy_cleanup(curl);
        return NULL;
    }
    chunk.memory[0] = 0;
    chunk.size = 0;
    
    HttpResponse* response = calloc(1, sizeof(HttpResponse));
    if (!response) {
        free(chunk.memory);
        curl_easy_cleanup(curl);
        return NULL;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void*)response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, MAX_REDIRECTS);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, CONNECTION_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONNECTION_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");
    
    CURLcode res = curl_easy_perform(curl);
    
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response->status_code);
        response->data = chunk.memory;
        response->data_size = chunk.size;
        response->success = true;
    } else {
        free(chunk.memory);
        response->success = false;
    }
    
    /* CORRECTED: Handle redirects without use-after-free */
    if (response->location && (response->status_code == 301 || response->status_code == 302 || 
                               response->status_code == 303 || response->status_code == 307 || 
                               response->status_code == 308)) {
        if (response->redirect_count < MAX_REDIRECTS) {
            int next_redirect_depth = response->redirect_count + 1;
            char* redirect_target = strdup(response->location);
            
            /* Clean up current response and curl handle before recursive call */
            if (response->data) free(response->data);
            if (response->content_type) free(response->content_type);
            if (response->location) free(response->location);
            free(response);
            curl_easy_cleanup(curl);
            
            if (redirect_target) {
                HttpResponse* redirected = http_get(redirect_target);
                free(redirect_target);
                if (redirected) {
                    redirected->redirect_count = next_redirect_depth;
                    return redirected;
                }
            }
            return NULL;
        }
    }
    
    curl_easy_cleanup(curl);
    return response;
}

HttpResponse* http_post(const char* url, const char* data, size_t data_len) {
    if (!initialized || !url) return NULL;
    
    CURL* curl = curl_easy_init();
    if (!curl) return NULL;
    
    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    if (!chunk.memory) {
        curl_easy_cleanup(curl);
        return NULL;
    }
    chunk.memory[0] = 0;
    chunk.size = 0;
    
    HttpResponse* response = calloc(1, sizeof(HttpResponse));
    if (!response) {
        free(chunk.memory);
        curl_easy_cleanup(curl);
        return NULL;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data_len);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, CONNECTION_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONNECTION_TIMEOUT);
    
    CURLcode res = curl_easy_perform(curl);
    
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response->status_code);
        response->data = chunk.memory;
        response->data_size = chunk.size;
        response->success = true;
    } else {
        free(chunk.memory);
        response->success = false;
    }
    
    curl_easy_cleanup(curl);
    return response;
}

HttpResponse* http_head(const char* url) {
    if (!initialized || !url) return NULL;
    
    CURL* curl = curl_easy_init();
    if (!curl) return NULL;
    
    HttpResponse* response = calloc(1, sizeof(HttpResponse));
    if (!response) {
        curl_easy_cleanup(curl);
        return NULL;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void*)response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, CONNECTION_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONNECTION_TIMEOUT);
    
    CURLcode res = curl_easy_perform(curl);
    
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response->status_code);
        response->success = true;
    } else {
        response->success = false;
    }
    
    curl_easy_cleanup(curl);
    return response;
}

void http_free_response(HttpResponse* response) {
    if (!response) return;
    
    if (response->data) free(response->data);
    if (response->content_type) free(response->content_type);
    if (response->location) free(response->location);
    free(response);
}

bool http_validate_content_type(HttpResponse* response, const char* expected_type) {
    if (!response || !response->content_type || !expected_type) return false;
    
    return strstr(response->content_type, expected_type) != NULL;
}

char* http_url_encode(const char* url) {
    if (!url) return NULL;
    
    CURL* curl = curl_easy_init();
    if (!curl) return NULL;
    
    char* encoded = curl_easy_escape(curl, url, strlen(url));
    if (encoded) {
        char* result = strdup(encoded);
        curl_free(encoded);
        curl_easy_cleanup(curl);
        return result;
    }
    
    curl_easy_cleanup(curl);
    return NULL;
}

char* http_url_decode(const char* url) {
    if (!url) return NULL;
    
    CURL* curl = curl_easy_init();
    if (!curl) return NULL;
    
    int out_len;
    char* decoded = curl_easy_unescape(curl, url, strlen(url), &out_len);
    if (decoded) {
        char* result = strdup(decoded);
        curl_free(decoded);
        curl_easy_cleanup(curl);
        return result;
    }
    
    curl_easy_cleanup(curl);
    return NULL;
}