#include "renderer.h"
#include "config.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

RenderContext* render_context_create(SDL_Renderer* renderer, int width, int height) {
    RenderContext* ctx = calloc(1, sizeof(RenderContext));
    if (!ctx) return NULL;
    
    ctx->renderer = renderer;
    ctx->width = width;
    ctx->height = height;
    ctx->texture_width = width;
    ctx->texture_height = height;
    
    ctx->frame_buffer = calloc(width * height, sizeof(uint32_t));
    if (!ctx->frame_buffer) {
        free(ctx);
        return NULL;
    }
    
    ctx->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                     SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!ctx->texture) {
        free(ctx->frame_buffer);
        free(ctx);
        return NULL;
    }
    
    return ctx;
}

void render_context_destroy(RenderContext* ctx) {
    if (!ctx) return;
    
    if (ctx->texture) SDL_DestroyTexture(ctx->texture);
    if (ctx->frame_buffer) free(ctx->frame_buffer);
    free(ctx);
}

void render_context_resize(RenderContext* ctx, int width, int height) {
    if (!ctx) return;
    
    if (ctx->texture) SDL_DestroyTexture(ctx->texture);
    if (ctx->frame_buffer) free(ctx->frame_buffer);
    
    ctx->width = width;
    ctx->height = height;
    ctx->texture_width = width;
    ctx->texture_height = height;
    
    ctx->frame_buffer = calloc(width * height, sizeof(uint32_t));
    ctx->texture = SDL_CreateTexture(ctx->renderer, SDL_PIXELFORMAT_ARGB8888,
                                     SDL_TEXTUREACCESS_STREAMING, width, height);
}

void render_text(SDL_Renderer* renderer, TTF_Font* font, const char* text,
                 int x, int y, SDL_Color* color) {
    if (!renderer || !font || !text || !color) return;
    
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, *color);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_Rect dest_rect = {x, y, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &dest_rect);
        SDL_DestroyTexture(texture);
    }
    
    SDL_FreeSurface(surface);
}

int render_wrapped_text(SDL_Renderer* renderer, TTF_Font* font, const char* text,
                        int x, int y, int max_width, int line_height,
                        SDL_Color* color, int* out_y) {
    if (!renderer || !font || !text || !color || !out_y) return x;
    
    char* text_copy = strdup(text);
    if (!text_copy) return x;
    
    char line_buffer[MAX_LINE_LENGTH] = {0};
    char* token = strtok(text_copy, " \t\n\r");
    int current_y = y;
    
    while (token != NULL) {
        char test_line[MAX_LINE_LENGTH];
        if (strlen(line_buffer) > 0) {
            snprintf(test_line, sizeof(test_line), "%s %s", line_buffer, token);
        } else {
            strncpy(test_line, token, sizeof(test_line) - 1);
            test_line[sizeof(test_line) - 1] = '\0';
        }
        
        int test_width, test_height;
        if (TTF_SizeUTF8(font, test_line, &test_width, &test_height) != 0) {
            token = strtok(NULL, " \t\n\r");
            continue;
        }
        
        if (test_width > max_width) {
            /* Current word pushes line over edge; draw accumulated line first */
            if (strlen(line_buffer) > 0) {
                render_text(renderer, font, line_buffer, x, current_y, color);
                current_y += line_height;
            }
            /* Start a fresh line with our active token */
            strncpy(line_buffer, token, sizeof(line_buffer) - 1);
            line_buffer[sizeof(line_buffer) - 1] = '\0';
        } else {
            /* Space permits; update our line buffer */
            strncpy(line_buffer, test_line, sizeof(line_buffer) - 1);
            line_buffer[sizeof(line_buffer) - 1] = '\0';
        }
        
        token = strtok(NULL, " \t\n\r");
    }
    
    /* Clear residual elements remaining in the buffer */
    if (strlen(line_buffer) > 0) {
        render_text(renderer, font, line_buffer, x, current_y, color);
        current_y += line_height;
    }
    
    *out_y = current_y;
    free(text_copy);
    return x;
}

void render_image(SDL_Renderer* renderer, uint32_t* image_data, int width, int height,
                  int x, int y) {
    if (!renderer || !image_data || width <= 0 || height <= 0) return;
    
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(image_data, width, height, 32,
                                                    width * 4,
                                                    0x00FF0000, 0x0000FF00, 0x000000FF,
                                                    0xFF000000);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_Rect dest_rect = {x, y, width, height};
        SDL_RenderCopy(renderer, texture, NULL, &dest_rect);
        SDL_DestroyTexture(texture);
    }
    
    SDL_FreeSurface(surface);
}

int measure_text_width(TTF_Font* font, const char* text) {
    if (!font || !text) return 0;
    
    int width, height;
    if (TTF_SizeUTF8(font, text, &width, &height) != 0) {
        return 0;
    }
    
    return width;
}