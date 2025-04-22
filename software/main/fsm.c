#include "configuration.h"

#include "esp_err.h"
#include "esp_log.h"
#include "fonts.h"
#include "mainloop.h"
#include "fsm.h"
#include "display.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

extern keyboardStatus keyboard;

uint8_t statusCurrent;

typedef struct {
    char    connections[CONFIG_MAX][CONFIG_LEN_NAME];
    size_t  connectionsNumber;
    int8_t  first,
            page,
            selected;
    bool    lockEvent;
} objMenu;
objMenu statusMenu;

typedef struct {
} objSystem;
objSystem systemMenu;

typedef struct {
    size_t  profile;
} objProfile;
objProfile userProfile;



esp_err_t statusInit() {
    if (displayInit() != ESP_OK) {
        return ESP_FAIL;
    }
    statusChange(STATUS_MENU);
    return ESP_OK;
} /**/


void statusChange(uint8_t status) {
    statusCurrent = status;
    switch (statusCurrent) {
        case STATUS_MENU:
            statusMenuInit();
            break;
        case STATUS_SYSTEM:
            statusSystemInit();
            break;
        case STATUS_PROFILE:
            statusProfileInit();
            break;
    }
} /**/


void eventButton(uint8_t button, bool status) {
    switch (statusCurrent) {
        case STATUS_MENU:
            statusMenuKeypress(button, status);
            break;
        case STATUS_SYSTEM:
            statusSystemKeypress(button, status);
            break;
        case STATUS_PROFILE:
            statusProfileKeypress(button, status);
            break;
    }
} /**/


void eventStatus() {
    switch (statusCurrent) {
        case STATUS_MENU:
            statusMenuEvent();
            break;
        case STATUS_SYSTEM:
            statusSystemEvent();
            break;
        case STATUS_PROFILE:
            statusProfileEvent();
            break;
    }
} /**/


/**
 * statusMenu->Init() - Initialize the menu state
 */
