#include "esp_err.h"
#include <stdint.h>

#include "mainloop.h"
#include "fsm.h"
#include "display.h"

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
