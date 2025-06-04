#ifndef FSM_PROFILE__H
#define FSM_PROFILE__H

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

#define TAG_PROFILE             "profile"


/**
 * FSM profile object properties
 */
typedef struct {
    statusObject base;                                          // Base object
    method       displayTitle;                                  // display header bar in profile view
    method       displayStatusBar;                              // display status bar in profile view
    void         (*displayError)(void* self, char *line1, char *line2, char *line3);
    esp_err_t    (*connect)(void* self);                        // Connection to the remote network
    esp_err_t    (*disconnect)(void* self);                     // Disconnection from the remote network
    esp_err_t    (*keySend)(void* self, uint8_t button);        // Connection to the remote network

    uint8_t      profileNumber;                                 // Selected profile number from the list
    profiles     configuration;                                 // Current configuration loaded
    int          socket;                                        // TCP socket (when TCP/HTTP is used)
} objectProfile;


// Object base methods
void* profileCreate(void* objects);                             // Constructor
void  profileInit(void* object);                                // init on statusChange()
void  profileEventStatus(void* object);
void  profileEventButton(void* object, uint8_t button, bool status);

// Object methods
void  profileDisplayTitle(objectProfile* self);
void  profileDisplayStatusBar(objectProfile* self);
void  profileDisplayError(objectProfile* self, char *line1, char *line2, char *line3);
esp_err_t profileConnect(objectProfile* self);
esp_err_t profileDisconnect(objectProfile* self);
esp_err_t profileKeySend(objectProfile* self, uint8_t button);

#endif
