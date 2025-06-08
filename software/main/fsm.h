#ifndef FSM__H
#define FSM__H

#include <stdint.h>
#include "freertos/FreeRTOS.h"                                  // IWYU pragma: keep
#include <esp_err.h>
#include <esp_log.h>
#include "driver/uart.h"                                        // IWYU pragma: keep


#define TAG_FSM                 "fsm"
#define MENU_CONFIGURATION      "< CONFIGURATION >"


typedef enum {
    STATUS_MENU,
    STATUS_SYSTEM,
    STATUS_PROFILE,
    STATUS_CONFIGURATION
} statusType;
#define STATUS_TYPE_ELEMENTS    4

typedef void (*method)(void* self);
typedef void (*methodStatus)(void* self, uint8_t button, bool status);

typedef struct {
    statusType      type;
    method          init;
    method          eventStatus;
    methodStatus    eventButton;
} statusObject;


esp_err_t statusInit();
void      statusChange(statusType mode);

void      eventButton(uint8_t button, bool status);
void      eventStatus();

#endif
