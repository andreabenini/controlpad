#include "serial.h"

#include "esp_err.h"
#include "esp_log.h"


extern QueueHandle_t queueMessage;


void taskSerial(void *pvParameters) {
    ESP_LOGI(TAG_SERIAL, "Serial receiver task started");
    uint8_t data[BUFFER_READ];                              // Buffer for incoming data
    int dataLength;
    char *buffer = (char *)malloc(BUFFER_SIZE);            // Configuration buffer
    char *configuration, *configurationEnd;
    memset(buffer, 0x00, BUFFER_SIZE);

    // Configure USB Serial/JTAG driver
    usb_serial_jtag_driver_config_t usbSerialJTAGconfig;
    usbSerialJTAGconfig.rx_buffer_size = 1024;
    usbSerialJTAGconfig.tx_buffer_size = 1024;
    // Install USB Serial/JTAG driver with the configuration
    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usbSerialJTAGconfig));

    configuration = NULL;
    while (1) {
        // Read data from USB Serial/JTAG
        dataLength = usb_serial_jtag_read_bytes(data, BUFFER_READ - 1, pdMS_TO_TICKS(100));
        if (dataLength > 0) {
            data[dataLength] = '\0';

            // Append incoming [data] to [buffer], when possible
            if (strlen(buffer)+dataLength >= BUFFER_SIZE-1) {
                if (configuration != NULL) {
                    configuration = NULL;
                    memset(buffer, 0x00, BUFFER_SIZE);
                } else {
                    char *source = buffer + strlen(buffer) - strlen(PATTERN_BEGIN);
                    memmove(buffer, source, strlen(source));
                    buffer[strlen(source)] = '\0';
                }
            }
            memcpy(buffer+strlen(buffer), data, dataLength+1);

            // Search pattern begin/end
            if (configuration == NULL) {
                configuration = strstr(buffer, PATTERN_BEGIN);
            }
            configurationEnd  = strstr(buffer, PATTERN_END);
            if (configuration != NULL  &&  configurationEnd != NULL) {
                configuration += strlen(PATTERN_BEGIN);
                *configurationEnd = '\0';
                loadConfiguration(configuration);
                configuration = NULL;
                memset(buffer, 0x00, BUFFER_SIZE);
            }
        }
        // Small delay to prevent CPU hogging
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    free(buffer);
    vTaskDelete(NULL);
} /**/


