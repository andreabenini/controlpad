#ifndef JOYSTICK__H
#define JOYSTICK__H


#include "freertos/FreeRTOS.h"                      // IWYU pragma: keep
#include <esp_err.h>
#include <esp_log.h>
#include "driver/i2c_master.h"                      // IWYU pragma: keep
#include "esp_adc/adc_continuous.h"


// Joysticks initialization
esp_err_t joystickInit(adc_channel_t (*channel)[4], adc_continuous_handle_t *adcHandle);


#endif
