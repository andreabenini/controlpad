#ifndef FSM_SYSTEM__H
#define FSM_SYSTEM__H

#define TAG_SYSTEM_MENU         "system"

#include "fsm.h"
#include "display.h"

typedef struct {
    statusObject base;          // Base object
    method       show;          // Show system menu on display

    bool         keypressWait;  // 0:NoLock, 1:Lock
} objectSystem;


// Object base methods
void* systemCreate(void* objects);                                // Constructor
void  systemInit(void* object);                                   // init on statusChange()
void  systemEventStatus(void* object);
void  systemEventButton(void* object, uint8_t button, bool status);

// Object additional methods
void  systemShow(objectSystem* self);

#endif
