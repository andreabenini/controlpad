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
RTC_NOINIT_ATTR uint8_t rebootMarker;                     // Define a marker and declare a variable in RTC memory
RTC_NOINIT_ATTR uint8_t profileNumber;                    // Optional variable, current user profile to load for fsm.profile.c


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
    rebootDetect();
    statusChange(statusCurrent);
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


/**
 * Detect latest reboot cause
 */
void rebootDetect() {
    esp_reset_reason_t reason = esp_reset_reason();
    switch (reason) {
        case ESP_RST_SW:                // This is a software reset. Now check the breadcrumb.
            if (rebootMarker > 0) {
                ESP_LOGW(TAG_FSM, "Boot reason: planned software reboot. Setting state: %d", rebootMarker);
                statusCurrent = rebootMarker;
            }
            break;
        default:                        // Another type of reboot, I don't care what it is
            ESP_LOGW(TAG_FSM, "Boot reason: %d, status: %d", reason, STATUS_MENU);
            statusCurrent = STATUS_MENU;
            profileNumber = UINT8_MAX;
            break;
    }
    rebootMarker = 0;
} /**/

/**
 * Gracefully force system reboot
 */
void rebootGraceful(uint8_t newState, uint8_t profile) {
    // Perform resources cleanup here if necessary
    rebootMarker  = newState;
    profileNumber = profile;
    ESP_LOGI(TAG_FSM, "Shutdown in progress... (State: %d->%d)", statusCurrent, rebootMarker);
    esp_restart();
} /**/