void loadConfiguration(char *config) {
    ESP_LOGI(TAG_SERIAL, "<<NEW CONFIG %d>> %s", strlen(config), config);
    cJSON *root = cJSON_Parse((const char *)config);
    cJSON *itemIterator;
    if (root == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            ESP_LOGE(TAG_SERIAL, "JSON error: %s", error_ptr);
        }
        return;
    }
    size_t itemCurrent, itemCount = cJSON_GetArraySize(root);
    ESP_LOGI(TAG_SERIAL, "Received new configuration, %d items found", itemCount);
    if (itemCount == 0 || itemCount >=CONFIG_MAX) {
        cJSON_Delete(root);
        configurationClear();
        uint32_t signal=0;          // 0: No config, 1: loaded config
        BaseType_t result = xQueueSendToBack(
            queueMessage,           // The queue to send to
            &signal,                // The data to send (our signal)
            0                       // Don't block if the queue is full (shouldn't be in this case)
        );
        if (result == pdPASS) {
            ESP_LOGI(TAG_SERIAL, "    - Sending notification to main loop (signal:%lu)", (long unsigned int)signal);
        } else {
            ESP_LOGE(TAG_SERIAL, "Cannot send notification to main loop: %lu", (long unsigned int)result);
        }

        return;
    }
    profiles *configurations = (profiles *)malloc(itemCount * sizeof(profiles));
    if (configurations == NULL) {
        ESP_LOGE(TAG_SERIAL, "Cannot allocate memory for %d profiles", itemCount);
        return;
    }
    itemCurrent = 0;
    cJSON_ArrayForEach(itemIterator, root) {
        if (cJSON_IsObject(itemIterator)) {
            // Profile name
            char *name = itemIterator->string;
            strncpy(configurations[itemCurrent].name, name, CONFIG_LEN_NAME-1);
            configurations[itemCurrent].name[CONFIG_LEN_NAME-1] = '\0';
            ESP_LOGI(TAG_SERIAL, "Name: %s", name);
            // Profile type
            char *mode = cJSON_GetStringValue(cJSON_GetObjectItem(itemIterator, "mode"));
            if (!strcmp(mode, "udp")) {
                configurations[itemCurrent].type = TYPE_UDP;
            } else if (!strcmp(mode, "http")) {
                configurations[itemCurrent].type = TYPE_HTTP;
            } else {
                configurations[itemCurrent].type = TYPE_TCP;
            }
            ESP_LOGI(TAG_SERIAL, "    - Mode [%d]  %s", configurations[itemCurrent].type, mode);
            // Username/Password
            char *username = cJSON_GetStringValue(cJSON_GetObjectItem(itemIterator, "wifi_ssid"));
            strncpy(configurations[itemCurrent].username, username, CONFIG_LEN_USERNAME-1);
            configurations[itemCurrent].username[CONFIG_LEN_USERNAME-1] = '\0';
            ESP_LOGI(TAG_SERIAL, "    - Username  %s", configurations[itemCurrent].username);
            char *password = cJSON_GetStringValue(cJSON_GetObjectItem(itemIterator, "wifi_password"));
            strncpy(configurations[itemCurrent].password, password, CONFIG_LEN_PASSWORD-1);
            configurations[itemCurrent].password[CONFIG_LEN_PASSWORD-1] = '\0';
            ESP_LOGI(TAG_SERIAL, "    - Password  %s", configurations[itemCurrent].password);
            // Local Address
            char *local    = cJSON_GetStringValue(cJSON_GetObjectItem(itemIterator, "wifi_ip"));
            if (local != NULL) {
                strncpy(configurations[itemCurrent].local, local, CONFIG_LEN_LOCAL-1);
                configurations[itemCurrent].local[CONFIG_LEN_LOCAL-1] = '\0';
            } else {
                configurations[itemCurrent].local[0] = '\0';
            }
            ESP_LOGI(TAG_SERIAL, "    - Local     [%s]", configurations[itemCurrent].local);
            // Remote Address
            char  *ip;
            cJSON *port;
            switch (configurations[itemCurrent].type) {
                case TYPE_UDP:
                case TYPE_TCP:
                default:
                    ip   = cJSON_GetStringValue(cJSON_GetObjectItem(itemIterator, "remote_ip"));
                    port = cJSON_GetObjectItem(itemIterator, "remote_port");
                    if (cJSON_IsNumber(port)) {
                        snprintf(configurations[itemCurrent].remote, CONFIG_LEN_REMOTE-1, "%s:%d", ip, (uint16_t) cJSON_GetNumberValue(port));
                    } else {
                        strncpy(configurations[itemCurrent].remote, ip, CONFIG_LEN_REMOTE-1);
                    }
                    configurations[itemCurrent].remote[CONFIG_LEN_REMOTE-1] = '\0';
                    break;
            }
            ESP_LOGI(TAG_SERIAL, "    - Remote    %s", configurations[itemCurrent].remote);
            // Buttons map
            cJSON *map = cJSON_GetObjectItem(itemIterator, "map");
            if (!cJSON_IsObject(map)) {
                ESP_LOGW(TAG_SERIAL, "No keyboard mapping found, no mapping preset loaded");
            } else {
                _loadMapping(map, configurations, itemCurrent);
            }

            ESP_LOGI(TAG_SERIAL, "    - ButtonMap %s", configurations[itemCurrent].remote);
        }
        itemCurrent++;
    }
    // Save loaded configuration and inform mainloop.c about it
    ESP_LOGI(TAG_SERIAL, "Configuration loaded");
    if (configurationSave(configurations,  itemCount) == ESP_OK) {
        uint32_t signal=1;
        BaseType_t result = xQueueSendToBack(
            queueMessage,           // The queue to send to
            &signal,                // The data to send (our signal)
            0                       // Don't block if the queue is full (shouldn't be in this case)
        );
        if (result == pdPASS) {
            ESP_LOGI(TAG_SERIAL, "    - Sending notification to main loop (signal:%lu)", (long unsigned int)signal);
        } else {
            ESP_LOGE(TAG_SERIAL, "Cannot send notification to main loop: %lu", (long unsigned int)result);
        }
    }
    free(configurations);
    cJSON_Delete(root);
} /**/

void _loadMapping(cJSON* buttonList, profiles* configurations, size_t current) {
    ESP_LOGI(TAG_SERIAL, "Loading button mapping");
    cJSON* element = NULL;
    uint8_t index;
    cJSON_ArrayForEach(element, buttonList) {
        if (cJSON_IsString(element) && (element->string != NULL)) {
            if        (!strcmp(element->string, "a")) {
                index = 0;
            } else if (!strcmp(element->string, "b")) {
                index = 1;
            } else if (!strcmp(element->string, "x")) {
                index = 2;
            } else if (!strcmp(element->string, "y")) {
                index = 3;
            } else if (!strcmp(element->string, "menu")) {
                index = 4;
            } else if (!strcmp(element->string, "option")) {
                index = 5;
            } else if (!strcmp(element->string, "right_top")) {
                index = 6;
            } else if (!strcmp(element->string, "right_bottom")) {
                index = 7;
            } else if (!strcmp(element->string, "dpad_left")) {
                index = 8;
            } else if (!strcmp(element->string, "dpad_right")) {
                index = 9;
            } else if (!strcmp(element->string, "dpad_up")) {
                index = 10;
            } else if (!strcmp(element->string, "dpad_down")) {
                index = 11;
            } else if (!strcmp(element->string, "select")) {
                index = 12;
            } else if (!strcmp(element->string, "left_top")) {
                index = 13;
            } else if (!strcmp(element->string, "left_bottom")) {
                index = 14;
            } else {
                index = UINT8_MAX;
            }
            if (index != UINT8_MAX) {
                _mapString(configurations, current, index, cJSON_GetStringValue(element));
            }
        }
    }
} /**/


void _mapString(profiles* configurations, size_t currentConfig, size_t item, char* evalString) {
    ESP_LOGW(TAG_SERIAL, "%d = %s", item, evalString);       // DEBUG:
    strncpy(configurations[currentConfig].map[item], evalString, CONFIG_LEN_MAPSTRING);
    configurations[currentConfig].map[item][CONFIG_LEN_MAPSTRING-1] = '\0';
}
