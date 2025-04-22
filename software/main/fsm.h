#ifndef FSM__H
#define FSM__H

#include <stdint.h>
#include "freertos/FreeRTOS.h"                                  // IWYU pragma: keep
#include <esp_err.h>
#include <esp_log.h>


#define TAG_FSM                 "fsm"

#define STATUS_MENU             0
#define STATUS_SYSTEM           1
#define STATUS_PROFILE          2

#define MENU_UP                 keyboard.button[B_MENU_UP].stateCurrent
#define MENU_DOWN               keyboard.button[B_MENU_DOWN].stateCurrent
#define MENU_CONFIRM            keyboard.button[B_MENU_CONFIRM].stateCurrent
#define MENU_CANCEL             keyboard.button[B_MENU_CANCEL].stateCurrent

#define MENU_CONFIGURATION  "< CONFIGURATION >"


esp_err_t statusInit();
void      statusChange(uint8_t mode);

/**
 * FSM object properties
 */
// Main Menu (Profile Selection)
void statusMenuInit();
void statusMenuShow();
void statusMenuEvent();
void statusMenuKeypress(uint8_t button, bool status);
// Configuration Menu (System Setup)
void statusSystemInit();
void statusSystemShow();
void statusSystemEvent();
void statusSystemKeypress(uint8_t button, bool status);
// User Profile (Transmit mode)
void statusProfileInit();
void statusProfileShow();
void statusProfileEvent();
void statusProfileKeypress(uint8_t button, bool status);

void eventButton(uint8_t button, bool status);
void eventStatus();

#endif
