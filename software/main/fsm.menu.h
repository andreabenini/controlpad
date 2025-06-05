#ifndef FSM_MENU__H
#define FSM_MENU__H

#define TAG_STATUS_MENU                 "menu"

#include "fsm.h"
#include "fsm.profile.h"
#include "display.h"
#include "configuration.h"
#include <string.h>


typedef struct {
    statusObject base;          // Base object
    method       show;          // Show menu on display

    char    connections[CONFIG_MAX][CONFIG_LEN_NAME];       // Menu list
    uint8_t connectionsNumber;  // Menu list items
    int8_t  first,              // first menu item to display
            page,               // number of elements per page
            selected;           // current menu item selected
    void*   parent;             // Parent array object reference
    bool    lockEvent;          // Process one key at the time (and block buffering keypresses too)
} objectMenu;

#define MENU_UP                 keyboard.button[B_MENU_UP].stateCurrent
#define MENU_DOWN               keyboard.button[B_MENU_DOWN].stateCurrent
#define MENU_CONFIRM            keyboard.button[B_MENU_CONFIRM].stateCurrent
#define MENU_CANCEL             keyboard.button[B_MENU_CANCEL].stateCurrent


// Object base methods
void* menuCreate(void* objects);                                // Constructor
void  menuInit(void* object);                                   // init on statusChange()
void  menuEventStatus(void* object);
void  menuEventButton(void* object, uint8_t button, bool status);

// Object additional methods
void  menuShow(objectMenu* self);

#endif
