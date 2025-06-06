#include "fsm.configuration.h"
#include "mainloop.h"
#include "display.h"


/**
 * (constructor) Create object profile
 */
void* configurationCreate(void* objects) {
    objectConfiguration* self = (objectConfiguration*)malloc(sizeof(objectConfiguration));
    if (self != NULL) {
        self->base.type        = STATUS_CONFIGURATION;
        self->base.init        = configurationInit;
        self->base.eventStatus = configurationEventStatus;
        self->base.eventButton = configurationEventButton;
    }
    return self;
} /**/


void configurationInit(void* object) {
    objectConfiguration* self = object;
    self->keypressWait = false;
    ESP_LOGI(TAG_FSM_CONFIGURATION, "Reloading configuration");
    // Reloading configuration window
    displayBackground(LCD_COLOR_CYAN);
    displayWindow(LCD_WIDTH/10, LCD_HEIGHT/5, LCD_WIDTH/10*8, LCD_HEIGHT/2, LCD_COLOR_BLACK, LCD_COLOR_WHITE, 2);
    displayTextBackground(26, 40, "CONFIGURATION", &font8x12, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
    displayTextBackground(26, 55, "   UPDATED   ", &font8x12, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
    displayTextBackground(20, LCD_HEIGHT-14, "PRESS ANY KEY...", &font8x12, LCD_COLOR_BLACK, LCD_COLOR_CYAN);
    self->keypressWait = true;
} /**/

void configurationEventStatus(void* object) {
} /**/

/**
 * CONFIGURATION PROFILE keypress event, return to menu when a button is pressed
 */
void configurationEventButton(void* object, uint8_t button, bool status) {
    objectConfiguration* self = object;
    if (self->keypressWait==true && status==1) {
        statusChange(STATUS_MENU);
    }
} /**/
