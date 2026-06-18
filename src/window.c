#include "window.h"
#include "renderer.h"
#include "parser.h"
#include "network.h"
#include "keybinds.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static bool is_url_valid(const char* url) {
    return url != NULL && strlen(url) > 0;
}

static void ensure_url_scheme(char* buffer, size_t size, const char* url) {
    if (strncmp(url, "http://", 7) != 0 && strncmp(url, "https://", 8) != 0) {
        snprintf(buffer, size, "https://%s", url);
    } else {
        strncpy(buffer, url, size - 1);
        buffer[size - 1] = '\0';
    }
}

static void render_urlbar(BrowserWindow* win) {
    SDL_Rect urlbar_rect = {
        .x = MARGIN_X,
        .y = 5,
        .w = win->width - MARGIN_X * 2,
        .h = 30
    };
    
    SDL_Color urlbar_bg = {THEME_URLBAR_BG_R, THEME_URLBAR_BG_G, THEME_URLBAR_BG_B, THEME_URLBAR_BG_A};
    SDL_SetRenderDrawColor(win->renderer, urlbar_bg.r, urlbar_bg.g, urlbar_bg.b, urlbar_bg.a);
    SDL_RenderFillRect(win->renderer, &urlbar_rect);
    
    SDL_Color border = {THEME_BORDER_R, THEME_BORDER_G, THEME_BORDER_B, THEME_BORDER_A};
    SDL_SetRenderDrawColor(win->renderer, border.r, border.g, border.b, border.a);
    SDL_RenderDrawRect(win->renderer, &urlbar_rect);
    
    const char* display_text = win->url_focused ? win->url_buffer : win->current_url;
    if (strlen(display_text) == 0) {
        display_text = "Enter URL or search query...";
    }
    
    SDL_Color text_color = {THEME_URLBAR_TEXT_R, THEME_URLBAR_TEXT_G, THEME_URLBAR_TEXT_B, THEME_URLBAR_TEXT_A};
    render_text(win->renderer, win->font, display_text, 
                MARGIN_X + 5, 8, &text_color);
    
    if (win->url_focused) {
        int cursor_x = MARGIN_X + 5;
        int cursor_y = 8;
        
        int text_w, text_h;
        char temp[2] = {0};
        for (size_t i = 0; i < strlen(win->url_buffer) && i < strlen(display_text); i++) {
            temp[0] = display_text[i];
            if (TTF_SizeUTF8(win->font, temp, &text_w, &text_h) == 0) {
                cursor_x += text_w;
            }
        }
        
        SDL_SetRenderDrawColor(win->renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(win->renderer, cursor_x, cursor_y, cursor_x, cursor_y + text_h);
    }
}

static void render_content(BrowserWindow* win) {
    if (!win->current_page || !win->current_page->elements) {
        return;
    }
    
    SDL_Color link_color = {THEME_LINK_R, THEME_LINK_G, THEME_LINK_B, THEME_LINK_A};
    SDL_Color visited_color = {THEME_VISITED_R, THEME_VISITED_G, THEME_VISITED_B, THEME_VISITED_A};
    SDL_Color text_color = {THEME_TEXT_R, THEME_TEXT_G, THEME_TEXT_B, THEME_TEXT_A};
    
    int current_y = MARGIN_Y - win->scroll_y;
    int current_x = MARGIN_X;
    int line_height = TTF_FontHeight(win->font) * LINE_HEIGHT_MULTIPLIER;
    
    for (int i = 0; i < win->current_page->element_count; i++) {
        ParsedElement* elem = &win->current_page->elements[i];
        
        if (current_y > win->height + MARGIN_Y) {
            break;
        }
        
        if (current_y + line_height < -MARGIN_Y) {
            current_y += line_height;
            continue;
        }
        
        if (elem->type == ELEMENT_TEXT) {
            current_x = render_wrapped_text(win->renderer, win->font, elem->content,
                                           MARGIN_X, current_y, win->width - MARGIN_X * 2,
                                           line_height, &text_color, &current_y);
        } else if (elem->type == ELEMENT_LINK) {
            int wrapper_current_y = current_y;
            current_x = render_wrapped_text(win->renderer, win->font, elem->content,
                                          MARGIN_X, current_y, win->width - MARGIN_X * 2,
                                          line_height, &link_color, &current_y);
            
            int text_w, text_h;
            TTF_SizeUTF8(win->font, elem->content, &text_w, &text_h);
            
            SDL_SetRenderDrawColor(win->renderer, link_color.r, link_color.g, 
                                  link_color.b, link_color.a);
            SDL_RenderDrawLine(win->renderer, MARGIN_X, current_y - 2,
                              MARGIN_X + text_w, current_y - 2);
        } else if (elem->type == ELEMENT_HEADING) {
            TTF_Font* heading_font = win->bold_font;
            int heading_size = FONT_SIZE + 4;
            
            TTF_Font* sized_font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", heading_size);
            if (sized_font) {
                render_wrapped_text(win->renderer, sized_font, elem->content,
                                  MARGIN_X, current_y, win->width - MARGIN_X * 2,
                                  line_height * 1.2, &text_color, &current_y);
                TTF_CloseFont(sized_font);
            } else {
                render_wrapped_text(win->renderer, heading_font, elem->content,
                                  MARGIN_X, current_y, win->width - MARGIN_X * 2,
                                  line_height * 1.2, &text_color, &current_y);
            }
        } else if (elem->type == ELEMENT_IMAGE) {
            render_image(win->renderer, elem->image_data, elem->image_width, 
                        elem->image_height, current_x, current_y);
            current_y += elem->image_height + 10;
        }
    }
    
    win->content_height = current_y + MARGIN_Y;
}

static void render_status_bar(BrowserWindow* win) {
    SDL_Rect status_rect = {
        .x = 0,
        .y = win->height - 20,
        .w = win->width,
        .h = 20
    };
    
    SDL_Color status_bg = {THEME_BG_R, THEME_BG_G, THEME_BG_B, THEME_BG_A};
    SDL_SetRenderDrawColor(win->renderer, status_bg.r, status_bg.g, status_bg.b, status_bg.a);
    SDL_RenderFillRect(win->renderer, &status_rect);
    
    char status_text[256];
    snprintf(status_text, sizeof(status_text), "Page: %s | History: %d/%d | Blocks: %d", 
             win->page_loaded ? "Loaded" : "Loading...",
             win->history.current_index + 1, win->history.count,
             blackhole_get_block_count());
    
    SDL_Color status_color = {THEME_TEXT_R, THEME_TEXT_G, THEME_TEXT_B, THEME_TEXT_A};
    render_text(win->renderer, win->font, status_text, 10, win->height - 17, &status_color);
}

BrowserWindow* window_create(const char* title, int width, int height) {
    BrowserWindow* win = calloc(1, sizeof(BrowserWindow));
    if (!win) return NULL;
    
    win->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   width, height, SDL_WINDOW_SHOWN);
    if (!win->window) {
        free(win);
        return NULL;
    }
    
    win->renderer = SDL_CreateRenderer(win->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!win->renderer) {
        SDL_DestroyWindow(win->window);
        free(win);
        return NULL;
    }
    
    win->width = width;
    win->height = height;
    win->scroll_y = 0;
    win->url_focused = false;
    win->page_loaded = false;
    win->history.count = 0;
    win->history.current_index = -1;
    
    win->font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", FONT_SIZE);
    win->bold_font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", FONT_SIZE);
    win->monospace_font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", FONT_SIZE - 2);
    
    if (!win->font) {
        win->font = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", FONT_SIZE);
    }
    if (!win->bold_font) {
        win->bold_font = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf", FONT_SIZE);
    }
    if (!win->monospace_font) {
        win->monospace_font = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf", FONT_SIZE - 2);
    }
    
    strcpy(win->current_url, "about:blank");
    strcpy(win->url_buffer, "");
    
    win->render_ctx = render_context_create(win->renderer, width, height);
    
    return win;
}

