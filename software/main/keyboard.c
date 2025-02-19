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
    for (int i=0; i<channelNumber; i++) {
        adc_pattern[i].atten = ADC_ATTEN;
        adc_pattern[i].channel = channel[i] & 0x7;
        adc_pattern[i].unit = ADC_UNIT_1;
        adc_pattern[i].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;
    }
    dig_cfg.adc_pattern = adc_pattern;
    ESP_ERROR_CHECK(adc_continuous_config(handle, &dig_cfg));
    *outputHandle = handle;
} /**/

// Configure the ADC continuous mode parameters
void joystickInit(adc_channel_t (*channel)[4], adc_continuous_handle_t *adcHandle) {
    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);        // J2 Y
    gpio_set_direction(GPIO_NUM_1, GPIO_MODE_INPUT);        // J2 X
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_INPUT);        // J1 Y
    gpio_set_direction(GPIO_NUM_3, GPIO_MODE_INPUT);        // J1 X
    taskHandle = xTaskGetCurrentTaskHandle();
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
    vTaskDelay(10 / portTICK_PERIOD_MS);                    // 10ms initial delay
} /**/

// keyboard task function
// FIXME: Merge this code into taskKeyboard() once done with I2C
void taskKeyboard1(void *pvParameter) {
    adc_channel_t channel[4] = {JOYSTICK1_X, JOYSTICK1_Y, JOYSTICK2_X, JOYSTICK2_Y};
    adc_continuous_handle_t adcHandle = NULL;
    
    ESP_LOGI(TAG_KEYBOARD, "Keyboard Init");
    keyboardInit();
    joystickInit(&channel, &adcHandle);
    esp_err_t adcResult;                                    // Continuous read result
    uint32_t  adcResultNum;                                 // Continuous read returned bytes
    uint8_t   adcData[ADC_BUFFER_SIZE] = {0};               // Continuous read adc data stream
    adc_digi_output_data_t *p;                              // data stream pointer
    keyboardStatus keyboard;
    while (1) {
        keyboard.buttonStart = !gpio_get_level(BUTTON_START);    // Read the button state (0: pressed, 1: released)
        // Reading analog values from all joystick axes
        keyboard.joystick1_X = keyboard.joystick1_Y = keyboard.joystick2_X = keyboard.joystick2_Y = 0;
        adcResult = adc_continuous_read(adcHandle, adcData, ADC_BUFFER_SIZE, &adcResultNum, 0);
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
                    case 2:
                        keyboard.joystick2_X = ADC_GET_DATA(p);
                        break;
                    case 3:
                        keyboard.joystick2_Y = ADC_GET_DATA(p);
                        break;
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
    ESP_LOGI(TAG_KEYBOARD, "I2C Initialized successfully");
    return ESP_OK;
} /**/


/**
 * Write a sequence of bytes to a MCP23017 register
 */
esp_err_t mcp23017_RegisterWrite(i2c_master_dev_handle_t handleDevice, uint8_t registerAddress, uint8_t data) {
    ESP_LOGI(TAG_KEYBOARD, "I2C Configuring MCP23017...");
    uint8_t writeBuffer[2] = {registerAddress, data};
    return i2c_master_transmit(handleDevice, writeBuffer, 2, -1);
} /**/


// Read from MCP23017 registers
esp_err_t mcp23017_RegisterRead(i2c_master_dev_handle_t handleDevice, uint8_t registerAddress, uint8_t *data) {
    return i2c_master_transmit_receive(handleDevice, &registerAddress, 1, data, 1, -1);
} /**/


// Initialize button inputs
// Configure GPIOA as inputs
esp_err_t mcp23017_RegisterInit(i2c_master_dev_handle_t handleDevice) {
    esp_err_t err;

    // Configure GPIOA and GPIOB as inputs
    // Set pins A0-A3 as inputs (1 = input, 0 = output)
    // #define BUTTON_MASK                0x0F    // Mask for buttons on A0-A3
    // #define MCP23017_REG_IODIRA        0x00    // I/O direction register A
    err = mcp23017_RegisterWrite(handleDevice, MCP23017_IODIRA, 0x0F);          // Mask for buttons on A0-A3
    if (err != ESP_OK) return err;
    // Enable pull-up resistors on A0-A3
    // #define MCP23017_REG_GPPUA         0x0C    // Pull-up resistor register A
    err = mcp23017_RegisterWrite(handleDevice, 0x0C, 0x0F);
    if (err != ESP_OK) return err;
    return ESP_OK;
}


void taskKeyboard(void *pvParameter) {
    ESP_LOGI(TAG_KEYBOARD, "Task created");
    i2c_master_bus_handle_t handleBus;
    i2c_master_dev_handle_t handleDevice;
    esp_err_t err = i2c_MasterInit(&handleBus, &handleDevice);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_KEYBOARD, "Failed to initialize I2C Master");
        return;
    }

    // Initialize buttons
    err = mcp23017_RegisterInit(handleDevice);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_KEYBOARD, "Failed to initialize push buttons");
        i2c_master_bus_rm_device(handleDevice);
        i2c_del_master_bus(handleBus);
        return;
    }
    ESP_LOGI(TAG_KEYBOARD, "I2C MCP23017 inputs configuration completed");

    // Read GPIO Values
    // #define MCP23017_REG_GPIOA         0x12    // GPIO register A
    uint8_t gpioValue;
    for (uint16_t i=0; i<30000; i++) {
        err = mcp23017_RegisterRead(handleDevice, MCP23017_GPIOA, &gpioValue);
        if (err != ESP_OK) {
            ESP_LOGE(TAG_KEYBOARD, "Failed to read GPIOA");
            // Clean up before returning
            i2c_master_bus_rm_device(handleDevice);
            i2c_del_master_bus(handleBus);
            return;
        }
        // Since buttons are connected to ground and pull-ups are enabled,
        // a 0 bit means the button is pressed, 1 means not pressed
        // #define BUTTON_MASK                0x0F    // Mask for buttons on A0-A3
        gpioValue = (~gpioValue) & 0x0F;    // Invert and mask to get pressed states
        ESP_LOGI(TAG_KEYBOARD, "I2C [%d] GPIOA value: A0:%d A1:%d A2:%d A3:%d",
            i,
            (gpioValue & 0x01) > 0,
            (gpioValue & 0x02) > 0,
            (gpioValue & 0x04) > 0,
            (gpioValue & 0x08) > 0);
        vTaskDelay(pdMS_TO_TICKS(100));         // Read every 100ms
    }
    // Clean up
    i2c_master_bus_rm_device(handleDevice);
    i2c_del_master_bus(handleBus);
    ESP_LOGI(TAG_KEYBOARD, "I2C bus closed");
    ESP_LOGI(TAG_KEYBOARD, "Task closed");
    vTaskDelete(NULL);                                      // Delete the task, without this I'm getting a guru meditation error with core0 in panic mode
} /**/
