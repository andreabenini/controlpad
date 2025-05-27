// Project includes
#include "i2c.h"
#include "mainloop.h"

// System includes
#include "freertos/FreeRTOS.h"                                  // IWYU pragma: keep
#include <esp_err.h>
#include <esp_log.h>



/**
 * I2C bus master initialization
 */
esp_err_t i2c_MasterInit(i2c_master_bus_handle_t *handleBus, i2c_master_dev_handle_t *handleDevice) {
    ESP_LOGI(TAG_MAINLOOP, "I2C Initialization");
    i2c_master_bus_config_t configuration = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port   = I2C_MASTER,
        .sda_io_num = I2C_SDA,
        .scl_io_num = I2C_SCL,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    // Install I2C master driver
    esp_err_t err = i2c_new_master_bus(&configuration, handleBus);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MAINLOOP, "I2C master bus install failed: %s", esp_err_to_name(err));
        return err;
    }
    i2c_device_config_t deviceConfig = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = MCP23017_ADDR,
        .scl_speed_hz = I2C_FREQ_HZ,
    };
    // i2c_master_dev_handle_t dev_handle;
    err = i2c_master_bus_add_device(*handleBus, &deviceConfig, handleDevice);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MAINLOOP, "Failed to initialize I2C master bus: %s", esp_err_to_name(err));
        return err;
    }
    ESP_LOGI(TAG_MAINLOOP, "    - Initialized successfully");
    return ESP_OK;
} /**/


void adcInitContinuous(adc_channel_t *channel, uint8_t channelNumber, adc_continuous_handle_t *outputHandle) {
    adc_continuous_handle_t handle = NULL;
    adc_continuous_handle_cfg_t adcConfig = {
        .max_store_buf_size = 1024,
        .conv_frame_size = ADC_BUFFER_SIZE,
    };
    ESP_LOGI(TAG_MAINLOOP, "        - Registering new handle");
    esp_err_t err = adc_continuous_new_handle(&adcConfig, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MAINLOOP, "Error while creating a new handle for ADC continuous mode: %d", err);
        *outputHandle = NULL;
        return;
    }
    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = 20 * 1000,                        // Sampling frequency (1000 might be = 1 kHz)
        .conv_mode = ADC_CONV_MODE,
        .format = ADC_OUTPUT_TYPE
    };
    adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = {0};
    dig_cfg.pattern_num = channelNumber;
    for (int i=0; i<channelNumber; i++) {
        adc_pattern[i].atten = ADC_ATTEN;
        adc_pattern[i].channel = channel[i] & 0x7;
        adc_pattern[i].unit = ADC_UNIT_1;
        adc_pattern[i].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;
    }
    dig_cfg.adc_pattern = adc_pattern;
    ESP_LOGI(TAG_MAINLOOP, "        - Setting continuous config mode (Channels:%d)", channelNumber);
    ESP_ERROR_CHECK(adc_continuous_config(handle, &dig_cfg));
    *outputHandle = handle;
} /**/



/**
 * Initialize button inputs, configure GPIOA as inputs
 */
 esp_err_t mcp23017_Init(i2c_master_dev_handle_t handleDevice) {
    ESP_LOGI(TAG_MAINLOOP, "I2C Configuring MCP23017...");
    // Configure GPIOA and GPIOB as inputs.  I/O direction (1=input, 0=output)
    esp_err_t err;
    err = mcp23017_RegisterWrite(handleDevice, MCP23017_IODIRA, BUTTON_I2C_MASK);       // Register A, mask for buttons on A0-A7
    if (err != ESP_OK) return err;
    err = mcp23017_RegisterWrite(handleDevice, MCP23017_IODIRB, BUTTON_I2C_MASK);       // Register B, mask for buttons on B0-B7
    if (err != ESP_OK) return err;
    // Enable pull-up resistors on all pins
    err = mcp23017_RegisterWrite(handleDevice, MCP23017_REG_GPPUA, BUTTON_I2C_MASK);
    if (err != ESP_OK) return err;
    err = mcp23017_RegisterWrite(handleDevice, MCP23017_REG_GPPUB, BUTTON_I2C_MASK);
    if (err != ESP_OK) return err;
    ESP_LOGI(TAG_MAINLOOP, "    - MCP23017 configuration completed");
    return ESP_OK;
} /**/

/**
 * Write a sequence of bytes to a MCP23017 register
 */
esp_err_t mcp23017_RegisterWrite(i2c_master_dev_handle_t handleDevice, uint8_t registerAddress, uint8_t data) {
    ESP_LOGI(TAG_MAINLOOP, "    - MCP23017 write (0x%02X)", registerAddress);
    uint8_t writeBuffer[2] = {registerAddress, data};
    return i2c_master_transmit(handleDevice, writeBuffer, 2, -1);
} /**/

/**
 * Read from MCP23017 registers
 */
esp_err_t mcp23017_RegisterRead(i2c_master_dev_handle_t handleDevice, uint8_t registerAddress, uint8_t *data) {
    return i2c_master_transmit_receive(handleDevice, &registerAddress, 1, data, 1, -1);
} /**/

