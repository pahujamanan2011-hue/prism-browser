#ifndef PRISM_IMAGE_LOADER_H
#define PRISM_IMAGE_LOADER_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_IMAGE_WIDTH 800
#define MAX_IMAGE_HEIGHT 600

typedef struct ImageData {
    uint32_t* pixels;
    int width;
    int height;
    int channels;
    size_t size;
} ImageData;

bool image_loader_init(void);
void image_loader_cleanup(void);
ImageData* image_load_from_url(const char* url);
ImageData* image_load_from_data(const uint8_t* data, size_t length);
ImageData* image_scale(ImageData* src, int new_width, int new_height);
ImageData* image_resize_fit(ImageData* src, int max_width, int max_height);
void image_free(ImageData* image);
uint32_t* image_get_pixels(ImageData* image);

#endif /* PRISM_IMAGE_LOADER_H */