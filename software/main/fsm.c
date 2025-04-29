#include "configuration.h"
#include "fsm.h"
#include "fsm.profile.h"
#include "fonts.h"
#include "mainloop.h"
#include "display.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

extern keyboardStatus keyboard;

uint8_t statusCurrent;

typedef struct {
    char    connections[CONFIG_MAX][CONFIG_LEN_NAME];
    uint8_t connectionsNumber;
    int8_t  first,
            page,
            selected;
    bool    lockEvent;
} objMenu;
objMenu statusMenu;

typedef struct {
    uint8_t keypressWait;       // 0:NoLock, 1:Lock, 2:PressAnyKey
} objSystem;
objSystem systemMenu;

typedef struct {
    uint8_t keypressWait;       // 0:NoLock, 1:Lock
} objReloadConfig;
objReloadConfig reloadConfig;


esp_err_t statusInit() {
    if (displayInit() != ESP_OK) {
        return ESP_FAIL;
    }
    statusChange(STATUS_MENU);
    return ESP_OK;
} /**/
void statusChange(uint8_t status) {
    statusCurrent = status;
    for (uint8_t i=0; i<BUTTON_I2C_NUM; i++) {
        keyboard.button[i].stateCurrent =
        keyboard.button[i].stateLastRaw = false;
    }
    switch (statusCurrent) {
        case STATUS_MENU:
            statusMenuInit();
            break;
        case STATUS_SYSTEM:
            statusSystemInit();
            break;
        case STATUS_RELOADCONFIG:
            statusReloadConfigInit();
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
        case STATUS_RELOADCONFIG:
            statusReloadConfigKeypress(button, status);
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
        case STATUS_RELOADCONFIG:
            statusReloadConfigEvent();
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
    menu((char*) statusMenu.connections,                    // MenuList
         statusMenu.connectionsNumber,                      // MenuItems,
         CONFIG_LEN_NAME,                                   // ItemLength
         statusMenu.page,                                   // Items per page
         statusMenu.first,                                  // First visible item in the menu
         statusMenu.selected,                               // Selected Item
         0, 16,                                             // X,Y
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
            statusProfileInitProfile(statusMenu.selected);
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
    systemMenu.keypressWait = 0;
    statusSystemShow();
} /**/
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
void statusSystemKeypress(uint8_t button, bool status) {
    if (systemMenu.keypressWait==2 && status == 1) {
        statusSystemShow();
        systemMenu.keypressWait = 0;
    }
} /**/


/**
 * Reload system configuration
 */
void statusReloadConfigInit() {
    reloadConfig.keypressWait = 0;
    ESP_LOGI(TAG_FSM, "Reloading configuration");
    // Waiting window
    displayBackground(LCD_COLOR_CYAN);
    displayWindow(LCD_WIDTH/10, LCD_HEIGHT/5, LCD_WIDTH/10*8, LCD_HEIGHT/2, LCD_COLOR_BLACK, LCD_COLOR_WHITE, 2);
    displayTextBackground(26, 40, "CONFIGURATION", &font8x12, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
    displayTextBackground(26, 55, "   UPDATED   ", &font8x12, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
    displayTextBackground(20, LCD_HEIGHT-14, "PRESS ANY KEY...", &font8x12, LCD_COLOR_BLACK, LCD_COLOR_CYAN);
    reloadConfig.keypressWait = 1;
} /**/
void statusReloadConfigEvent() {
}
/**
 * USER PROFILE LOADING keypress event
 */
 void statusReloadConfigKeypress(uint8_t button, bool status) {
    if (reloadConfig.keypressWait==1 && status==1) {
        statusChange(STATUS_MENU);
    }
} /**/
