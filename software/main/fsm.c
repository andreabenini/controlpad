#include "configuration.h"

#include "esp_err.h"
#include "esp_log_level.h"
#include "fonts.h"
#include "mainloop.h"
#include "fsm.h"
#include "display.h"
#include <stdint.h>
#include <string.h>

extern keyboardStatus keyboard;

uint8_t modeCurrent;


esp_err_t modeInit() {
    if (displayInit() != ESP_OK) {
        return ESP_FAIL;
    }
    modeChange(MODE_MENU);
    return ESP_OK;
} /**/


void modeChange(uint8_t mode) {
    modeCurrent = mode;
    switch (modeCurrent) {
        case MODE_MENU:
            modeChangeMenu();
            break;
    }
} /**/


void eventButton(uint8_t button, bool status) {
    ESP_LOGI(TAG_FSM, "Button [%d] Status: %d", button, status);
} /**/


void eventStatus() {
    switch (modeCurrent) {
        case MODE_MENU:
            break;
        case MODE_TRANSMIT:
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
} /**/


/**
 * Profile selection menu
 */
void modeChangeMenu() {
    char   connections[CONFIG_MAX][CONFIG_LEN_NAME] = {0};
    size_t connectionsNumber = 0;

    configurationList(connections, &connectionsNumber);
    ESP_LOGI(TAG_FSM, "Found %d profiles in the list", connectionsNumber);

    // DEBUG: Begin
    memcpy(connections[connectionsNumber], "Item 1", 6);
    connections[connectionsNumber][6] = '\0';
    connectionsNumber++;
    memcpy(connections[connectionsNumber], "Item 2", 6);
    connections[connectionsNumber][6] = '\0';
    connectionsNumber++;
    memcpy(connections[connectionsNumber], "Something Else", 14);
    connections[connectionsNumber][14] = '\0';
    connectionsNumber++;
    memcpy(connections[connectionsNumber], "Last Item Here :)", 17);
    connections[connectionsNumber][17] = '\0';
    connectionsNumber++;
    // DEBUG: End
    memcpy(connections[connectionsNumber], MENU_CONFIGURATION, strlen(MENU_CONFIGURATION));
    connections[connectionsNumber][strlen(MENU_CONFIGURATION)] = '\0';
    connectionsNumber++;
    ESP_LOGI(TAG_FSM, "Menu Items");
    for (uint8_t i=0; i<connectionsNumber; i++) {
        ESP_LOGI(TAG_FSM, "    - %s", connections[i]);
    }
    displayBackground(LCD_COLOR_WHITE);
    displayTextBackground(18, 0, "SELECT  PROFILE", &font8x12, LCD_COLOR_RED, LCD_COLOR_WHITE);

    char *menuList;
    uint8_t menuSelected = 0;
    menuList = (char*) connections;
    menu(menuList, connectionsNumber, CONFIG_LEN_NAME,              // MenuList, MenuItems, ItemLength
                    8,                                              // Items per page
                    menuSelected,                                   // Selected Item
                    0, 16,                                          // X,Y
                    &font8x14, LCD_COLOR_BLACK, LCD_COLOR_WHITE);

    // while (1) {
    //     ;
    // }
} /**/
