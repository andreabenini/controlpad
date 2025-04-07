#ifndef KEYBOARD__H
#define KEYBOARD__H

#include <stdint.h>
#include <driver/gpio.h>
#include "driver/i2c_master.h"
#include "esp_adc/adc_continuous.h"


// Define GPIO for the buttons
#define GPIO_BUTTON_START       GPIO_NUM_10         // GPIO 10  (last gpio left, everything else on I2C external bus)
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
#define B_CUSTOM            15                      // Unassigned custom buttom
#define B_START             BUTTON_I2C_NUM          // Middle start button

// button struct to handle debounce state for each button
typedef struct {
    bool            stateCurrent,                   // Current debounced state
                    stateLastRaw;                   // Last raw state read
    uint32_t        timeLastChange;                 // Time of last state change
} button_state_t;
// Keyboard Status structure
typedef struct {
    button_state_t  button[BUTTON_I2C_NUM+1];       // I2C Buttons + Start button (physically handled from the board)
    uint16_t        joystick1_X,                    // Left joystick
                    joystick1_Y,
                    joystick2_X,                    // Right joystick
                    joystick2_Y;
} keyboardStatus;



// Main tasks for the keyboard management
void taskKeyboard(void *pvParameter);
void buttonRead(uint8_t gpioA_data, uint8_t gpioB_data);
void buttonRead_Start();
void eventButton(uint8_t button, bool status);
void eventStatus();

// Other hw functions
esp_err_t keyboardInit();
esp_err_t joystickInit(adc_channel_t (*channel)[4], adc_continuous_handle_t *adcHandle);

// I2C MCP 23017 functions
esp_err_t mcp23017_Init(i2c_master_dev_handle_t handleDevice);
esp_err_t mcp23017_RegisterWrite(i2c_master_dev_handle_t handleDevice, uint8_t registerAddress, uint8_t data);
esp_err_t mcp23017_RegisterRead(i2c_master_dev_handle_t handleDevice, uint8_t registerAddress, uint8_t *data);


#define TAG_KEYBOARD    "keyboard"

#endif
