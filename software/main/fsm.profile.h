#ifndef FSM_PROFILE__H
#define FSM_PROFILE__H

#include "freertos/FreeRTOS.h"                                  // IWYU pragma: keep
#include <esp_err.h>
#include <esp_log.h>
#include <stdint.h>
#include <stdbool.h>
#include "driver/uart.h"                                        // IWYU pragma: keep


#include "wifi.h"                                               // IWYU pragma: keep

#define TAG_PROFILE             "profile"


/**
 * FSM object properties
 */
void statusProfileInit();
void statusProfileInitProfile(uint8_t number);
void statusProfileInitError(char *line1, char *line2, char *line3);
void statusProfileShow();
void statusProfileEvent();
void statusProfileKeypress(uint8_t button, bool status);

#endif
