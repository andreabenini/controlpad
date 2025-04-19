#ifndef I2C__H
#define I2C__H


#include "freertos/FreeRTOS.h"                                  // IWYU pragma: keep
#include "driver/i2c_master.h"                                  // IWYU pragma: keep
#include "esp_adc/adc_continuous.h"


// I2C initialization
esp_err_t i2c_MasterInit(i2c_master_bus_handle_t *handleBus, i2c_master_dev_handle_t *handleDevice);

// Init ADC continuous mode
void adcInitContinuous(adc_channel_t *channel, uint8_t channelNumber, adc_continuous_handle_t *outputHandle);

// I2C MCP 23017 functions
esp_err_t mcp23017_Init(i2c_master_dev_handle_t handleDevice);
esp_err_t mcp23017_RegisterWrite(i2c_master_dev_handle_t handleDevice, uint8_t registerAddress, uint8_t data);
esp_err_t mcp23017_RegisterRead(i2c_master_dev_handle_t handleDevice, uint8_t registerAddress, uint8_t *data);


#endif
