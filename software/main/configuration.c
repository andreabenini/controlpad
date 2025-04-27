#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"                                  // IWYU pragma: keep
#include "esp_system.h"                                         // IWYU pragma: keep
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

#include "configuration.h"


/**
 * Init the configuration module (NVS setup)
 */
esp_err_t configurationInit() {
    esp_err_t result = nvs_flash_init();
    if (result == ESP_ERR_NVS_NO_FREE_PAGES || result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG_CONFIGURATION, "NVS needs to be erased");
        ESP_ERROR_CHECK(nvs_flash_erase());
        result = nvs_flash_init();                  // Retry init
    }
    if (result != ESP_OK) {
        ESP_LOGI(TAG_CONFIGURATION, "Failed to initialize NVS: %s", esp_err_to_name(result));
    } else {
        ESP_LOGI(TAG_CONFIGURATION, "NVS Initialized");
    }
    return result;
} /**/


/**
 * Return a list with all available configurations from storage
 */
esp_err_t configurationList(char names[][CONFIG_LEN_NAME], size_t *count) {
    // Initialize NVS
    *count = 0;
    nvs_handle_t nvs_handle;
    esp_err_t    result = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (result != ESP_OK) {
        ESP_LOGW(TAG_CONFIGURATION, "Error opening NVS namespace: %s", esp_err_to_name(result));
        return result;
    }
    // Get number of available configurations
    size_t used_entries;
    result = nvs_get_used_entry_count(nvs_handle, &used_entries);
    if (result != ESP_OK) {
        ESP_LOGE(TAG_CONFIGURATION, "Error getting entries count: %s", esp_err_to_name(result));
        nvs_close(nvs_handle);
        return result;
    }
    // Listing all keys in NVS
    nvs_iterator_t iterator = NULL;
    nvs_entry_info_t info;
    result = nvs_entry_find(STORAGE_NAMESPACE, NULL, NVS_TYPE_BLOB, &iterator);
    while (result == ESP_OK && *count < CONFIG_MAX-1) {
        nvs_entry_info(iterator, &info);
        if (info.type == NVS_TYPE_BLOB) {
            strncpy(names[*count], info.key, CONFIG_LEN_NAME-1);
            names[*count][CONFIG_LEN_NAME-1] = '\0';
            *count += 1;
        }
        result = nvs_entry_next(&iterator);
    }
    nvs_release_iterator(iterator);
    nvs_close(nvs_handle);
    return ESP_OK;
} /**/
