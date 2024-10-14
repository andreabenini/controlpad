#ifndef MAIN__H
#define MAIN__H


/**
 * Set project log level, Default is ESP_LOG_INFO. This define can disable logging all at once (for dealing with other serial devices for example)
 * Log level can also be changed with:
 *      - CONFIG_LOG_MAXIMUM_LEVEL setting in menuconfig
 *      - esp_log_level_set() in esp_log.h
 * Possible log values are:
 *      ESP_LOG_NONE
 *      ESP_LOG_ERROR
 *      ESP_LOG_WARN
 *      ESP_LOG_INFO        (default)
 *      ESP_LOG_DEBUG
 *      ESP_LOG_VERBOSE
 */
// #define LOG_LOCAL_LEVEL ESP_LOG_NONE


// Bluetooth variables
#define VAR_BT_ADDRESS              "bluetooth"

// #include "secret.h"
// Inside secret.h there are just these two definitions, feel free to create
// your secret.h file and keep them separated or uncomment those two lines below
// #define WIFI_SSID                   "yourSSID"
// #define WIFI_PASSWORD               "SecretWIFIPassword"
// [Default values] Wireless configuration
#define WIFI_MAX_RETRIES            5
#define WIFI_RETRY_TIMEOUT          1500        // ms
// [Default values] Bluetooth configuration
#define DEFAULT_BT_MAC_ADDRESS      "1a:2b:3c:01:01:01"

// Camera configuration
// Program configuration
#define TAG_APP                     "ControlPad"
// #define BOOTLOADER_ONLY


// User defined functions
// #include "esp_task_wdt.h"
// #define PROGRAM_HALT                esp_task_wdt_delete(NULL); while (1){vTaskDelay(portMAX_DELAY);}
// #define PROGRAM_HALT                while (1){vTaskDelay(portMAX_DELAY);}

#endif
