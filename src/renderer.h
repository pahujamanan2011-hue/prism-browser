#ifndef PRISM_RENDERER_H
#define PRISM_RENDERER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct RenderContext {
    SDL_Renderer* renderer;
    int width;
    int height;
    uint32_t* frame_buffer;
    SDL_Texture* texture;
    int texture_width;
    int texture_height;
} RenderContext;

RenderContext* render_context_create(SDL_Renderer* renderer, int width, int height);
void render_context_destroy(RenderContext* ctx);
void render_context_resize(RenderContext* ctx, int width, int height);

void render_text(SDL_Renderer* renderer, TTF_Font* font, const char* text,
                 int x, int y, SDL_Color* color);
int render_wrapped_text(SDL_Renderer* renderer, TTF_Font* font, const char* text,
                        int x, int y, int max_width, int line_height,
                        SDL_Color* color, int* out_y);
void render_image(SDL_Renderer* renderer, uint32_t* image_data, int width, int height,
                  int x, int y);
int measure_text_width(TTF_Font* font, const char* text);

#endif /* PRISM_RENDERER_H */