void statusMenuInit() {
    memset(statusMenu.connections, 0x00, sizeof(statusMenu.connections));
    statusMenu.connectionsNumber = 0;
    configurationList(statusMenu.connections, &statusMenu.connectionsNumber);
    ESP_LOGI(TAG_FSM, "Found %d profiles in the list", statusMenu.connectionsNumber);
    // Adding configuration menu to the list
    memcpy(statusMenu.connections[statusMenu.connectionsNumber], MENU_CONFIGURATION, strlen(MENU_CONFIGURATION));
    statusMenu.connections[statusMenu.connectionsNumber][strlen(MENU_CONFIGURATION)] = '\0';
    statusMenu.connectionsNumber++;
    ESP_LOGI(TAG_FSM, "Menu Items");
    for (uint8_t i=0; i<statusMenu.connectionsNumber; i++) {
        ESP_LOGI(TAG_FSM, "    - %s", statusMenu.connections[i]);
    }
    // Display some context
    displayBackground(LCD_COLOR_WHITE);
    displayTextBackground(18, 0, "SELECT  PROFILE", &font8x12, LCD_COLOR_RED, LCD_COLOR_WHITE);
    statusMenu.first =
    statusMenu.selected = 0;
    statusMenu.page = 8;
    statusMenu.lockEvent = false;
    statusMenuShow();
} /**/
void statusMenuShow() {
    menu(   (char*) statusMenu.connections,                 // MenuList
            statusMenu.connectionsNumber,                   // MenuItems,
            CONFIG_LEN_NAME,                                // ItemLength
            statusMenu.page,                                // Items per page
            statusMenu.first,                               // First visible item in the menu
            statusMenu.selected,                            // Selected Item
            0, 16,                                          // X,Y
            &font8x14, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
} /**/

void statusMenuEvent() {
    if (statusMenu.lockEvent) {
        return;
    }
    if (MENU_UP == 1) {
        statusMenu.lockEvent = true;
        if (statusMenu.selected<=0) {
            statusMenu.selected = statusMenu.connectionsNumber-1;
            statusMenu.first    = statusMenu.selected-statusMenu.page < 0? 0: statusMenu.selected-statusMenu.page+1;
         } else {
            statusMenu.selected -= 1;
            statusMenu.first = statusMenu.first<=statusMenu.selected? statusMenu.first: statusMenu.first-1;
         }
        statusMenuShow();
    } else if (MENU_DOWN == 1) {
        statusMenu.lockEvent = true;
        if (statusMenu.selected>=statusMenu.connectionsNumber-1) {
            statusMenu.selected = 0;
            statusMenu.first    = 0;
        } else {
            statusMenu.selected = statusMenu.selected+1;
            statusMenu.first    = statusMenu.selected-statusMenu.page < statusMenu.first? statusMenu.first: statusMenu.first+1;
        }
        statusMenuShow();
    } else if (MENU_CONFIRM == 1) {
        if (statusMenu.selected == statusMenu.connectionsNumber-1) {
            statusChange(STATUS_SYSTEM);
        } else {
            statusChange(STATUS_PROFILE);
        }
    } else if (MENU_CANCEL == 1) {
        statusChange(STATUS_MENU);
    }
} /**/
void statusMenuKeypress(uint8_t button, bool status) {
    if (MENU_UP == 0 || MENU_DOWN == 0 || MENU_CONFIRM == 0 || MENU_CANCEL == 0) {
        statusMenu.lockEvent = false;
    }
} /**/


/**
 * State: System Configuration, initialization method
 */
void statusSystemInit() {
    // Display some context
    displayBackground(LCD_COLOR_CYAN);
    displayTextBackground(18,  0, " CONFIGURATION ", &font8x12, LCD_COLOR_BLACK, LCD_COLOR_CYAN);
    displayTextBackground(2,   50, "X", &font8x14, LCD_COLOR_RED, LCD_COLOR_CYAN);
    displayTextBackground(16,  49, "RELOAD FROM SERVER", &font8x12, LCD_COLOR_BLACK, LCD_COLOR_CYAN);
    displayTextBackground(2,   70, "Y", &font8x14, LCD_COLOR_RED, LCD_COLOR_CYAN);
    displayTextBackground(16,  69, "CONFIGURE SERVER", &font8x12, LCD_COLOR_BLACK, LCD_COLOR_CYAN);
    displayTextBackground(2,   90, "A", &font8x14, LCD_COLOR_RED, LCD_COLOR_CYAN);
    displayTextBackground(16,  89, "I/O DIAGNOSTICS", &font8x12, LCD_COLOR_BLACK, LCD_COLOR_CYAN);
    displayTextBackground(2,  110, "B", &font8x14, LCD_COLOR_RED, LCD_COLOR_CYAN);
    displayTextBackground(16, 109, "RETURN TO MENU", &font8x12, LCD_COLOR_BLACK, LCD_COLOR_CYAN);
} /**/
void statusSystemShow() {
    return;
} /**/
void statusSystemEvent() {
    if (keyboard.button[B_SYSTEM_CANCEL].stateCurrent == 1) {
        statusChange(STATUS_MENU);
    }
} /**/
void statusSystemKeypress(uint8_t button, bool status) {
    return;
} /**/



// User Profile (Transmit mode)
void statusProfileInit() {
    statusChange(STATUS_MENU);                  // TODO:
}
void statusProfileShow() {
    return;
}
void statusProfileEvent() {
    ESP_LOGI(TAG_FSM, "I2C "                    // FIXME: Convert it to a Display function
        "A[%d%d%d%d%d%d%d%d] "
        "B[%d%d%d%d%d%d%d%d]  "
        "[%4d,%4d] [%4d,%4d]",
        keyboard.button[0].stateCurrent,        // A0..7, B0..7 I2C buttons
        keyboard.button[1].stateCurrent,
        keyboard.button[2].stateCurrent,
        keyboard.button[3].stateCurrent,
        keyboard.button[4].stateCurrent,
        keyboard.button[5].stateCurrent,
        keyboard.button[6].stateCurrent,
        keyboard.button[7].stateCurrent,
        keyboard.button[8].stateCurrent,
        keyboard.button[9].stateCurrent,
        keyboard.button[10].stateCurrent,
        keyboard.button[11].stateCurrent,
        keyboard.button[12].stateCurrent,
        keyboard.button[13].stateCurrent,
        keyboard.button[14].stateCurrent,
        keyboard.button[15].stateCurrent,
        keyboard.joystick1_X, keyboard.joystick1_Y,
        keyboard.joystick2_X, keyboard.joystick2_Y
    );
}
void statusProfileKeypress(uint8_t button, bool status) {
    return;
}
