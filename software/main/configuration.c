#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"                                  // IWYU pragma: keep
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_system.h"                                         // IWYU pragma: keep

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


esp_err_t configurationClear() {
    // [nvsHandle] init
    nvs_handle_t nvsHandle;
    esp_err_t result = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &nvsHandle);
    if (result != ESP_OK) {
        ESP_LOGE(TAG_CONFIGURATION, "Error opening NVS: %s", esp_err_to_name(result));
        return result;
    }
    // Storing the size of the array
    result = nvs_set_u8(nvsHandle, NVS_ARRAY_SIZE, 0);
    if (result != ESP_OK) {
        ESP_LOGE(TAG_CONFIGURATION, "Cannot write the array size: %s", esp_err_to_name(result));
        nvs_close(nvsHandle);
        return result;
    }
    ESP_LOGI(TAG_CONFIGURATION, "    - Profile configuration cleared from NVS");
    return ESP_OK;
} /**/


esp_err_t configurationSave(const profiles *profileArray, uint8_t sizeArray) {
    if (sizeArray==0) {
        return configurationClear();
    }
    // [nvsHandle] init
    nvs_handle_t nvsHandle;
    esp_err_t result = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &nvsHandle);
    if (result != ESP_OK) {
        ESP_LOGE(TAG_CONFIGURATION, "Error opening NVS: %s", esp_err_to_name(result));
        return result;
    }
    // Storing the size of the array
    result = nvs_set_u8(nvsHandle, NVS_ARRAY_SIZE, sizeArray);
    if (result != ESP_OK) {
        ESP_LOGE(TAG_CONFIGURATION, "Cannot write the array size: %s", esp_err_to_name(result));
        nvs_close(nvsHandle);
        return result;
    }
    // Storing the array as a data blob
    result = nvs_set_blob(nvsHandle, NVS_ARRAY, profileArray, sizeArray*sizeof(profiles));
    if (result != ESP_OK) {
        ESP_LOGE(TAG_CONFIGURATION, "Error writing data array to NVS: %s", esp_err_to_name(result));
        nvs_close(nvsHandle);
        return result;
    }
    // NVS commit
    result = nvs_commit(nvsHandle);
    if (result != ESP_OK) {
        ESP_LOGE(TAG_CONFIGURATION, "Error while committing NVS updates: %s", esp_err_to_name(result));
        return result;
    }
    nvs_close(nvsHandle);
    ESP_LOGI(TAG_CONFIGURATION, "    - Configuration saved in the NVS area");
    return ESP_OK;
} /**/


/**
 * Return a list with all available configurations from storage
 */
esp_err_t configurationList(char names[][CONFIG_LEN_NAME], uint8_t *count) {
    // Initialize NVS
    *count = 0;
    nvs_handle_t nvsHandle;
    esp_err_t    result = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &nvsHandle);
    if (result != ESP_OK) {
        ESP_LOGE(TAG_CONFIGURATION, "Error opening NVS: %s", esp_err_to_name(result));
        return result;
    }
    // Get number of available configurations
    result = nvs_get_u8(nvsHandle, NVS_ARRAY_SIZE, count);
    if (result != ESP_OK) {
        ESP_LOGE(TAG_CONFIGURATION, "Error getting entries count: %s", esp_err_to_name(result));
        nvs_close(nvsHandle);
        return result;
    }
    // Listing all configuration names (keys) in NVS
    size_t configurationsSize;
    result = nvs_get_blob(nvsHandle, NVS_ARRAY, NULL, &configurationsSize);
    if (result != ESP_OK) {
        ESP_LOGE(TAG_CONFIGURATION, "Error getting the size of the data array from NVS: %s", esp_err_to_name(result));
        nvs_close(nvsHandle);
        return result;
    }
    *count = configurationsSize / sizeof(profiles);
    profiles *configurations  = (profiles *)malloc(configurationsSize);
    if (configurations == NULL) {
        ESP_LOGE(TAG_CONFIGURATION, "Cannot allocate memory for %zu", *count);
        return ESP_ERR_NO_MEM;
    }
    result = nvs_get_blob(nvsHandle, NVS_ARRAY, configurations, &configurationsSize);
    if (result != ESP_OK) {
        ESP_LOGE(TAG_CONFIGURATION, "Error getting data array from NVS: %s", esp_err_to_name(result));
        free(configurations);
        nvs_close(nvsHandle);
        return result;
    }
    nvs_close(nvsHandle);
    // Iterating available configuration profiles
    ESP_LOGI(TAG_CONFIGURATION, "Profile list [%d]", *count);
    for (size_t i=0; i<*count; i++) {
        strncpy(names[i], configurations[i].name, CONFIG_LEN_NAME-1);
        names[i][CONFIG_LEN_NAME-1] = '\0';
        ESP_LOGI(TAG_CONFIGURATION, "    [%s]", names[i]);
    }
    free(configurations);
    return ESP_OK;
} /**/


esp_err_t configurationLoad(profiles *profile, uint8_t profileNumber) {
    // Initialize NVS
    nvs_handle_t nvsHandle;
    esp_err_t    result = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &nvsHandle);
    if (result != ESP_OK) {
        ESP_LOGE(TAG_CONFIGURATION, "Error opening NVS: %s", esp_err_to_name(result));
        return result;
    }
    // Get number of available configurations
    uint8_t count;
    result = nvs_get_u8(nvsHandle, NVS_ARRAY_SIZE, &count);
    if (result != ESP_OK) {
        ESP_LOGE(TAG_CONFIGURATION, "Error getting entries count: %s", esp_err_to_name(result));
        nvs_close(nvsHandle);
        return result;
    }
    if (count <= profileNumber) {
        ESP_LOGE(TAG_CONFIGURATION, "Element %d doesn't exists, configuration only has %d items", profileNumber, count);
        return ESP_ERR_INVALID_ARG;
    }
    // Listing all configuration names (keys) in NVS
    size_t configurationsSize = 0;
    result = nvs_get_blob(nvsHandle, NVS_ARRAY, NULL, &configurationsSize);
    if (result != ESP_OK) {
        ESP_LOGE(TAG_CONFIGURATION, "Error getting the size of the data array from NVS: %s", esp_err_to_name(result));
        nvs_close(nvsHandle);
        return result;
    }
    count = configurationsSize / sizeof(profiles);
    profiles *configurations  = (profiles *)malloc(configurationsSize);
    if (configurations == NULL) {
        ESP_LOGE(TAG_CONFIGURATION, "Cannot allocate memory for %zu", count);
        return ESP_ERR_NO_MEM;
    }
    result = nvs_get_blob(nvsHandle, NVS_ARRAY, configurations, &configurationsSize);
    if (result != ESP_OK) {
        ESP_LOGE(TAG_CONFIGURATION, "Error getting data array from NVS: %s", esp_err_to_name(result));
        free(configurations);
        nvs_close(nvsHandle);
        return result;
    }
    nvs_close(nvsHandle);
    *profile = configurations[profileNumber];
    free(configurations);
    return ESP_OK;
} /**/
