#ifndef KEYBOARD__H
#define KEYBOARD__H

#include <stdint.h>
#include <driver/gpio.h>
#include "driver/i2c_master.h"
#include "esp_adc/adc_continuous.h"


// Define GPIO for the buttons
#define BUTTON_START            GPIO_NUM_10         // GPIO 10  (last gpio left, everything else on external bus)
// Define Analog Joysticks
#define DEFAULT_VREF            1100
#define JOYSTICK1_X             ADC_CHANNEL_0       // GPIO0
#define JOYSTICK1_Y             ADC_CHANNEL_1       // GPIO1
#define JOYSTICK2_X             ADC_CHANNEL_2       // GPIO2
#define JOYSTICK2_Y             ADC_CHANNEL_3       // GPIO3
#define ADC_ATTEN               ADC_ATTEN_DB_12     // Set attenuation to 11dB (0<->3.3V)  [ADC_ATTEN_DB_11 is now deprecated]
#define ADC_WIDTH               ADC_WIDTH_BIT_12    // Set width to 12-bit for values between 0 and 4095
#define ADC_BUFFER_SIZE         60                  // Buffer size for continous ADC readings (60 works)
#define ADC_CONV_MODE           ADC_CONV_SINGLE_UNIT_1

// I2C Bus setup
#define I2C_MASTER              I2C_NUM_0           // I2C port number for master dev
#define I2C_SDA                 8                   // gpio number for I2C master data
#define I2C_SCL                 9                   // gpio number for I2C master clock
#define I2C_FREQ_HZ             100000              // I2C master clock frequency
#define I2C_TIMEOUT_MS          1000
// #define I2C_TX_BUF_DISABLE      0                   // I2C master doesn't need buffer
// #define I2C_RX_BUF_DISABLE      0                   // I2C master doesn't need buffer
#define MCP23017_ADDR           0x20                // MCP23017 I2C address
#define MCP23017_IODIRA         0x00                // MCP23017 IODIRA register address
#define MCP23017_IODIRB         0x01                // MCP23017 IODIRB register address
#define MCP23017_GPIOA          0x12                // MCP23017 GPIOA register address
#define MCP23017_GPIOB          0x13                // MCP23017 GPIOB register address

#define WRITE_BIT  0x00  // LSB 0 for write operation
// #define READ_BIT   0x01  // LSB 1 for read operation
#define ACK_CHECK_EN 0x1


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
    bool        buttonStart,                        // Middle START button
                buttonMenu,                         // Menu button below the StartButton
                buttonSelect,                       // Top left
                buttonOptions,                      // Top right
                buttonDPadLeft,                     // Left DPAD (up, down, left right)
                buttonDPadRight,
                buttonDPadTop,
                buttonDPadDown,
                buttonA,                            // A,B,X,Y  Using classic PS layout
                buttonB,
                buttonX,
                buttonY,
                buttonLT,                           // Left Top      [digital]
                buttonRT;                           // Right Top     [digital]
    uint16_t    buttonLB,                           // Left Bottom   [analog ]
                buttonRB,                           // Right Bottom  [analog ]
                joystick1_X,                        // Left joystick
                joystick1_Y,
                joystick2_X,                        // Right joystick
                joystick2_Y;
} keyboardStatus;


void keyboardInit();
void joystickInit(adc_channel_t (*channel)[4], adc_continuous_handle_t *adcHandle);

void taskKeyboard(void *pvParameter);



#define TAG_KEYBOARD    "keyboard"

#endif