void window_destroy(BrowserWindow* win) {
    if (!win) return;
    
    if (win->font) TTF_CloseFont(win->font);
    if (win->bold_font) TTF_CloseFont(win->bold_font);
    if (win->monospace_font) TTF_CloseFont(win->monospace_font);
    
    if (win->current_page) {
        parser_free_page(win->current_page);
    }
    
    if (win->render_ctx) {
        render_context_destroy(win->render_ctx);
    }
    
    for (int i = 0; i < win->history.count; i++) {
        if (win->history.urls[i]) {
            free(win->history.urls[i]);
        }
    }
    
    if (win->renderer) SDL_DestroyRenderer(win->renderer);
    if (win->window) SDL_DestroyWindow(win->window);
    
    free(win);
}

bool window_load_url(BrowserWindow* win, const char* url) {
    if (!win || !url) return false;
    
    char full_url[MAX_URL_LENGTH];
    ensure_url_scheme(full_url, sizeof(full_url), url);
    
    if (blackhole_is_blocked(full_url)) {
        strcpy(win->current_url, full_url);
        win->page_loaded = false;
        return false;
    }
    
    HttpResponse* response = http_get(full_url);
    if (!response || response->status_code != 200) {
        if (response) http_free_response(response);
        return false;
    }
    
    if (win->current_page) {
        parser_free_page(win->current_page);
        win->current_page = NULL;
    }
    
    win->current_page = parser_parse_html((const char*)response->data, response->data_size);
    if (!win->current_page) {
        http_free_response(response);
        return false;
    }
    
    win->current_page->base_url = strdup(full_url);
    
    http_free_response(response);
    
    strcpy(win->current_url, full_url);
    strcpy(win->url_buffer, full_url);
    win->page_loaded = true;
    win->scroll_y = 0;
    
    if (win->history.count == 0 || strcmp(win->history.urls[win->history.current_index], full_url) != 0) {
        window_update_history(win, full_url);
    }
    
    js_runtime_execute_page(win->current_page);
    
    return true;
}

