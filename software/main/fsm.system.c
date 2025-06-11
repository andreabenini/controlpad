#include "fsm.system.h"
#include "mainloop.h"

extern keyboardStatus keyboard;


/**
 * (constructor) Create object menu
 */
void* systemCreate(void* objects) {
    objectSystem* self = (objectSystem*)malloc(sizeof(objectSystem));
    if (self != NULL) {
        self->base.type        = STATUS_MENU;
        self->base.init        = systemInit;
        self->base.eventStatus = systemEventStatus;
        self->base.eventButton = systemEventButton;
        self->show             = (void*)systemShow;
    }
    return self;
} /**/

/**
 * State: System Configuration, object constructor
 */
void systemInit(void* object) {
    objectSystem* self = object;
    ESP_LOGI(TAG_SYSTEM_MENU, "System Init");
    self->keypressWait = false;
    self->show(self);
} /**/

/**
 * Display system settings menu
 */
void systemShow(objectSystem* self) {
    displayBackground(LCD_COLOR_CYAN);
    displayTextBackground(18,  0, " CONFIGURATION ", &font8x12, LCD_COLOR_BLACK, LCD_COLOR_CYAN);
    displayTextBackground(4,   40, "A", &font8x14, LCD_COLOR_RED, LCD_COLOR_CYAN);
    displayTextBackground(20,  39, "I/O DIAGNOSTICS", &font8x12, LCD_COLOR_BLACK, LCD_COLOR_CYAN);
    displayTextBackground(4,   60, "B", &font8x14, LCD_COLOR_RED, LCD_COLOR_CYAN);
    displayTextBackground(20,  59, "STICK CALIBRATION", &font8x12, LCD_COLOR_BLACK, LCD_COLOR_CYAN);
    displayTextBackground(4,  112, "START", &font8x14, LCD_COLOR_RED, LCD_COLOR_CYAN);
    displayTextBackground(70, 111, "MAIN MENU", &font8x12, LCD_COLOR_BLACK, LCD_COLOR_CYAN);
} /**/


void systemEventStatus(void* object) {
} /**/

void systemEventButton(void* object, uint8_t button, bool status) {
    if (status==true && button==B_START) {
        statusChange(STATUS_MENU);
    }
    /*
    if (self->keypressWait==true && status==1) {
    }
    if (MENU_UP == 0 || MENU_DOWN == 0 || MENU_CONFIRM == 0 || MENU_CANCEL == 0) {
        ((objectMenu*)object)->lockEvent = false;
    }
    if (systemMenu.keypressWait==2 && status == 1) {
        statusSystemShow();
        systemMenu.keypressWait = 0;
    }
*/
} /**/
