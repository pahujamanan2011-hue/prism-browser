#include "image_loader.h"
#include "network.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

typedef struct {
    int width;
    int height;
    int channels;
    uint8_t* data;
    size_t data_size;
} DecodedImage;

static bool initialized = false;

/* Simple PPM decoder - in production this would use stb_image or similar */
static DecodedImage* decode_ppm(const uint8_t* data, size_t length) {
    if (!data || length < 4) return NULL;
    
    const char* header = (const char*)data;
    if (header[0] != 'P' || (header[1] != '3' && header[1] != '6')) {
        return NULL;
    }
    
    DecodedImage* img = calloc(1, sizeof(DecodedImage));
    if (!img) return NULL;
    
    const char* ptr = header + 2;
    while (*ptr && (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r')) ptr++;
    img->width = atoi(ptr);
    
    while (*ptr && (*ptr >= '0' && *ptr <= '9')) ptr++;
    while (*ptr && (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r')) ptr++;
    img->height = atoi(ptr);
    
    while (*ptr && (*ptr >= '0' && *ptr <= '9')) ptr++;
    while (*ptr && (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r')) ptr++;
    int max_val = atoi(ptr);
    while (*ptr && (*ptr >= '0' && *ptr <= '9')) ptr++;
    while (*ptr && (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r')) ptr++;
    
    img->channels = 3;
    size_t pixel_count = img->width * img->height;
    size_t data_size = pixel_count * 4;
    
    img->data = malloc(data_size);
    if (!img->data) {
        free(img);
        return NULL;
    }
    img->data_size = data_size;
    
    uint32_t* pixels = (uint32_t*)img->data;
    int idx = 0;
    
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            int r = 0, g = 0, b = 0;
            
            while (*ptr && (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r')) ptr++;
            r = atoi(ptr);
            while (*ptr && (*ptr >= '0' && *ptr <= '9')) ptr++;
            
            while (*ptr && (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r')) ptr++;
            g = atoi(ptr);
            while (*ptr && (*ptr >= '0' && *ptr <= '9')) ptr++;
            
            while (*ptr && (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r')) ptr++;
            b = atoi(ptr);
            while (*ptr && (*ptr >= '0' && *ptr <= '9')) ptr++;
            
            if (max_val != 255) {
                r = (r * 255) / max_val;
                g = (g * 255) / max_val;
                b = (b * 255) / max_val;
            }
            
            pixels[idx++] = (0xFF << 24) | (r << 16) | (g << 8) | b;
        }
    }
    
    return img;
}

bool image_loader_init(void) {
    if (initialized) return true;
    initialized = true;
    return true;
}

void image_loader_cleanup(void) {
    initialized = false;
}

ImageData* image_load_from_url(const char* url) {
    if (!url || !initialized) return NULL;
    
    HttpResponse* response = http_get(url);
    if (!response || !response->success || !response->data) {
        if (response) http_free_response(response);
        return NULL;
    }
    
    ImageData* result = image_load_from_data(response->data, response->data_size);
    
    http_free_response(response);
    return result;
}

ImageData* image_load_from_data(const uint8_t* data, size_t length) {
    if (!data || length == 0) return NULL;
    
    DecodedImage* decoded = decode_ppm(data, length);
    if (!decoded) return NULL;
    
    ImageData* img = calloc(1, sizeof(ImageData));
    if (!img) {
        free(decoded->data);
        free(decoded);
        return NULL;
    }
    
    img->pixels = (uint32_t*)decoded->data;
    img->width = decoded->width;
    img->height = decoded->height;
    img->channels = decoded->channels;
    img->size = decoded->data_size;
    
    free(decoded);
    return img;
}

ImageData* image_scale(ImageData* src, int new_width, int new_height) {
    if (!src || !src->pixels || new_width <= 0 || new_height <= 0) return NULL;
    
    ImageData* result = calloc(1, sizeof(ImageData));
    if (!result) return NULL;
    
    result->pixels = calloc(new_width * new_height, sizeof(uint32_t));
    if (!result->pixels) {
        free(result);
        return NULL;
    }
    
    result->width = new_width;
    result->height = new_height;
    result->channels = src->channels;
    result->size = new_width * new_height * sizeof(uint32_t);
    
    float x_ratio = (float)src->width / new_width;
    float y_ratio = (float)src->height / new_height;
    
    for (int y = 0; y < new_height; y++) {
        for (int x = 0; x < new_width; x++) {
            int src_x = (int)(x * x_ratio);
            int src_y = (int)(y * y_ratio);
            
            if (src_x >= src->width) src_x = src->width - 1;
            if (src_y >= src->height) src_y = src->height - 1;
            
            result->pixels[y * new_width + x] = src->pixels[src_y * src->width + src_x];
        }
    }
    
    return result;
}

ImageData* image_resize_fit(ImageData* src, int max_width, int max_height) {
    if (!src || !src->pixels) return NULL;
    
    float width_ratio = (float)max_width / src->width;
    float height_ratio = (float)max_height / src->height;
    float ratio = width_ratio < height_ratio ? width_ratio : height_ratio;
    
    if (ratio >= 1.0f) {
        ImageData* copy = malloc(sizeof(ImageData));
        if (!copy) return NULL;
        memcpy(copy, src, sizeof(ImageData));
        return copy;
    }
    
    int new_width = (int)(src->width * ratio);
    int new_height = (int)(src->height * ratio);
    
    return image_scale(src, new_width, new_height);
}

void image_free(ImageData* image) {
    if (!image) return;
    
    if (image->pixels) free(image->pixels);
    free(image);
}

uint32_t* image_get_pixels(ImageData* image) {
    if (!image) return NULL;
    return image->pixels;
}