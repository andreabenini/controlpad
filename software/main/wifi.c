#include "wifi.h"


uint8_t             wifiRetries,                    // Connection retries before dropping it off
                    wifiStatus = 0;                 // Generic flag to keep wifi current status
EventGroupHandle_t  wifiEventGroup;                 // FreeRTOS event group to signal when there's a connection
esp_netif_t         *netifHandler = NULL;           // Global variable to store the netif pointer
esp_event_handler_instance_t instanceAnyID;         // Managed event handlers for esp_event_handler_instance_register()
esp_event_handler_instance_t instanceGotIP;


/**
 * @brief WiFi Event handler
 * @see   Used from wifiConnect() only. Do NOT use it directly
 */
void wifiEvent(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData) {
    if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_DISCONNECTED) {
        if (wifiRetries < WIFI_RETRY_MAXIMUM) {
            ESP_LOGW(TAG_WIFI, "Retrying WiFi connection (%d) attempt", wifiRetries+1);
            vTaskDelay(WIFI_RETRY_TIMEOUT / portTICK_PERIOD_MS);
            esp_wifi_connect();
            wifiRetries++;
        } else {
            xEventGroupSetBits(wifiEventGroup, WIFI_BIT_FAIL);
            ESP_LOGE(TAG_WIFI, "AP connection failed, aborting network connection");
        }
        wifiStatus = 0;
    } else if (eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)eventData;
        ESP_LOGI(TAG_WIFI, "    Received IP:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifiEventGroup, WIFI_BIT_CONNECTED);
        wifiRetries = 0;
        wifiStatus  = 1;
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
    if (wifiStatus == 1) {
        ESP_LOGW(TAG_WIFI, "WiFi already initialized");
        return ESP_OK;
    }
    wifiRetries = 0;
    ESP_LOGI(TAG_WIFI, "Trying to connect to '%s' network", ssid);
    wifiEventGroup = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    esp_err_t result = esp_event_loop_create_default();
    if (result != ESP_OK) {
        ESP_LOGE(TAG_WIFI, "    Failed to create the event loop. Error: %s", esp_err_to_name(result));
        return result; // Abort further initialization, including Wi-Fi
    }
    netifHandler = esp_netif_create_default_wifi_sta();                 // Create a network interface. WiFi station mode (client)
    if (netifHandler==NULL) {
        ESP_LOGE(TAG_WIFI, "    Failed to create default WiFi network interface");
        return ESP_ERR_WIFI_STATE;
    }

    // Init wifi and register event handlers
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));
    // ... Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,    &wifiEvent, NULL, &instanceAnyID));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,   IP_EVENT_STA_GOT_IP, &wifiEvent, NULL, &instanceGotIP));
    
    // Configure WiFi
    wifi_config_t wifiConfiguration = {
        .sta = {
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
            /** Setting a password implies station will connect to all security modes including WEP/WPA.
             *  However these modes are deprecated and not advisable to be used. In case Access point doesn't support WPA2,
             *  these modes can be enabled by commenting below line */
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,           // WiFi, WPA2 with preshared key (default)
        },
    };
    // Copy SSID and password to the config
    strlcpy((char*)wifiConfiguration.sta.ssid,     ssid,     sizeof(wifiConfiguration.sta.ssid));
    strlcpy((char*)wifiConfiguration.sta.password, password, sizeof(wifiConfiguration.sta.password));

    // Configure and start WiFi
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfiguration));
    // DEBUG: Here's the error (sometimes the wifi module hangs up)
    //      - phy_init: saving new calibration data because of checksum failure, mode(0)
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG_WIFI, "    WiFi configuration completed");
    // Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
    // number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler()
    EventBits_t bits = xEventGroupWaitBits(wifiEventGroup, WIFI_BIT_CONNECTED|WIFI_BIT_FAIL, pdFALSE, pdFALSE, portMAX_DELAY);
    strcpy(ip, "");
    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually happened */
    if (bits & WIFI_BIT_CONNECTED) {
        ESP_LOGI(TAG_WIFI, "    Connected to AP, SSID:%s", ssid);
/* TODO: Reactivate once done with the user profile change status
        esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        if (netif) {
            esp_netif_ip_info_t ip_info;
            ESP_ERROR_CHECK(esp_netif_get_ip_info(netif, &ip_info));
            sprintf(ip, IPSTR, IP2STR(&ip_info.ip));
        }
        ESP_LOGI(TAG_WIFI, "    Initialization completed");
*/
        result = ESP_OK;
    } else if (bits & WIFI_BIT_FAIL) {
        ESP_LOGE(TAG_WIFI, "    Failed to connect to SSID:%s", ssid);
        result = ESP_FAIL;
    } else {
        ESP_LOGE(TAG_WIFI, "    Unknown event: %u", (unsigned int) bits);
        result = ESP_ERR_INVALID_STATE;
    }
    vEventGroupDelete(wifiEventGroup);
    return result;
} /**/


esp_err_t wifiDisconnect() {
    if (wifiStatus == 0) {
        ESP_LOGW(TAG_WIFI, "WiFi not initialized, disconnection not needed");
        return ESP_OK;
    }
    wifiStatus = 0;
    ESP_LOGI(TAG_WIFI, "Disconnecting from WiFi");

    ESP_LOGI(TAG_WIFI, "    Unregistering associated events");
    esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID,    &instanceAnyID);
    esp_event_handler_instance_unregister(IP_EVENT,   IP_EVENT_STA_GOT_IP, &instanceGotIP);

    // Stop Wifi
    esp_wifi_disconnect();
    ESP_ERROR_CHECK(esp_wifi_stop());                   // Stop WiFi
    // FIXME: Try this without and with the upcoming instruction
    // ESP_ERROR_CHECK(esp_wifi_deinit());                 // Optional: Deinitialize WiFi driver (somewhat suggested even if still optional) 


    // Destroy the network interface                    // From esp_netif_create_default_wifi_sta() init in the Connect()
    ESP_LOGI(TAG_WIFI, "    Destroying the network interface");
    esp_netif_destroy(netifHandler);
    
    // Removing handlers and default loop created from esp_event_loop_create_default()
    ESP_LOGI(TAG_WIFI, "    Removing event loop");
    esp_event_loop_delete_default();
    ESP_LOGI(TAG_WIFI, "    WiFi disconnected successfully");
    return ESP_OK;
} /**/
