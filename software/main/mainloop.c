#include "mainloop.h"
#include "freertos/projdefs.h"
#include "i2c.h"
#include "joystick.h"
#include "fsm.h"
#include "configuration.h"



keyboardStatus keyboard = {0};
extern QueueHandle_t queueMessage;


/**
 * buttonRead - Collect button events from I2C
 * @param gpioA_data (uint8_t) GPIO-A register from MCP23017
 * @param gpioB_data (uint8_t) GPIO-B register from MCP23017
 * @return (void)
 */
void buttonRead(uint8_t gpioA_data, uint8_t gpioB_data) {
    uint8_t  i;
    bool     stateRaw;
    uint32_t timeCurrent = pdTICKS_TO_MS(xTaskGetTickCount());
    
    // Buttons are connected to ground and pull-ups are enabled,
    // Invert and mask to get pressed states, 0=pressed, 1=not pressed
    gpioA_data = ~gpioA_data;
    gpioB_data = ~gpioB_data;
    // GPIO-A (0-7) Process buttons, if needed
    for (i=0; i<8; i++) {
        stateRaw = (gpioA_data & (1<<i)) > 0;
        // If stateRaw changed from latest reading
        if (stateRaw != keyboard.button[i].stateLastRaw) {
            keyboard.button[i].stateLastRaw   = stateRaw;
            keyboard.button[i].timeLastChange = timeCurrent;
        } else {
            // If enough time has passed since the last change, update the current state
            if ((timeCurrent - keyboard.button[i].timeLastChange >= TIME_DEBOUNCE_BUTTONS) && (keyboard.button[i].stateCurrent != stateRaw)) {
                keyboard.button[i].stateCurrent = stateRaw;
                eventButton(i, stateRaw);
            }
        }
    }
    // GPIO-B (8-15) Process buttons, if needed
    for (i=0; i<8; i++) {
        stateRaw = (gpioB_data & (1<<i)) > 0;
        // If stateRaw changed from latest reading
        if (stateRaw != keyboard.button[i+8].stateLastRaw) {
            keyboard.button[i+8].stateLastRaw   = stateRaw;
            keyboard.button[i+8].timeLastChange = timeCurrent;
    
        } else {
            // If enough time has passed since the last change, update the current state
            if ((timeCurrent - keyboard.button[i+8].timeLastChange >= TIME_DEBOUNCE_BUTTONS) && (keyboard.button[i+8].stateCurrent != stateRaw)) {
                keyboard.button[i+8].stateCurrent = stateRaw;
                eventButton(i+8, stateRaw);
            }
        }
    }
} /**/


/**
 * Main task function for keyboard management
 */
