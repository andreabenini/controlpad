#include "configuration.h"
#include "fsm.h"
#include "fsm.menu.h"
#include "fsm.system.h"
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


esp_err_t statusInit() {
    if (displayInit() != ESP_OK) {
        return ESP_FAIL;
    }
    statusObjects[STATUS_MENU]          = menuCreate(statusObjects);
    statusObjects[STATUS_SYSTEM]        = systemCreate(statusObjects);
    statusObjects[STATUS_PROFILE]       = profileCreate(statusObjects);
    statusObjects[STATUS_CONFIGURATION] = configurationCreate(statusObjects);
    for (uint8_t i=0; i<STATUS_TYPE_ELEMENTS; i++) {
        if (statusObjects[i] == NULL) {
            ESP_LOGE(TAG_FSM, "Cannot create object [%d]", i);
            return ESP_ERR_INVALID_STATE;
        }
    }
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
