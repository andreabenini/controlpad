#include "keyboard.h"

// System includes
#include "freertos/FreeRTOS.h"
#include <freertos/task.h>
#include <esp_err.h>
#include <esp_log.h>


static TaskHandle_t taskHandle;
keyboardStatus keyboard = {0};


esp_err_t keyboardInit() {
    ESP_LOGI(TAG_KEYBOARD, "Keyboard Init  gpio_config()");
    // Set up the GPIO for the input button with a pull-up resistor
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_BUTTON_START),        // GPIO pin bit mask
        .mode         = GPIO_MODE_INPUT,                    // Set as input mode
        .pull_up_en   = GPIO_PULLUP_ENABLE,                 // Enable pull-up resistor
        .pull_down_en = GPIO_PULLDOWN_DISABLE,              // Disable pull-down resistor
        .intr_type    = GPIO_INTR_DISABLE                   // No interrupts
    };
    return gpio_config(&io_conf);
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
    ESP_LOGI(TAG_KEYBOARD, "        - Registering new handle");
    esp_err_t err = adc_continuous_new_handle(&adcConfig, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_KEYBOARD, "Error while creating a new handle for ADC continuous mode: %d", err);
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
    ESP_LOGI(TAG_KEYBOARD, "        - Setting continuous config mode (Channels:%d)", channelNumber);
    ESP_ERROR_CHECK(adc_continuous_config(handle, &dig_cfg));
    ESP_LOGI(TAG_KEYBOARD, "        - done");     // TODO: Remove when line before does not cause the bug anymore
    *outputHandle = handle;
} /**/

// Configure the ADC continuous mode parameters
esp_err_t joystickInit(adc_channel_t (*channel)[4], adc_continuous_handle_t *adcHandle) {
    ESP_LOGI(TAG_KEYBOARD, "Joystick ADC Init");
    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);        // J2 Y
    gpio_set_direction(GPIO_NUM_1, GPIO_MODE_INPUT);        // J2 X
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_INPUT);        // J1 Y
    gpio_set_direction(GPIO_NUM_3, GPIO_MODE_INPUT);        // J1 X
    taskHandle = xTaskGetCurrentTaskHandle();
    ESP_LOGI(TAG_KEYBOARD, "    - ADC init, continuous mode.  adcInitContinuous()");
    adcInitContinuous(*channel, sizeof(*channel)/sizeof(adc_channel_t), adcHandle);
    adc_continuous_evt_cbs_t cbs = {
        .on_conv_done = s_conv_done_cb,
    };
    ESP_LOGI(TAG_KEYBOARD, "    - register event callbacks");
    ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(*adcHandle, &cbs, NULL));
    ESP_LOGI(TAG_KEYBOARD, "    - set ADC in continuous start mode");
    esp_err_t err = adc_continuous_start(*adcHandle);
    ESP_ERROR_CHECK(err);
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(TIME_POLL_DELAY));             // set initial delay
    return err;
} /**/

// keyboard task function
// FIXME: Merge this code into taskKeyboard() once done with I2C
// void taskKeyboard1(void *pvParameter) {
//     adc_channel_t channel[4] = {JOYSTICK1_X, JOYSTICK1_Y, JOYSTICK2_X, JOYSTICK2_Y};
//     adc_continuous_handle_t adcHandle = NULL;

//     ESP_LOGI(TAG_KEYBOARD, "Keyboard Init");
//     keyboardInit();
//     joystickInit(&channel, &adcHandle);
//     esp_err_t adcResult;                                    // Continuous read result
//     uint32_t  adcResultNum;                                 // Continuous read returned bytes
//     uint8_t   adcData[ADC_BUFFER_SIZE] = {0};               // Continuous read adc data stream
//     adc_digi_output_data_t *p;                              // data stream pointer
//     keyboardStatus keyboard;
//     while (1) {
//         keyboard.buttonStart = !gpio_get_level(GPIO_BUTTON_START);    // Read the button state (0: pressed, 1: released)
//         // Reading analog values from all joystick axes
//         keyboard.joystick1_X = keyboard.joystick1_Y = keyboard.joystick2_X = keyboard.joystick2_Y = 0;
//         adcResult = adc_continuous_read(adcHandle, adcData, ADC_BUFFER_SIZE, &adcResultNum, 0);
//         if (adcResult == ESP_OK) {
//             for (int i=0; i<adcResultNum; i+=SOC_ADC_DIGI_RESULT_BYTES) {
//                 p = (adc_digi_output_data_t*)&adcData[i];
//                 // Check the channel number validation, the data is invalid if the channel num exceed the maximum channel
//                 // #define ADC_GET_DATA(p_data) ((p_data)->type2.data)
//                 switch (ADC_GET_CHANNEL(p)) {
//                     case 0:
//                         keyboard.joystick1_X = ADC_GET_DATA(p);
//                         break;
//                     case 1:
//                         keyboard.joystick1_Y = ADC_GET_DATA(p);
//                         break;
//                     case 2:
//                         keyboard.joystick2_X = ADC_GET_DATA(p);
//                         break;
//                     case 3:
//                         keyboard.joystick2_Y = ADC_GET_DATA(p);
//                         break;
//                 }
//             }
//         } else if (adcResult == ESP_ERR_TIMEOUT) {
//             // Try to read `ADC_BUFFER_SIZE` until API returns timeout, which means there's no available data
//             ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
//             vTaskDelay(100 / portTICK_PERIOD_MS);
//         }

