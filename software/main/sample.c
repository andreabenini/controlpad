#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_continuous.h" // Include the continuous ADC header

#define ADC_CHANNEL ADC1_CHANNEL_3 // GPIO3 corresponds to ADC1 Channel 3
#define BUFFER_SIZE 1024            // Buffer size for continuous ADC readings

static const char *TAG = "ADC_CONTINUOUS";

void app_main(void)
{
    adc_continuous_handle_t adc_handle;

    // Configuration for continuous ADC
    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = BUFFER_SIZE, // Maximum buffer size for storing ADC data
        .conv_frame_size = sizeof(uint32_t), // Size of each conversion frame
    };

    // Initialize the ADC continuous driver
    ESP_ERROR_CHECK(esp_adc_continuous_new_handle(&adc_config, &adc_handle));

    // Configure the ADC channel settings
    adc_channel_t adc_channel = ADC_CHANNEL; // GPIO3 -> ADC1 Channel 3

    // Configure the continuous ADC channel
    adc_continuous_config_t chan_config = {
        .adc_channel = adc_channel,
        .adc_atten = ADC_ATTEN_DB_11,  // Input range: 0 to 3.3V
        .adc_width = ADC_WIDTH_BIT_12, // 12-bit resolution
        .adc_unit = ADC_UNIT_1,        // ADC unit
    };

    // Start continuous ADC sampling
    ESP_ERROR_CHECK(esp_adc_continuous_config(adc_handle, &chan_config));    
    ESP_ERROR_CHECK(esp_adc_continuous_start(adc_handle));

    // Buffer for storing ADC data
    uint8_t buffer[BUFFER_SIZE];
    int read_bytes = 0;
    while (1) {
        // Read data from ADC buffer
        esp_err_t ret = esp_adc_continuous_read(adc_handle, buffer, BUFFER_SIZE, &read_bytes, 1000);
        if (ret == ESP_OK && read_bytes > 0) {
            for (int i = 0; i < read_bytes; i += sizeof(uint32_t)) {
                uint32_t raw_data = *((uint32_t *)(buffer + i)); // Extract raw ADC value
                float voltage = (raw_data / 4095.0) * 3.3; // Convert to voltage
                ESP_LOGI(TAG, "Raw: %d, Voltage: %.2f V", raw_data, voltage);
            }
        } else {
            ESP_LOGW(TAG, "ADC read timeout or error");
        }
        // Delay to reduce output rate if needed
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // Clean up
    ESP_ERROR_CHECK(esp_adc_continuous_stop(adc_handle));
    ESP_ERROR_CHECK(esp_adc_continuous_deinit(adc_handle));
}