void window_render(BrowserWindow* win) {
    SDL_Color bg_color = {THEME_BG_R, THEME_BG_G, THEME_BG_B, THEME_BG_A};
    SDL_SetRenderDrawColor(win->renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    SDL_RenderClear(win->renderer);
    
    render_urlbar(win);
    render_content(win);
    render_status_bar(win);
}

void window_scroll(BrowserWindow* win, int delta) {
    if (!win) return;
    
    int max_scroll = win->content_height - win->height + MARGIN_Y;
    if (max_scroll < 0) max_scroll = 0;
    
    win->scroll_y += delta;
    if (win->scroll_y < 0) win->scroll_y = 0;
    if (win->scroll_y > max_scroll) win->scroll_y = max_scroll;
}

void window_focus_urlbar(BrowserWindow* win) {
    if (!win) return;
    win->url_focused = true;
    strcpy(win->url_buffer, win->current_url);
    SDL_StartTextInput();
}

void window_reload_page(BrowserWindow* win) {
    if (!win || !win->page_loaded) return;
    window_load_url(win, win->current_url);
}

void window_navigate_back(BrowserWindow* win) {
    if (!win || win->history.current_index <= 0) return;
    
    win->history.current_index--;
    char* url = win->history.urls[win->history.current_index];
    if (url) {
        window_load_url(win, url);
    }
}

void window_navigate_forward(BrowserWindow* win) {
    if (!win || win->history.current_index >= win->history.count - 1) return;
    
    win->history.current_index++;
    char* url = win->history.urls[win->history.current_index];
    if (url) {
        window_load_url(win, url);
    }
}

void window_handle_url_input(BrowserWindow* win, char c) {
    if (!win || !win->url_focused) return;
    
    size_t len = strlen(win->url_buffer);
    
    if (c == KEY_ENTER) {
        win->url_focused = false;
        SDL_StopTextInput();
        if (strlen(win->url_buffer) > 0) {
            window_load_url(win, win->url_buffer);
        }
        return;
    }
    
    if (c == KEY_ESCAPE) {
        win->url_focused = false;
        SDL_StopTextInput();
        return;
    }
    
    if (c == KEY_BACKSPACE && len > 0) {
        win->url_buffer[len - 1] = '\0';
        return;
    }
    
    if (len < MAX_URL_LENGTH - 1 && c >= 32 && c <= 126) {
        win->url_buffer[len] = c;
        win->url_buffer[len + 1] = '\0';
    }
}

void window_update_history(BrowserWindow* win, const char* url) {
    if (!win || !url) return;
    
    if (win->history.count >= MAX_HISTORY) {
        free(win->history.urls[0]);
        memmove(win->history.urls, win->history.urls + 1, 
                (MAX_HISTORY - 1) * sizeof(char*));
        win->history.count--;
        win->history.current_index--;
    }
    
    win->history.urls[win->history.count] = strdup(url);
    if (win->history.urls[win->history.count]) {
        win->history.count++;
        win->history.current_index = win->history.count - 1;
    }
}