//         // ESP_LOGW(TAG_KEYBOARD, "Start: %d", keyboard.buttonStart);
//         // DEBUG: This is good once done
//         ESP_LOGW(TAG_KEYBOARD, "Start:%d  [X1:%d, Y1:%d, X2:%d, Y2:%d]",
//                                 keyboard.buttonStart,
//                                 keyboard.joystick1_X, keyboard.joystick1_Y,
//                                 keyboard.joystick2_X, keyboard.joystick2_Y);
//         /**
//          * Because printing is slow, so every time you call `ulTaskNotifyTake`, it will immediately return.
//          * To avoid a task watchdog timeout, add a delay here. When you replace the way you process the data,
//          * usually you don't need this delay (as this task will block for a while).
//          */
//         ulTaskNotifyTake(pdTRUE, portMAX_DELAY);            // Adding it to avoid task's watchdog timeout
//         vTaskDelay(10 / portTICK_PERIOD_MS);                // Delay every 10 ms, under it I've had a task watchdog timeout with current ESP_LOGW
//     }
//     ESP_ERROR_CHECK(adc_continuous_stop(adcHandle));
//     ESP_ERROR_CHECK(adc_continuous_deinit(adcHandle));
// } /**/


/**
 * I2C bus master initialization
 */
esp_err_t i2c_MasterInit(i2c_master_bus_handle_t *handleBus, i2c_master_dev_handle_t *handleDevice) {
    ESP_LOGI(TAG_KEYBOARD, "I2C Initialization");
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
        ESP_LOGE(TAG_KEYBOARD, "I2C master bus install failed: %s", esp_err_to_name(err));
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
        ESP_LOGE(TAG_KEYBOARD, "Failed to initialize I2C master bus: %s", esp_err_to_name(err));
        return err;
    }
    ESP_LOGI(TAG_KEYBOARD, "    - Initialized successfully");
    return ESP_OK;
} /**/


/**
 * Initialize button inputs, configure GPIOA as inputs
 */
esp_err_t mcp23017_Init(i2c_master_dev_handle_t handleDevice) {
    ESP_LOGI(TAG_KEYBOARD, "I2C Configuring MCP23017...");
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
    ESP_LOGI(TAG_KEYBOARD, "    - MCP23017 configuration completed");
    return ESP_OK;
} /**/


/**
 * Write a sequence of bytes to a MCP23017 register
 */
esp_err_t mcp23017_RegisterWrite(i2c_master_dev_handle_t handleDevice, uint8_t registerAddress, uint8_t data) {
    ESP_LOGI(TAG_KEYBOARD, "    - MCP23017 write (0x%02X)", registerAddress);
    uint8_t writeBuffer[2] = {registerAddress, data};
    return i2c_master_transmit(handleDevice, writeBuffer, 2, -1);
} /**/

/**
 * Read from MCP23017 registers
 */
esp_err_t mcp23017_RegisterRead(i2c_master_dev_handle_t handleDevice, uint8_t registerAddress, uint8_t *data) {
    return i2c_master_transmit_receive(handleDevice, &registerAddress, 1, data, 1, -1);
} /**/


/**
 * buttonRead_Start - Detect [start] button keypress
 * @return (void)
 */
void buttonRead_Start() {
    bool stateRaw = !gpio_get_level(GPIO_BUTTON_START);      // Read [start] button on GPIO_BUTTON_START
    uint32_t timeCurrent = pdTICKS_TO_MS(xTaskGetTickCount());

    if (stateRaw != keyboard.button[B_START].stateLastRaw) {
        keyboard.button[B_START].stateLastRaw = stateRaw;
        keyboard.button[B_START].timeLastChange = timeCurrent;
    } else {
        if ((timeCurrent - keyboard.button[B_START].timeLastChange >= TIME_DEBOUNCE_BUTTONS) && (keyboard.button[B_START].stateCurrent != stateRaw)) {
            keyboard.button[B_START].stateCurrent = stateRaw;
            eventButton(B_START, stateRaw);
        }
    }
} /**/

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


void eventButton(uint8_t button, bool status) {
    ESP_LOGI(TAG_KEYBOARD, "Button [%d] Status: %d", button, status);
} /**/

