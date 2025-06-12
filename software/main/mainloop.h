#ifndef MAINLOOP__H
#define MAINLOOP__H

// System includes
#include "freertos/FreeRTOS.h"                      // IWYU pragma: keep
#include <stdint.h>
#include <stdbool.h>
#include <esp_err.h>
#include <esp_log.h>
#include <driver/gpio.h>                            // IWYU pragma: keep
#include "driver/i2c_master.h"                      // IWYU pragma: keep


// Define GPIO for the buttons
#define BUTTON_I2C_MASK         0xFF                // Mask for buttons on A0-A3
#define BUTTON_I2C_NUM          16                  // Digital buttons on I2C this module is handling
#define TIME_POLL_DELAY         10                  // Delay between two reads
#define TIME_DEBOUNCE_BUTTONS   50                  // Debounce time in milliseconds, should be greater than TIME_POLL_DELAY (ie: 10, 50)
// Define Analog Joysticks
#define DEFAULT_VREF            1100
#define JOYSTICK1_X             ADC_CHANNEL_0       // GPIO0
#define JOYSTICK1_Y             ADC_CHANNEL_1       // GPIO1
#define JOYSTICK2_X             ADC_CHANNEL_2       // GPIO2
#define JOYSTICK2_Y             ADC_CHANNEL_3       // GPIO3
#define ADC_ATTEN               ADC_ATTEN_DB_12     // Set attenuation to 11dB (0<->3.3V)  [ADC_ATTEN_DB_11 is now deprecated]
#define ADC_WIDTH               SOC_ADC_DIGI_MAX_BITWIDTH   // Set width to 12-bit for values between 0 and 4095
#define ADC_BUFFER_SIZE         60                  // Buffer size for continous ADC readings (60 works)
#define ADC_CONV_MODE           ADC_CONV_SINGLE_UNIT_1

// I2C Bus setup
#define I2C_MASTER              I2C_NUM_0           // I2C master port number
#define I2C_SDA                 8                   // gpio number for I2C master data
#define I2C_SCL                 9                   // gpio number for I2C master clock
#define I2C_FREQ_HZ             400000              // I2C master clock frequency (400 KHz)
#define I2C_TIMEOUT_MS          1000
#define MCP23017_ADDR           0x20                // MCP23017 I2C address
#define MCP23017_IODIRA         0x00                // MCP23017 IODIRA register address
#define MCP23017_IODIRB         0x01                // MCP23017 IODIRB register address
#define MCP23017_GPIOA          0x12                // MCP23017 GPIOA register address
#define MCP23017_GPIOB          0x13                // MCP23017 GPIOB register address
#define MCP23017_REG_GPPUA      0x0C                // Pull-up resistor register A
#define MCP23017_REG_GPPUB      0x0D                // Pull-up resistor register B


#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
#define ADC_OUTPUT_TYPE             ADC_DIGI_OUTPUT_FORMAT_TYPE1
#define ADC_GET_CHANNEL(p_data)     ((p_data)->type1.channel)
#define ADC_GET_DATA(p_data)        ((p_data)->type1.data)
#else
#define ADC_OUTPUT_TYPE             ADC_DIGI_OUTPUT_FORMAT_TYPE2
#define ADC_GET_CHANNEL(p_data)     ((p_data)->type2.channel)
#define ADC_GET_DATA(p_data)        ((p_data)->type2.data)
#endif


#define B_A                 0                       // A,B,X,Y  Using classic PS layout
#define B_B                 1
#define B_X                 2
#define B_Y                 3
#define B_MENU              4                       // Menu button, just below the start button B_START
#define B_OPTION            5                       // Top right options button
#define B_RIGHT_TOP         6                       // R1 - Right Top button
#define B_RIGHT_BOTTOM      7                       // R2 - Right Bottom button
#define B_DPAD_LEFT         8                       // Left DPAD (left, right, up, down)
#define B_DPAD_RIGHT        9
#define B_DPAD_UP           10
#define B_DPAD_DOWN         11
#define B_SELECT            12                      // Top left select button
#define B_LEFT_TOP          13                      // L1 - Left Top button
#define B_LEFT_BOTTOM       14                      // L2 - Left Bottom button
#define B_START             15                      // Middle start button (system button)

#define B_MENU_UP           B_A                     // Navigation keys on menu selections (profile or configuration menu)
#define B_MENU_DOWN         B_B
#define B_MENU_CONFIRM      B_X
#define B_MENU_CANCEL       B_Y

#define B_SYSTEM_CONFIG     B_X                     // Hot keys valid in the system configuration menu
#define B_SYSTEM_TEST       B_B
#define B_SYSTEM_CANCEL     B_Y


// button struct to handle debounce state for each button
typedef struct {
    bool            stateCurrent,                   // Current debounced state
                    stateLastRaw;                   // Last raw state read
    uint32_t        timeLastChange;                 // Time of last state change
} button_state_t;
// Keyboard Status structure
typedef struct {
    button_state_t  button[BUTTON_I2C_NUM];         // I2C Buttons (physically handled from the board)
    uint16_t        joystick1_X,                    // Left joystick
                    joystick1_Y,
                    joystick2_X,                    // Right joystick
                    joystick2_Y;
} keyboardStatus;



// Main tasks for the keyboard management
void taskMainLoop(void *pvParameter);
void buttonRead(uint8_t gpioA_data, uint8_t gpioB_data);

#define TAG_MAINLOOP    "loop"

#endif
