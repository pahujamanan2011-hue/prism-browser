#ifndef PRISM_WINDOW_H
#define PRISM_WINDOW_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "renderer.h"
#include "parser.h"
#include "network.h"

#define MAX_HISTORY 100

typedef struct PageHistory {
    char* urls[MAX_HISTORY];
    int count;
    int current_index;
} PageHistory;

typedef struct BrowserWindow {
    SDL_Window* window;
    SDL_Renderer* renderer;
    int width;
    int height;
    int scroll_x;
    int scroll_y;
    int content_width;
    int content_height;
    char current_url[MAX_URL_LENGTH];
    char url_buffer[MAX_URL_LENGTH];
    bool url_focused;
    bool page_loaded;
    PageHistory history;
    ParsedPage* current_page;
    RenderContext* render_ctx;
    TTF_Font* font;
    TTF_Font* bold_font;
    TTF_Font* monospace_font;
} BrowserWindow;

BrowserWindow* window_create(const char* title, int width, int height);
void window_destroy(BrowserWindow* win);
bool window_load_url(BrowserWindow* win, const char* url);
void window_render(BrowserWindow* win);
void window_scroll(BrowserWindow* win, int delta);
void window_focus_urlbar(BrowserWindow* win);
void window_reload_page(BrowserWindow* win);
void window_navigate_back(BrowserWindow* win);
void window_navigate_forward(BrowserWindow* win);
void window_handle_url_input(BrowserWindow* win, char c);
void window_update_history(BrowserWindow* win, const char* url);

#endif /* PRISM_WINDOW_H */