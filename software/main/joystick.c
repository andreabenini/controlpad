#include "mainloop.h"
#include "joystick.h"
#include "i2c.h"


static TaskHandle_t taskHandle;


bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data) {
    BaseType_t mustYield = pdFALSE;
    vTaskNotifyGiveFromISR(taskHandle, &mustYield);         // Notify that ADC continuous driver has done enough number of conversions
    return (mustYield == pdTRUE);
} /**/


// Configure the ADC continuous mode parameters
esp_err_t joystickInit(adc_channel_t (*channel)[4], adc_continuous_handle_t *adcHandle) {
    ESP_LOGI(TAG_MAINLOOP, "Joystick ADC Init");
    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);        // J2 Y
    gpio_set_direction(GPIO_NUM_1, GPIO_MODE_INPUT);        // J2 X
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_INPUT);        // J1 Y
    gpio_set_direction(GPIO_NUM_3, GPIO_MODE_INPUT);        // J1 X
    taskHandle = xTaskGetCurrentTaskHandle();
    ESP_LOGI(TAG_MAINLOOP, "    - ADC init, continuous mode.  adcInitContinuous()");
    adcInitContinuous(*channel, sizeof(*channel)/sizeof(adc_channel_t), adcHandle);
    adc_continuous_evt_cbs_t cbs = {
        .on_conv_done = s_conv_done_cb,
    };
    ESP_LOGI(TAG_MAINLOOP, "    - register event callbacks");
    ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(*adcHandle, &cbs, NULL));
    ESP_LOGI(TAG_MAINLOOP, "    - set ADC in continuous start mode");
    esp_err_t err = adc_continuous_start(*adcHandle);
    ESP_ERROR_CHECK(err);
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(TIME_POLL_DELAY));             // set initial delay
    return err;
} /**/
