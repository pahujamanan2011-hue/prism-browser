#ifndef PRISM_KEYBINDS_H
#define PRISM_KEYBINDS_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "window.h"

typedef enum KeyActionType {
    KEY_ACTION_NONE,
    KEY_ACTION_QUIT,
    KEY_ACTION_FOCUS_URL,
    KEY_ACTION_RELOAD,
    KEY_ACTION_NAVIGATE_BACK,
    KEY_ACTION_NAVIGATE_FORWARD,
    KEY_ACTION_SCROLL_UP,
    KEY_ACTION_SCROLL_DOWN,
    KEY_ACTION_SCROLL_PAGE_UP,
    KEY_ACTION_SCROLL_PAGE_DOWN,
    KEY_ACTION_SCROLL_HOME,
    KEY_ACTION_SCROLL_END,
    KEY_ACTION_URL_INPUT,
    KEY_ACTION_SELECT_ALL,
    KEY_ACTION_COPY,
    KEY_ACTION_PASTE,
    KEY_ACTION_CUT
} KeyActionType;

typedef struct KeyAction {
    KeyActionType type;
    char character;
    bool consumed;
} KeyAction;

void keybinds_init(void);
void keybinds_cleanup(void);
KeyAction keybinds_handle_event(SDL_Event* event, BrowserWindow* window);
bool keybinds_is_modifier_pressed(void);
void keybinds_reset_state(void);

#endif /* PRISM_KEYBINDS_H */