void eventStatus() {
    ESP_LOGI(TAG_KEYBOARD, "I2C (S:%d)  "
                "A[%d%d%d%d%d%d%d%d]  "
                "B[%d%d%d%d%d%d%d%d]  "
                "[%4d,%4d] [%4d,%4d]",
                keyboard.button[16].stateCurrent,       // START button (GPIO-10)
                keyboard.button[0].stateCurrent,        // A0..7, B0..7 I2C buttons
                keyboard.button[1].stateCurrent,
                keyboard.button[2].stateCurrent,
                keyboard.button[3].stateCurrent,
                keyboard.button[4].stateCurrent,
                keyboard.button[5].stateCurrent,
                keyboard.button[6].stateCurrent,
                keyboard.button[7].stateCurrent,
                keyboard.button[8].stateCurrent,
                keyboard.button[9].stateCurrent,
                keyboard.button[10].stateCurrent,
                keyboard.button[11].stateCurrent,
                keyboard.button[12].stateCurrent,
                keyboard.button[13].stateCurrent,
                keyboard.button[14].stateCurrent,
                keyboard.button[15].stateCurrent,
                keyboard.joystick1_X, keyboard.joystick1_Y,
                keyboard.joystick2_X, keyboard.joystick2_Y
    );
} /**/


/**
 * Main task function for keyboard management
 */
void taskKeyboard(void *pvParameter) {
    ESP_LOGI(TAG_KEYBOARD, "Task created");
    esp_err_t               err;
    adc_channel_t           channel[4] = {JOYSTICK1_X, JOYSTICK1_Y, JOYSTICK2_X, JOYSTICK2_Y};
    adc_continuous_handle_t adcHandle = NULL;
    uint32_t                adcResultNum = 0;               // Continuous read returned bytes
    uint8_t                 adcData[ADC_BUFFER_SIZE] = {0}; // Continuous read adc data stream
    adc_digi_output_data_t  *p;                             // data stream pointer
    i2c_master_bus_handle_t handleBus;
    i2c_master_dev_handle_t handleDevice;
    // Keyboard I/O and ADC on axes
    err = keyboardInit();
    if (err != ESP_OK) {
        ESP_LOGE(TAG_KEYBOARD, "Failed to initialize keyboard configuration (%d)", err);
        vTaskDelete(NULL);
        return;
    }
    err = joystickInit(&channel, &adcHandle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_KEYBOARD, "Failed to initialize joystick configuration (%d)", err);
        vTaskDelete(NULL);
        return;
    }
    // I2C
    err = i2c_MasterInit(&handleBus, &handleDevice);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_KEYBOARD, "Failed to initialize I2C Master");
        vTaskDelete(NULL);
        return;
    }
    // Initialize buttons on I2C gpio extender
    err = mcp23017_Init(handleDevice);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_KEYBOARD, "Failed to initialize push buttons (0x%04X), check I2C connections, program aborted", err);
        i2c_master_bus_rm_device(handleDevice);
        i2c_del_master_bus(handleBus);
        vTaskDelete(NULL);
        return;
    }

    // [[LOOP]] // Read controller status, endless loop for the task
    uint8_t gpioA_data, gpioB_data;
    while (1) {
        // Read input from I2C, Register A
        err = mcp23017_RegisterRead(handleDevice, MCP23017_GPIOA, &gpioA_data);
        if (err != ESP_OK) {
            ESP_LOGE(TAG_KEYBOARD, "Failed to read GPIO buttons, Register A");
            // Clean up before returning
            i2c_master_bus_rm_device(handleDevice);
            i2c_del_master_bus(handleBus);
            vTaskDelete(NULL);
            return;
        }
        // Read input from I2C, Register B
        err = mcp23017_RegisterRead(handleDevice, MCP23017_GPIOB, &gpioB_data);
        if (err != ESP_OK) {
            ESP_LOGE(TAG_KEYBOARD, "Failed to read GPIO buttons, Register B");
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
                // #define ADC_GET_DATA(p_data) ((p_data)->type2.data)
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
            ESP_LOGW(TAG_KEYBOARD, "ADC Continuous read timeout");
        }

        // Read button functions and trigger events
        buttonRead_Start();                             // Read [start] button and raise events, if any
        buttonRead(gpioA_data, gpioB_data);             // Read buttons and raise events, if any
        eventStatus();                                  // Raise event to report overall system state
        /**
         * Because printing is slow every time `ulTaskNotifyTake` is called it will immediately return.
         * To avoid a task watchdog timeout, adding a delay here. When you replace the way you process the data,
         * usually you don't need this delay (as this task will block for a while).
         */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);        // Adding it to avoid task's watchdog timeout (see note above)
        vTaskDelay(pdMS_TO_TICKS(TIME_POLL_DELAY));     // POLL_DELAY between reads (as .h define)
    }
    // Clean up
    ESP_ERROR_CHECK(adc_continuous_stop(adcHandle));    // TODO: Add these twos to every exit condition or rearrange exit cleanly
    ESP_ERROR_CHECK(adc_continuous_deinit(adcHandle));

    i2c_master_bus_rm_device(handleDevice);
    i2c_del_master_bus(handleBus);
    ESP_LOGI(TAG_KEYBOARD, "I2C bus closed");
    ESP_LOGI(TAG_KEYBOARD, "Task closed");
    vTaskDelete(NULL);                                  // Delete the task, without this I'm getting a guru meditation error with core0 in panic mode
} /**/
