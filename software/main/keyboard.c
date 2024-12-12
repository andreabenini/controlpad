#include "keyboard.h"

// System includes
#include "freertos/FreeRTOS.h"
#include <freertos/task.h>
#include <esp_err.h>
#include <esp_log.h>


static TaskHandle_t taskHandle;


void keyboardInit() {
    // Set up the GPIO for the input button with a pull-up resistor
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_START),             // GPIO pin bit mask
        .mode         = GPIO_MODE_INPUT,                    // Set as input mode
        .pull_up_en   = GPIO_PULLUP_ENABLE,                 // Enable pull-up resistor
        .pull_down_en = GPIO_PULLDOWN_DISABLE,              // Disable pull-down resistor
        .intr_type    = GPIO_INTR_DISABLE                   // No interrupts
    };
    gpio_config(&io_conf);
} /**/

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

// Configure the ADC continuous mode parameters
void joystickInit(adc_channel_t (*channel)[4], adc_continuous_handle_t *adcHandle) {
    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
    gpio_set_direction(GPIO_NUM_1, GPIO_MODE_INPUT);
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_INPUT);
    gpio_set_direction(GPIO_NUM_3, GPIO_MODE_INPUT);
    taskHandle = xTaskGetCurrentTaskHandle();
    
    ESP_LOGI(TAG_KEYBOARD, "%d", sizeof(*channel)/sizeof(adc_channel_t));

    adcInitContinuous(*channel, sizeof(*channel)/sizeof(adc_channel_t), adcHandle);
    adc_continuous_evt_cbs_t cbs = {
        .on_conv_done = s_conv_done_cb,
    };
    ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(*adcHandle, &cbs, NULL));
    ESP_ERROR_CHECK(adc_continuous_start(*adcHandle));
    /**
     * This is to show you the way to use the ADC continuous mode driver event callback.
     * This `ulTaskNotifyTake` will block when the data processing in the task is fast.
     * However in this example, the data processing (print) is slow, so you barely block here.
     *
     * Without using this event callback (to notify this task), you can still just call
     * `adc_continuous_read()` here in a loop, with/without a certain block timeout.
     */
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    vTaskDelay(10 / portTICK_PERIOD_MS);                // 10ms initial delay
} /**/

// keyboard task function
void taskKeyboard(void *pvParameter) {
    // adc_channel_t channel[2] = {JOYSTICK1_X, JOYSTICK1_Y};   // FIXME: This one was good
    // adc_channel_t channel[2] = {JOYSTICK2_X, JOYSTICK2_Y};   // FIXME: This one was good
    adc_channel_t channel[4] = {JOYSTICK1_X, JOYSTICK1_Y, JOYSTICK2_X, JOYSTICK2_Y}; // TODO: Try to have them both connected
    adc_continuous_handle_t adcHandle = NULL;
    
    ESP_LOGI(TAG_KEYBOARD, "Keyboard Init");
    keyboardInit();
    joystickInit(&channel, &adcHandle);
    esp_err_t adcResult;                        // Continuous read result
    uint32_t  adcResultNum;                     // Continuous read returned bytes
    uint8_t   adcData[ADC_BUFFER_SIZE] = {0};   // Continuous read adc data stream
    adc_digi_output_data_t *p;                  // data stream pointer
    keyboardStatus keyboard;
    while (1) {
        keyboard.buttonStart = !gpio_get_level(BUTTON_START);    // Read the button state (0: pressed, 1: released)
        // Reading analog values from all joystick axes
        keyboard.joystick1_X = keyboard.joystick1_Y = keyboard.joystick2_X = keyboard.joystick2_Y = 0;
        adcResult = adc_continuous_read(adcHandle, adcData, ADC_BUFFER_SIZE, &adcResultNum, 0);

        // char buff[1024] = {0};
        // strcpy(buff, "");
        // for (int i=0; i<sizeof(adcData); i++) {
        //     if (adcData[i] != 0) {
        //         sprintf(buff+strlen(buff), " %hhu", adcData[i]);
        //     }
        // }
        // ESP_LOGI(TAG_KEYBOARD, "buff=%s", buff);


        if (adcResult == ESP_OK) {
            for (int i=0; i<adcResultNum; i+=SOC_ADC_DIGI_RESULT_BYTES) {
                p = (adc_digi_output_data_t*)&adcData[i];
                // Check the channel number validation, the data is invalid if the channel num exceed the maximum channel
                // #define ADC_GET_DATA(p_data) ((p_data)->type2.data)
                switch (ADC_GET_CHANNEL(p)) {
                    case 0:
                        keyboard.joystick1_X = ADC_GET_DATA(p);
                        break;
                    case 1:
                        keyboard.joystick1_Y = ADC_GET_DATA(p);
                        break;
                    // case 2:
                    //     keyboard.joystick2_X = ADC_GET_DATA(p);
                    //     break;
                    // case 3:
                    //     keyboard.joystick2_Y = ADC_GET_DATA(p);
                    //     break;
                }
            }
        } else if (adcResult == ESP_ERR_TIMEOUT) {
            // Try to read `ADC_BUFFER_SIZE` until API returns timeout, which means there's no available data
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        // ESP_LOGW(TAG_KEYBOARD, "Start: %d", keyboard.buttonStart);
        // DEBUG: This is good once done
        ESP_LOGW(TAG_KEYBOARD, "Start:%d  [X1:%d, Y1:%d, X2:%d, Y2:%d]",
                                keyboard.buttonStart,
                                keyboard.joystick1_X, keyboard.joystick1_Y,
                                keyboard.joystick2_X, keyboard.joystick2_Y);
        /**
         * Because printing is slow, so every time you call `ulTaskNotifyTake`, it will immediately return.
         * To avoid a task watchdog timeout, add a delay here. When you replace the way you process the data,
         * usually you don't need this delay (as this task will block for a while).
         */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);            // Adding it to avoid task's watchdog timeout
        vTaskDelay(10 / portTICK_PERIOD_MS);                // Delay every 10 ms, under it I've had a task watchdog timeout with current ESP_LOGW
    }
    ESP_ERROR_CHECK(adc_continuous_stop(adcHandle));
    ESP_ERROR_CHECK(adc_continuous_deinit(adcHandle));
} /**/
