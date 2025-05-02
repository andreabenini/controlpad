#include "wifi.h"


int  retryNum = 0;
bool wifiInitialized = false;                       // Generic flag to keep wifi current status
EventGroupHandle_t wifiEventGroup;                  // FreeRTOS event group to signal when there's a connection


/**
 * @brief WiFi Event handler
 * @see Used from wifiConnect() only. Do not use it directly
 */
void eventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData) {
    switch (eventId) {
        case WIFI_EVENT_STA_START:
            if (eventBase == WIFI_EVENT) {
                esp_wifi_connect();
            }
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            if (eventBase == WIFI_EVENT) {
                if (retryNum < WIFI_MAXIMUM_RETRY) {
                    esp_wifi_connect();
                    retryNum++;
                    ESP_LOGI(TAG_WIFI, "Retrying WiFi connection (%d) attempt", retryNum+1);
                } else {
                    xEventGroupSetBits(wifiEventGroup, WIFI_FAIL_BIT);
                }
                ESP_LOGI(TAG_WIFI, "WiFi connection failed");
            }
            break;
        case IP_EVENT_STA_GOT_IP:
            if (eventBase == IP_EVENT) {
                ip_event_got_ip_t *event = (ip_event_got_ip_t *)eventData;
                ESP_LOGI(TAG_WIFI, "Received IP:" IPSTR, IP2STR(&event->ip_info.ip));
                retryNum = 0;
                xEventGroupSetBits(wifiEventGroup, WIFI_CONNECTED_BIT);
            }
            break;
    }
} /**/


/**
 * @brief Initialize and connect to WiFi network
 * 
 * @param ssid     (const char*) WiFi SSID
 * @param password (const char*) WiFi password
 * @param ip       (char *) IP address of this device once connected
 * @return (esp_err_t) ESP_OK on success, error code otherwise
 */
esp_err_t wifiConnect(const char* ssid, const char* password, char *ip) {
    // Return immediately if already connected
    if (wifiInitialized) {
        ESP_LOGI(TAG_WIFI, "WiFi already initialized");
        return ESP_OK;
    }
    ESP_LOGI(TAG_WIFI, "Trying to connect to '%s' network", ssid);
    wifiEventGroup = xEventGroupCreate();
    // TODO: Remove ALL these error checks and do something different to better evaluate all possible errors
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));
    // Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,    &eventHandler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,   IP_EVENT_STA_GOT_IP, &eventHandler, NULL, NULL));
    
    // Configure WiFi
    wifi_config_t wifiConfiguration = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,           // WiFi, WPA2 with preshared key (default)
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    // Copy SSID and password to the config
    strlcpy((char*)wifiConfiguration.sta.ssid,     ssid,     sizeof(wifiConfiguration.sta.ssid));
    strlcpy((char*)wifiConfiguration.sta.password, password, sizeof(wifiConfiguration.sta.password));

    // Configure and start WiFi
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfiguration));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG_WIFI, "WiFi configuration completed");
    // Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
    // number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler()
    EventBits_t bits = xEventGroupWaitBits(wifiEventGroup, WIFI_CONNECTED_BIT|WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    strcpy(ip, "");
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG_WIFI, "Connected to AP, SSID:%s", ssid);
        wifiInitialized = true;
        esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        if (netif) {
            esp_netif_ip_info_t ip_info;
            ESP_ERROR_CHECK(esp_netif_get_ip_info(netif, &ip_info));
            sprintf(ip, IPSTR, IP2STR(&ip_info.ip));
        }
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG_WIFI, "Failed to connect to SSID:%s", ssid);
        return ESP_FAIL;
    }
    ESP_LOGE(TAG_WIFI, "Unexpected event");
    return ESP_ERR_INVALID_STATE;
} /**/


esp_err_t wifiDisconnect() {
    if (!wifiInitialized) {
        ESP_LOGI(TAG_WIFI, "WiFi not initialized, disconnection not needed");
        return ESP_OK;
    }
    ESP_LOGI(TAG_WIFI, "Disconnecting from WiFi");
    ESP_ERROR_CHECK(esp_wifi_stop());                   // Stop WiFi
    ESP_ERROR_CHECK(esp_wifi_deinit());                 // Optional: Deinitialize WiFi driver
    wifiInitialized = false;                            // Reset state
    retryNum = 0;
    if (wifiEventGroup != NULL) {                       // Delete event group
        vEventGroupDelete(wifiEventGroup);
        wifiEventGroup = NULL;
    }
    ESP_LOGI(TAG_WIFI, "WiFi disconnected successfully");
    return ESP_OK;
} /**/


/* FIXME: I am now working on this one
ESP_ERROR_CHECK failed: esp_err_t 0x103 (ESP_ERR_INVALID_STATE) at 0x4200c340
--- 0x4200c340: wifiConnect at /home/ben/Documents/controlpad/software/main/wifi.c:46 (discriminator 1)

file: "./main/wifi.c" line 46
func: wifiConnect
expression: esp_event_loop_create_default()

abort() was called at PC 0x40388d5d on core 0
--- 0x40388d5d: _esp_error_check_failed at /home/ben/esp/v5.4/esp-idf/components/esp_system/esp_err.c:49
*/
