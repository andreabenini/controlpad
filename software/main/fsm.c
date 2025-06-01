#include "configuration.h"
#include "fsm.h"
#include "fsm.menu.h"
#include "fsm.profile.h"
#include "fsm.configuration.h"

#include "fonts.h"
#include "mainloop.h"
#include "display.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

extern keyboardStatus keyboard;

void*       statusObjects[STATUS_TYPE_ELEMENTS] = { NULL };
statusType  statusCurrent;


// FIXME: below this line
typedef struct {
    uint8_t keypressWait;       // 0:NoLock, 1:Lock, 2:PressAnyKey
} objSystem;
objSystem systemMenu;


esp_err_t statusInit() {
    if (displayInit() != ESP_OK) {
        return ESP_FAIL;
    }
    statusObjects[STATUS_MENU]          = menuCreate(statusObjects);
    statusObjects[STATUS_PROFILE]       = profileCreate(statusObjects);
    statusObjects[STATUS_CONFIGURATION] = configurationCreate(statusObjects);
    // TODO: Add here other objects creation
    // for (uint8_t i=0; i<STATUS_TYPE_ELEMENTS; i++) {
    //     if (statusObjects[i] == NULL) {
    //         ESP_LOGE(TAG_FSM, "Cannot create object [%d]", i);
    //         return ESP_ERR_INVALID_STATE;
    //     }
    // }
    statusChange(STATUS_MENU);
    return ESP_OK;
} /**/

void statusChange(statusType status) {
    statusCurrent = status;
    for (uint8_t i=0; i<BUTTON_I2C_NUM; i++) {
        keyboard.button[i].stateCurrent =
        keyboard.button[i].stateLastRaw = false;
    }
    statusObject* ptr = (statusObject*)statusObjects[statusCurrent];
    ESP_LOGW(TAG_FSM, "Changing status to [%d]", statusCurrent);
    ptr->init(statusObjects[statusCurrent]);
} /**/


void eventButton(uint8_t button, bool status) {
    ((statusObject*)statusObjects[statusCurrent])->eventButton(statusObjects[statusCurrent], button, status);
} /**/

void eventStatus() {
    ((statusObject*)statusObjects[statusCurrent])->eventStatus(statusObjects[statusCurrent]);
} /**/


// TODO: Move it to fsm.system.c
/**
 * State: System Configuration, initialization method
 */
void statusSystemInit() {
    systemMenu.keypressWait = 0;
    statusSystemShow();
} /**/
// TODO: Move it to fsm.system.c
/**
 * System Configuration Menu
 */
void statusSystemShow() {
    displayBackground(LCD_COLOR_CYAN);
    displayTextBackground(18,  0, " CONFIGURATION ", &font8x12, LCD_COLOR_BLACK, LCD_COLOR_CYAN);
    displayTextBackground(4,   70, "X", &font8x14, LCD_COLOR_RED, LCD_COLOR_CYAN);
    displayTextBackground(20,  69, "SHOW CONFIG", &font8x12, LCD_COLOR_BLACK, LCD_COLOR_CYAN);
    displayTextBackground(4,   90, "A", &font8x14, LCD_COLOR_RED, LCD_COLOR_CYAN);
    displayTextBackground(20,  89, "I/O DIAGNOSTICS", &font8x12, LCD_COLOR_BLACK, LCD_COLOR_CYAN);
    displayTextBackground(4,  110, "B", &font8x14, LCD_COLOR_RED, LCD_COLOR_CYAN);
    displayTextBackground(20, 109, "RETURN TO MENU", &font8x12, LCD_COLOR_BLACK, LCD_COLOR_CYAN);
} /**/
// TODO: Move it to fsm.system.c
void statusSystemEvent() {
    switch (systemMenu.keypressWait) {
        case 0:
            break;
        case 1:
            return;
    }
    if (keyboard.button[B_SYSTEM_CANCEL].stateCurrent == 1) {
        statusChange(STATUS_MENU);
    } else if (keyboard.button[B_SYSTEM_CONFIG].stateCurrent == 1) {
    //     statusLoadConfiguration();   // TODO: Show configurations info (all but password)
    }
} /**/
// TODO: Move it to fsm.system.c
void statusSystemKeypress(uint8_t button, bool status) {
    if (systemMenu.keypressWait==2 && status == 1) {
        statusSystemShow();
        systemMenu.keypressWait = 0;
    }
} /**/
