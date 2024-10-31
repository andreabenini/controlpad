#ifndef KEYBOARD__H
#define KEYBOARD__H

#include <stdint.h>
#include <driver/adc.h>
#include <driver/gpio.h>


// Define GPIO for the buttons
#define BUTTON_START    GPIO_NUM_10
// Define Analog Joysticks
#define DEFAULT_VREF    1100
#define JOYSTICK1_X     ADC1_CHANNEL_3      // GPIO3
#define JOYSTICK1_Y     ADC1_CHANNEL_2      // GPIO2
#define JOYSTICK_ATTEN  ADC_ATTEN_DB_11     // Set attenuation to 11dB (0 .. 3.3V)
#define JOYSTICK_WIDTH  ADC_WIDTH_BIT_12    // Set width to 12-bit for values between 0 and 4095


typedef struct {
    uint8_t     buttonStart;
    uint16_t    joystick1_X;
    uint16_t    joystick1_Y;
} keyboardStatus;


void taskKeyboard(void *pvParameter);


#define TAG_KEYBOARD    "keyboard"

#endif
