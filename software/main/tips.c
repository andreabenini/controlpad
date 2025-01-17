#
#
# METHOD 1
#
#


#include <stdio.h>
#include "driver/i2c.h"
#include "esp_log.h"

// I2C definitions
#define I2C_MASTER_SCL_IO           26          /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO           25          /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM              I2C_NUM_0   /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ          100000     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

// MCP23017 definitions
#define MCP23017_ADDR               0x20        /*!< I2C address of MCP23017 */
#define MCP23017_IODIRA              0x00        /*!< IODIRA register address */
#define MCP23017_IODIRB              0x01        /*!< IODIRB register address */
#define MCP23017_GPIOA              0x12        /*!< GPIOA register address */
#define MCP23017_GPIOB              0x13        /*!< GPIOB register address */

static const char *TAG = "i2c-example";

/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
        // .clk_flags = 0,          /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
    };
    esp_err_t err = i2c_param_config(i2c_master_port, &conf);
    if (err != ESP_OK) {
        return err;
    }
    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

/**
 * @brief Read a sequence of bytes from a MCP23017 register
 */
static esp_err_t mcp23017_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, MCP23017_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, MCP23017_ADDR << 1 | READ_BIT, ACK_CHECK_EN);
    if (len > 1) {
        i2c_master_read(cmd, data, len - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, data + len - 1, NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

/**
 * @brief Write a sequence of bytes to a MCP23017 register
 */
static esp_err_t mcp23017_register_write(uint8_t reg_addr, uint8_t *data, size_t len)
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, MCP23017_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
    i2c_master_write(cmd, data, len, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

void app_main(void)
{
    // Initialize I2C
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");

    // Configure GPIOA and GPIOB as inputs
    uint8_t data = 0xFF; // Set all pins as inputs
    ESP_ERROR_CHECK(mcp23017_register_write(MCP23017_IODIRA, &data, 1));
    ESP_ERROR_CHECK(mcp23017_register_write(MCP23017_IODIRB, &data, 1));
    ESP_LOGI(TAG, "MCP23017 configured as inputs");

    while (1) {
        // Read GPIOA and GPIOB
        uint8_t gpioa_data;
        uint8_t gpiob_data;
        ESP_ERROR_CHECK(mcp23017_register_read(MCP23017_GPIOA, &gpioa_data, 1));
        ESP_ERROR_CHECK(mcp23017_register_read(MCP23017_GPIOB, &gpiob_data, 1));

        // Print the values
        ESP_LOGI(TAG, "GPIOA: 0x%02x", gpioa_data);
        ESP_LOGI(TAG, "GPIOB: 0x%02x", gpiob_data);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}





// VERSION 2 with a Task
#include <stdio.h>
#include "driver/i2c.h"
#include "esp_log.h"

// ... (I2C and MCP23017 definitions as before) ...

static const char *TAG = "i2c-example";

// ... (i2c_master_init, mcp23017_register_read, mcp23017_register_write functions as before) ...

void mcp23017_task(void *pvParameters)
{
    // Configure GPIOA and GPIOB as inputs
    uint8_t data = 0xFF; // Set all pins as inputs
    ESP_ERROR_CHECK(mcp23017_register_write(MCP23017_IODIRA, &data, 1));
    ESP_ERROR_CHECK(mcp23017_register_write(MCP23017_IODIRB, &data, 1));
    ESP_LOGI(TAG, "MCP23017 configured as inputs");

    while (1) {
        // Read GPIOA and GPIOB
        uint8_t gpioa_data;
        uint8_t gpiob_data;
        ESP_ERROR_CHECK(mcp23017_register_read(MCP23017_GPIOA, &gpioa_data, 1));
        ESP_ERROR_CHECK(mcp23017_register_read(MCP23017_GPIOB, &gpiob_data, 1));

        // Print the values
        ESP_LOGI(TAG, "GPIOA: 0x%02x", gpioa_data);
        ESP_LOGI(TAG, "GPIOB: 0x%02x", gpiob_data);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // Initialize I2C
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");

    // Create the task
    xTaskCreate(mcp23017_task, "mcp23017_task", 2048, NULL, 5, NULL);
}

#
#
# METHOD 2. chat
#
#



#include <stdio.h>
#include "driver/i2c_master.h"
#include "esp_log.h"

#define I2C_MASTER_SCL_IO          22       // SCL GPIO number
#define I2C_MASTER_SDA_IO          21       // SDA GPIO number
#define I2C_MASTER_FREQ_HZ         100000   // Frequency of I2C
#define MCP23017_ADDR              0x20     // MCP23017 I2C address
#define MCP23017_IODIRA            0x00     // IODIRA register
#define MCP23017_GPIOA             0x12     // GPIOA register

static const char *TAG = "MCP23017";

// Initialize I2C master using i2c_master.h
esp_err_t i2c_master_init(void) {
    i2c_master_config_t i2c_config = {
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_speed = I2C_MASTER_FREQ_HZ,
    };
    return i2c_master_driver_initialize(I2C_MASTER_NUM, &i2c_config);
}

// Write to MCP23017 register
esp_err_t mcp23017_write_register(uint8_t reg_addr, uint8_t data) {
    i2c_master_cmd_handle_t cmd = i2c_master_cmd_create(I2C_MASTER_NUM);
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (MCP23017_ADDR << 1) | I2C_MASTER_WRITE));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, reg_addr));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, data));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    return i2c_master_cmd_submit(cmd, pdMS_TO_TICKS(1000));
}

// Read from MCP23017 register
esp_err_t mcp23017_read_register(uint8_t reg_addr, uint8_t *data) {
    i2c_master_cmd_handle_t cmd = i2c_master_cmd_create(I2C_MASTER_NUM);
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (MCP23017_ADDR << 1) | I2C_MASTER_WRITE));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, reg_addr));
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (MCP23017_ADDR << 1) | I2C_MASTER_READ));
    ESP_ERROR_CHECK(i2c_master_read_byte(cmd, data, I2C_MASTER_NACK));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    return i2c_master_cmd_submit(cmd, pdMS_TO_TICKS(1000));
}

void app_main(void) {
    ESP_LOGI(TAG, "Initializing I2C master...");
    ESP_ERROR_CHECK(i2c_master_init());

    ESP_LOGI(TAG, "Configuring MCP23017...");
    // Configure GPIOA as input
    ESP_ERROR_CHECK(mcp23017_write_register(MCP23017_IODIRA, 0xFF));

    while (1) {
        uint8_t gpio_state = 0;
        ESP_ERROR_CHECK(mcp23017_read_register(MCP23017_GPIOA, &gpio_state));

        // Extract the first 4 bits
        uint8_t first_four = gpio_state & 0x0F;

        // Log the states
        ESP_LOGI(TAG, "First 4 GPIOs state: 0x%X", first_four);

        // Delay to avoid flooding logs
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
