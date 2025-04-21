#ifndef FSM__H
#define FSM__H

#include <stdint.h>
#include "freertos/FreeRTOS.h"                                  // IWYU pragma: keep
#include <esp_err.h>
#include <esp_log.h>


#define TAG_FSM             "fsm"

#define MODE_MENU           0
#define MODE_TRANSMIT       1




esp_err_t modeInit();
void modeChange(uint8_t mode);

void eventButton(uint8_t button, bool status);
void eventStatus();



#endif
