#ifndef FSM_CONFIGURATION__H
#define FSM_CONFIGURATION__H

#include "freertos/FreeRTOS.h"                                  // IWYU pragma: keep
#include <esp_err.h>
#include <esp_log.h>
#include <stdint.h>
#include <stdbool.h>
#include "driver/uart.h"                                        // IWYU pragma: keep

// Project includes
#include "wifi.h"                                               // IWYU pragma: keep
#include "fsm.h"
#include "configuration.h"

#define TAG_FSM_CONFIGURATION                                   "configuration"


/**
 * FSM profile object properties
 */
typedef struct {
    statusObject base;                                          // Base object
    bool         keypressWait;                                  // 0:NoLock, 1:Lock
} objectConfiguration;


// Object base methods
void* configurationCreate(void* objects);                       // Constructor
void  configurationInit(void* object);                          // init on statusChange()
void  configurationEventStatus(void* object);
void  configurationEventButton(void* object, uint8_t button, bool status);

// Object methods

#endif
