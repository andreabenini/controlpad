#include "keyboard.h"

// System includes
#include "freertos/FreeRTOS.h"
#include <freertos/task.h>
#include <esp_err.h>
#include <esp_log.h>


static TaskHandle_t taskHandle;

bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data) {
    BaseType_t mustYield = pdFALSE;
    vTaskNotifyGiveFromISR(taskHandle, &mustYield);         // Notify that ADC continuous driver has done enough number of conversions
    return (mustYield == pdTRUE);
} /**/

void adcInitContinuous(adc_channel_t *channel, uint8_t channelNumber, adc_continuous_handle_t *outputHandle) {
    adc_continuous_handle_t handle = NULL;
    adc_continuous_handle_cfg_t adcConfig = {
        .max_store_buf_size = 1024,
        .conv_frame_size = ADC_BUFFER_SIZE,
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adcConfig, &handle));
    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = 20 * 1000,                        // Sampling frequency (1000 might be = 1 kHz)
        .conv_mode = ADC_CONV_MODE,
        .format = ADC_OUTPUT_TYPE
    };
    adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = {0};
    dig_cfg.pattern_num = channelNumber;
    for (int i = 0; i < channelNumber; i++) {
        adc_pattern[i].atten = ADC_ATTEN;
        adc_pattern[i].channel = channel[i] & 0x7;
        adc_pattern[i].unit = ADC_UNIT_1;
        adc_pattern[i].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;
        ESP_LOGI(TAG_KEYBOARD, "adc_pattern[%d].atten is :%"PRIx8, i, adc_pattern[i].atten);
        ESP_LOGI(TAG_KEYBOARD, "adc_pattern[%d].channel is :%"PRIx8, i, adc_pattern[i].channel);
        ESP_LOGI(TAG_KEYBOARD, "adc_pattern[%d].unit is :%"PRIx8, i, adc_pattern[i].unit);
    }
    dig_cfg.adc_pattern = adc_pattern;
    ESP_ERROR_CHECK(adc_continuous_config(handle, &dig_cfg));
    *outputHandle = handle;
} /**/


void keyboardInit() {
} /**/

// Configure the ADC continuous mode parameters
void joystickInit(adc_channel_t (*channel)[2], adc_continuous_handle_t *adcHandle) {
} /**/

// keyboard task function
void taskKeyboard(void *pvParameter) {
} /**/
