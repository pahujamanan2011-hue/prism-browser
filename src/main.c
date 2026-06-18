#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <curl/curl.h>
#include "config.h"
#include "window.h"
#include "renderer.h"
#include "network.h"
#include "blackhole.h"
#include "js_runtime.h"
#include "image_loader.h"
#include "keybinds.h"

static volatile sig_atomic_t running = 1;
static BrowserWindow* g_window = NULL;

static void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        running = 0;
    }
}

static void setup_signal_handlers(void) {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

static bool initialize_subsystems(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }
    
    if (TTF_Init() < 0) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        SDL_Quit();
        return false;
    }
    
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        fprintf(stderr, "curl_global_init failed\n");
        TTF_Quit();
        SDL_Quit();
        return false;
    }
    
    if (!network_init()) {
        fprintf(stderr, "network_init failed\n");
        curl_global_cleanup();
        TTF_Quit();
        SDL_Quit();
        return false;
    }
    
    if (!blackhole_init()) {
        fprintf(stderr, "blackhole_init failed\n");
        network_cleanup();
        curl_global_cleanup();
        TTF_Quit();
        SDL_Quit();
        return false;
    }
    
    if (!js_runtime_init()) {
        fprintf(stderr, "js_runtime_init failed\n");
        blackhole_cleanup();
        network_cleanup();
        curl_global_cleanup();
        TTF_Quit();
        SDL_Quit();
        return false;
    }
    
    if (!image_loader_init()) {
        fprintf(stderr, "image_loader_init failed\n");
        js_runtime_cleanup();
        blackhole_cleanup();
        network_cleanup();
        curl_global_cleanup();
        TTF_Quit();
        SDL_Quit();
        return false;
    }
    
    return true;
}

static void cleanup_subsystems(void) {
    if (g_window) {
        window_destroy(g_window);
        g_window = NULL;
    }
    
    image_loader_cleanup();
    js_runtime_cleanup();
    blackhole_cleanup();
    network_cleanup();
    TTF_Quit();
    SDL_Quit();
    curl_global_cleanup();
}

int main(int argc, char** argv) {
    setup_signal_handlers();
    
    if (!initialize_subsystems()) {
        return EXIT_FAILURE;
    }
    
    g_window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!g_window) {
        cleanup_subsystems();
        return EXIT_FAILURE;
    }
    
    keybinds_init();
    
    SDL_Event event;
    Uint32 last_tick = SDL_GetTicks();
    const Uint32 frame_delay = 16; /* ~60 FPS */
    
    while (running) {
        Uint32 current_tick = SDL_GetTicks();
        Uint32 delta = current_tick - last_tick;
        
        if (delta < frame_delay) {
            SDL_Delay(frame_delay - delta);
        }
        last_tick = current_tick;
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
                break;
            }
            
            if (event.type == SDL_KEYDOWN) {
                KeyAction action = keybinds_handle_event(&event, g_window);
                
                switch (action.type) {
                    case KEY_ACTION_QUIT:
                        running = 0;
                        break;
                    case KEY_ACTION_FOCUS_URL:
                        window_focus_urlbar(g_window);
                        break;
                    case KEY_ACTION_RELOAD:
                        window_reload_page(g_window);
                        break;
                    case KEY_ACTION_NAVIGATE_BACK:
                        window_navigate_back(g_window);
                        break;
                    case KEY_ACTION_NAVIGATE_FORWARD:
                        window_navigate_forward(g_window);
                        break;
                    case KEY_ACTION_SCROLL_UP:
                        window_scroll(g_window, -SCROLL_STEP);
                        break;
                    case KEY_ACTION_SCROLL_DOWN:
                        window_scroll(g_window, SCROLL_STEP);
                        break;
                    case KEY_ACTION_SCROLL_PAGE_UP:
                        window_scroll(g_window, -(g_window->height / 2));
                        break;
                    case KEY_ACTION_SCROLL_PAGE_DOWN:
                        window_scroll(g_window, g_window->height / 2);
                        break;
                    case KEY_ACTION_SCROLL_HOME:
                        window_scroll(g_window, -g_window->scroll_y);
                        break;
                    case KEY_ACTION_SCROLL_END:
                        window_scroll(g_window, g_window->content_height - g_window->height);
                        break;
                    case KEY_ACTION_URL_INPUT:
                        window_handle_url_input(g_window, action.character);
                        break;
                    default:
                        break;
                }
            }
            
            if (event.type == SDL_TEXTINPUT) {
                if (g_window->url_focused) {
                    window_handle_url_input(g_window, event.text.text[0]);
                }
            }
        }
        
        window_render(g_window);
        SDL_RenderPresent(g_window->renderer);
    }
    
    cleanup_subsystems();
    return EXIT_SUCCESS;
}