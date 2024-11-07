#ifndef KEYBOARD__H
#define KEYBOARD__H

#include <stdint.h>
#include <driver/gpio.h>
#include "esp_adc/adc_continuous.h"


// Define GPIO for the buttons
#define BUTTON_START    GPIO_NUM_10
// Define Analog Joysticks
#define DEFAULT_VREF            1100
#define JOYSTICK1_X             ADC_CHANNEL_0       // GPIO0
#define JOYSTICK1_Y             ADC_CHANNEL_1       // GPIO1
#define JOYSTICK2_X             ADC_CHANNEL_2       // GPIO2
#define JOYSTICK2_Y             ADC_CHANNEL_3       // GPIO3
#define ADC_ATTEN               ADC_ATTEN_DB_12     // Set attenuation to 11dB (0<->3.3V)  [ADC_ATTEN_DB_11 is now deprecated]
#define ADC_WIDTH               ADC_WIDTH_BIT_12    // Set width to 12-bit for values between 0 and 4095
#define ADC_BUFFER_SIZE         1024                // TODO:Resize   Buffer size for continous ADC readings
#define ADC_CONV_MODE           ADC_CONV_SINGLE_UNIT_1

#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
#define ADC_OUTPUT_TYPE             ADC_DIGI_OUTPUT_FORMAT_TYPE1
#define ADC_GET_CHANNEL(p_data)     ((p_data)->type1.channel)
#define ADC_GET_DATA(p_data)        ((p_data)->type1.data)
#else
#define ADC_OUTPUT_TYPE             ADC_DIGI_OUTPUT_FORMAT_TYPE2
#define ADC_GET_CHANNEL(p_data)     ((p_data)->type2.channel)
#define ADC_GET_DATA(p_data)        ((p_data)->type2.data)
#endif


typedef struct {
    uint8_t     buttonStart;
    uint16_t    joystick1_X;
    uint16_t    joystick1_Y;
    uint16_t    joystick2_X;
    uint16_t    joystick2_Y;
} keyboardStatus;


void keyboardInit();
void joystickInit(adc_channel_t (*channel)[2], adc_continuous_handle_t *adcHandle);

void taskKeyboard(void *pvParameter);



#define TAG_KEYBOARD    "keyboard"

#endif
