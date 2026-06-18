#include "keybinds.h"
#include "config.h"
#include <stdbool.h>
#include <string.h>

typedef struct KeybindState {
    bool ctrl_pressed;
    bool shift_pressed;
    bool alt_pressed;
    bool caps_lock;
    bool num_lock;
    bool scroll_lock;
} KeybindState;

static KeybindState state = {0};
static bool initialized = false;

void keybinds_init(void) {
    if (initialized) return;
    memset(&state, 0, sizeof(state));
    initialized = true;
}

void keybinds_cleanup(void) {
    initialized = false;
}

static void update_modifier_state(SDL_Event* event) {
    if (event->type != SDL_KEYDOWN && event->type != SDL_KEYUP) return;
    
    SDL_Keymod mod = SDL_GetModState();
    state.ctrl_pressed = (mod & KMOD_CTRL) != 0;
    state.shift_pressed = (mod & KMOD_SHIFT) != 0;
    state.alt_pressed = (mod & KMOD_ALT) != 0;
    
    state.caps_lock = (mod & KMOD_CAPS) != 0;
    state.num_lock = (mod & KMOD_NUM) != 0;
    state.scroll_lock = (mod & KMOD_SCROLL) != 0;
}

KeyAction keybinds_handle_event(SDL_Event* event, BrowserWindow* window) {
    KeyAction action = {KEY_ACTION_NONE, 0, false};
    
    if (!event) return action;
    
    if (event->type == SDL_KEYDOWN) {
        update_modifier_state(event);
        SDL_Keycode key = event->key.keysym.sym;
        
        /* If URL bar is focused, only handle essential navigation keys */
        if (window && window->url_focused) {
            /* Allow Ctrl+Q to quit even when URL bar is focused */
            if (state.ctrl_pressed && key == 'q') {
                action.type = KEY_ACTION_QUIT;
                action.consumed = true;
                return action;
            }
            
            /* Allow Ctrl+O to focus URL bar (or unfocus if already focused) */
            if (state.ctrl_pressed && key == 'o') {
                action.type = KEY_ACTION_FOCUS_URL;
                action.consumed = true;
                return action;
            }
            
            /* Handle URL input */
            switch (key) {
                case SDLK_RETURN:
                case SDLK_KP_ENTER:
                    action.type = KEY_ACTION_URL_INPUT;
                    action.character = KEY_ENTER;
                    action.consumed = true;
                    return action;
                    
                case SDLK_ESCAPE:
                    action.type = KEY_ACTION_URL_INPUT;
                    action.character = KEY_ESCAPE;
                    action.consumed = true;
                    return action;
                    
                case SDLK_BACKSPACE:
                    action.type = KEY_ACTION_URL_INPUT;
                    action.character = KEY_BACKSPACE;
                    action.consumed = true;
                    return action;
                    
                default:
                    if (key >= 32 && key <= 126) {
                        action.type = KEY_ACTION_URL_INPUT;
                        action.character = (char)key;
                        action.consumed = true;
                        return action;
                    }
                    break;
            }
            
            /* Don't process navigation keys when URL bar is focused */
            return action;
        }
        
        /* Ctrl+Q - Quit (always available) */
        if (state.ctrl_pressed && key == 'q') {
            action.type = KEY_ACTION_QUIT;
            action.consumed = true;
            return action;
        }
        
        /* Ctrl+O - Focus URL bar */
        if (state.ctrl_pressed && key == 'o') {
            action.type = KEY_ACTION_FOCUS_URL;
            action.consumed = true;
            return action;
        }
        
        /* Ctrl+R - Reload */
        if (state.ctrl_pressed && key == 'r') {
            action.type = KEY_ACTION_RELOAD;
            action.consumed = true;
            return action;
        }
        
        /* Ctrl+A - Select All */
        if (state.ctrl_pressed && key == 'a') {
            action.type = KEY_ACTION_SELECT_ALL;
            action.consumed = true;
            return action;
        }
        
        /* Ctrl+C - Copy */
        if (state.ctrl_pressed && key == 'c') {
            action.type = KEY_ACTION_COPY;
            action.consumed = true;
            return action;
        }
        
        /* Ctrl+V - Paste */
        if (state.ctrl_pressed && key == 'v') {
            action.type = KEY_ACTION_PASTE;
            action.consumed = true;
            return action;
        }
        
        /* Ctrl+X - Cut */
        if (state.ctrl_pressed && key == 'x') {
            action.type = KEY_ACTION_CUT;
            action.consumed = true;
            return action;
        }
        
        /* Navigation keys - only if not in URL bar */
        switch (key) {
            case SDLK_UP:
                action.type = KEY_ACTION_SCROLL_UP;
                action.consumed = true;
                return action;
                
            case SDLK_DOWN:
                action.type = KEY_ACTION_SCROLL_DOWN;
                action.consumed = true;
                return action;
                
            case SDLK_LEFT:
                if (state.alt_pressed) {
                    action.type = KEY_ACTION_NAVIGATE_BACK;
                    action.consumed = true;
                    return action;
                }
                action.type = KEY_ACTION_NONE;
                return action;
                
            case SDLK_RIGHT:
                if (state.alt_pressed) {
                    action.type = KEY_ACTION_NAVIGATE_FORWARD;
                    action.consumed = true;
                    return action;
                }
                action.type = KEY_ACTION_NONE;
                return action;
                
            case SDLK_PAGEUP:
                action.type = KEY_ACTION_SCROLL_PAGE_UP;
                action.consumed = true;
                return action;
                
            case SDLK_PAGEDOWN:
                action.type = KEY_ACTION_SCROLL_PAGE_DOWN;
                action.consumed = true;
                return action;
                
            case SDLK_HOME:
                action.type = KEY_ACTION_SCROLL_HOME;
                action.consumed = true;
                return action;
                
            case SDLK_END:
                action.type = KEY_ACTION_SCROLL_END;
                action.consumed = true;
                return action;
                
            default:
                break;
        }
    }
    
    if (event->type == SDL_TEXTINPUT) {
        if (window && window->url_focused) {
            action.type = KEY_ACTION_URL_INPUT;
            action.character = event->text.text[0];
            action.consumed = true;
            return action;
        }
    }
    
    return action;
}

bool keybinds_is_modifier_pressed(void) {
    return state.ctrl_pressed || state.shift_pressed || state.alt_pressed;
}

void keybinds_reset_state(void) {
    memset(&state, 0, sizeof(state));
}