void taskMainLoop(void *pvParameter) {
    ESP_LOGI(TAG_MAINLOOP, "Task created");
    esp_err_t               err;
    adc_channel_t           channel[4] = {JOYSTICK1_X, JOYSTICK1_Y, JOYSTICK2_X, JOYSTICK2_Y};
    adc_continuous_handle_t adcHandle = NULL;
    uint32_t                adcResultNum = 0;               // Continuous read returned bytes
    uint8_t                 adcData[ADC_BUFFER_SIZE] = {0}; // Continuous read adc data stream
    adc_digi_output_data_t  *p;                             // data stream pointer
    i2c_master_bus_handle_t handleBus;
    i2c_master_dev_handle_t handleDevice;
    // Init ADC on axes
    err = joystickInit(&channel, &adcHandle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MAINLOOP, "Failed to initialize joystick configuration (%d)", err);
        vTaskDelete(NULL);
        return;
    }
    // I2C
    err = i2c_MasterInit(&handleBus, &handleDevice);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MAINLOOP, "Failed to initialize I2C Master");
        vTaskDelete(NULL);
        return;
    }
    // Initialize buttons on I2C gpio extender
    err = mcp23017_Init(handleDevice);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MAINLOOP, "Failed to initialize I2C buttons (0x%04X), check I2C connections, program aborted", err);
        i2c_master_bus_rm_device(handleDevice);
        i2c_del_master_bus(handleBus);
        vTaskDelete(NULL);
        return;
    }

    // [[LOOP]] // Read controller status, endless loop for the task
    uint8_t gpioA_data, gpioB_data;
    if (configurationInit() != ESP_OK) {
        ESP_LOGE(TAG_MAINLOOP, "Failed to load configuration from the memory");
        i2c_master_bus_rm_device(handleDevice);
        i2c_del_master_bus(handleBus);
        vTaskDelete(NULL);
        return;
    }
    if (statusInit() != ESP_OK) {
        ESP_LOGE(TAG_MAINLOOP, "Failed to initialize FSM statusInit()");
        i2c_master_bus_rm_device(handleDevice);
        i2c_del_master_bus(handleBus);
        vTaskDelete(NULL);
        return;
    }
    uint32_t   receivedSignal;
    BaseType_t receivedQueue;
    while (1) {
        // Read input from I2C, Register A
        err = mcp23017_RegisterRead(handleDevice, MCP23017_GPIOA, &gpioA_data);
        if (err != ESP_OK) {
            ESP_LOGE(TAG_MAINLOOP, "Failed to read GPIO buttons, Register A");
            // Clean up before returning
            i2c_master_bus_rm_device(handleDevice);
            i2c_del_master_bus(handleBus);
            vTaskDelete(NULL);
            return;
        }
        // Read input from I2C, Register B
        err = mcp23017_RegisterRead(handleDevice, MCP23017_GPIOB, &gpioB_data);
        if (err != ESP_OK) {
            ESP_LOGE(TAG_MAINLOOP, "Failed to read GPIO buttons, Register B");
            // Clean up before returning
            i2c_master_bus_rm_device(handleDevice);
            i2c_del_master_bus(handleBus);
            vTaskDelete(NULL);
            return;
        }
        // Read analog values from all joystick axes
        err = adc_continuous_read(adcHandle, adcData, ADC_BUFFER_SIZE, &adcResultNum, 0);
        if (err == ESP_OK) {
            for (int i=0; i<adcResultNum; i+=SOC_ADC_DIGI_RESULT_BYTES) {
                p = (adc_digi_output_data_t*)&adcData[i];
                // Check the channel number validation, the data is invalid if the channel num exceed the maximum channel
                switch (ADC_GET_CHANNEL(p)) {
                    case 0:
                        keyboard.joystick1_X = ADC_GET_DATA(p);
                        break;
                    case 1:
                        keyboard.joystick1_Y = ADC_GET_DATA(p);
                        break;
                    case 2:
                        keyboard.joystick2_X = ADC_GET_DATA(p);
                        break;
                    case 3:
                        keyboard.joystick2_Y = ADC_GET_DATA(p);
                        break;
                }
            }
        } else if (err == ESP_ERR_TIMEOUT) {
            // Try to read `ADC_BUFFER_SIZE` until API returns timeout, which means there's no available data. Usually "break;"
            ESP_LOGW(TAG_MAINLOOP, "ADC Continuous read timeout");
        }

        // Read button functions and trigger events
        buttonRead(gpioA_data, gpioB_data);             // Read buttons and raise events, if any
        eventStatus();                                  // Raise event to report overall system state
        /**
         * Because printing is slow every time `ulTaskNotifyTake` is called it will immediately return.
         * To avoid a task watchdog timeout, adding a delay here. When you replace the way you process the data,
         * usually you don't need this delay (as this task will block for a while).
         */
        // ESP_LOGI(TAG_MAINLOOP, ".");    // DEBUG:
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);        // Adding it to avoid task's watchdog timeout (see note above)
        // ESP_LOGI(TAG_MAINLOOP, "+");    // DEBUG:
        // Non-blocking check for item in the queue from Task2
        receivedQueue = xQueueReceive(
                    queueMessage,                       // The queue to receive from
                    &receivedSignal,                    // Pointer to where received data will be stored
                    0                                   // Don't wait (non-blocking)
                );
        if (receivedQueue == pdTRUE) {
            statusChange(STATUS_CONFIGURATION);
        }
        vTaskDelay(pdMS_TO_TICKS(TIME_POLL_DELAY));     // POLL_DELAY between reads (as .h define)
    }
    // Clean up
    // TODO: Add these twos in every exit condition or rearrange exit cleanly
    ESP_ERROR_CHECK(adc_continuous_stop(adcHandle));
    ESP_ERROR_CHECK(adc_continuous_deinit(adcHandle));

    i2c_master_bus_rm_device(handleDevice);
    i2c_del_master_bus(handleBus);
    ESP_LOGI(TAG_MAINLOOP, "I2C bus closed");
    ESP_LOGI(TAG_MAINLOOP, "Task closed");
    vTaskDelete(NULL);                                  // Delete the task, without this I'm getting a guru meditation error with core0 in panic mode
} /**/
