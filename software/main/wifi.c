#include "wifi.h"

uint8_t             wifiRetries,                    // Connection retries before dropping it off
                    wifiStatus = 0;                 // Generic flag to keep wifi current status
EventGroupHandle_t  wifiEventGroup = NULL;          // FreeRTOS event group to signal when there's a connection
esp_event_handler_instance_t instanceAnyID;         // Managed event handlers for esp_event_handler_instance_register()
esp_event_handler_instance_t instanceGotIP;


/**
 * @brief WiFi Event handler
 * @see   Used from wifiConnect() only. Do NOT use it directly
 */
void wifiEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData) {
    if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_DISCONNECTED) {
        if (wifiRetries < WIFI_RETRY_MAXIMUM) {
            ESP_LOGW(TAG_WIFI, "Retrying WiFi connection (%d) attempt", wifiRetries+1);
            xEventGroupClearBits(wifiEventGroup, WIFI_BIT_CONNECTED); // Clear connected bit
            esp_wifi_connect();
            vTaskDelay(WIFI_RETRY_TIMEOUT / portTICK_PERIOD_MS);
            wifiRetries++;
        } else {
            xEventGroupSetBits(wifiEventGroup, WIFI_BIT_FAIL);
            ESP_LOGE(TAG_WIFI, "AP connection failed, aborting network connection");
            esp_wifi_disconnect();
        }
    } else if (eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)eventData;
        ESP_LOGI(TAG_WIFI, "    - Received IP " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifiEventGroup,   WIFI_BIT_CONNECTED);
        xEventGroupClearBits(wifiEventGroup, WIFI_BIT_FAIL);        // Clear connected bit
        wifiRetries = 0;
    }
} /**/


/**
 * 
 */
esp_err_t wifiInit() {
    if (wifiEventGroup != NULL) {
        ESP_LOGI(TAG_WIFI, "    - WiFi already initalized");
        return ESP_OK;
    }
    ESP_LOGI(TAG_WIFI, "    - Network init");
    ESP_RETURN_ON_ERROR(esp_netif_init(),                TAG_WIFI, "esp_netif_init() Initialization error");
    ESP_RETURN_ON_ERROR(esp_event_loop_create_default(), TAG_WIFI, "Cannot create the event loop");
    esp_netif_t *netifHandler;                                          // netif pointer
    netifHandler = esp_netif_create_default_wifi_sta();                 // Create a network interface. WiFi station mode (client)
    if (netifHandler==NULL) {
        ESP_LOGE(TAG_WIFI, "    Failed to create default WiFi network interface");
        return ESP_ERR_WIFI_STATE;
    }
    wifiEventGroup = xEventGroupCreate();
    // ... Register event handlers
    ESP_LOGI(TAG_WIFI, "    - registering handlers: ESP_EVENT_ANY_ID");
    ESP_RETURN_ON_ERROR(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,    &wifiEventHandler, NULL, &instanceAnyID), TAG_WIFI, "Cannot esp_event_handler_instance_register() on ANY_EVENT");
    ESP_LOGI(TAG_WIFI, "    - registering handlers: IP_EVENT_STA_GOT_IP");
    ESP_RETURN_ON_ERROR(esp_event_handler_instance_register(IP_EVENT,   IP_EVENT_STA_GOT_IP, &wifiEventHandler, NULL, &instanceGotIP), TAG_WIFI, "Cannot esp_event_handler_instance_register() on GOT_IP");
    return ESP_OK;
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
    esp_err_t result;
    wifiRetries = 0;
    ESP_LOGI(TAG_WIFI, "Trying to connect to '%s' network", ssid);
    ESP_RETURN_ON_ERROR(wifiInit(), TAG_WIFI, "Cannot initialize the WiFi stack");
    // Init wifi and register event handlers
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_RETURN_ON_ERROR(esp_wifi_init(&config), TAG_WIFI, "esp_wifi_init() WiFi init error");
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
    // Copy SSID and password to the config, instead of direct assign in the [wifi_config_t] struct I prefer this to avoid casting
    strlcpy((char*)wifiConfiguration.sta.ssid,     ssid,     sizeof(wifiConfiguration.sta.ssid));
    strlcpy((char*)wifiConfiguration.sta.password, password, sizeof(wifiConfiguration.sta.password));

    // Configure and start WiFi
    // TODO: wifi_country_t wifi_country = {.cc="EU", .schan=1, .nchan=13, .policy=WIFI_COUNTRY_POLICY_AUTO};
    // ESP_RETURN_ON_ERROR( esp_wifi_set_country(&wifi_country) );
    ESP_LOGI(TAG_WIFI, "    - Setting mode: station");
    ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_STA), TAG_WIFI, "esp_wifi_set_mode() Cannot set WiFi station mode");
    ESP_LOGI(TAG_WIFI, "    - Setting configuration");
    ESP_RETURN_ON_ERROR(esp_wifi_set_config(WIFI_IF_STA, &wifiConfiguration), TAG_WIFI, "esp_wifi_set_config() Cannot setup wifi configuration");
    ESP_LOGI(TAG_WIFI, "    - Starting");
    ESP_RETURN_ON_ERROR(esp_wifi_start(), TAG_WIFI, "Cannot start WiFi");

    // XXX: Testing this setting: set maximum power to avoid problems with C3-Super-Mini-Flaw bug
    /**
     ESP32-C3 super mini has a broken antenna design. Reducing or changing the Tx-Power to reduce reflections
          (see: https://forum.arduino.cc/t/no-wifi-connect-with-esp32-c3-super-mini/1324046/24)
     There are two possible solutions:
     - Remove the C3 from breadboard, to be precise: remove Pin21, the one which is close by the xtal 40MHz generator
          C3 might be soldered at 90deg with one pin still connected while other might be moved away from the xtal.
          If you have a decent MCU or an external antenna this won't affect you (hopefully)
     - Reduce TX-Power, something like:
          >> WiFi.setTxPower(WIFI_POWER_8_5dBm);                  // WIFI_POWER_8_5dBm = 34,  // 8.5dBm
          >> ESP_ERROR_CHECK(esp_wifi_set_max_tx_power(80));      // Do not use 80, go down below...
          Here's current enum definition:
              typedef enum {
                  WIFI_POWER_19_5dBm = 78,// 19.5dBm
                  WIFI_POWER_19dBm = 76,// 19dBm
                  WIFI_POWER_18_5dBm = 74,// 18.5dBm
                  WIFI_POWER_17dBm = 68,// 17dBm
                  WIFI_POWER_15dBm = 60,// 15dBm
                  WIFI_POWER_13dBm = 52,// 13dBm
                  WIFI_POWER_11dBm = 44,// 11dBm
                  WIFI_POWER_8_5dBm = 34,// 8.5dBm
                  WIFI_POWER_7dBm = 28,// 7dBm
                  WIFI_POWER_5dBm = 20,// 5dBm
                  WIFI_POWER_2dBm = 8,// 2dBm
                  WIFI_POWER_MINUS_1dBm = -4// -1dBm
              } wifi_power_t;
          Or plain simple ESP-IDF libraries as usual, reference:
          https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_wifi.html#_CPPv425esp_wifi_set_max_tx_power6int8_t
     XXX: Also testing slow tx init with some delays in between
     */
    ESP_LOGI(TAG_WIFI, "    - Power regulations (8..34)");
    vTaskDelay(pdMS_TO_TICKS(50));
    ESP_RETURN_ON_ERROR(esp_wifi_set_max_tx_power(8), TAG_WIFI, "Cannot set WiFi max power to 8");
    vTaskDelay(pdMS_TO_TICKS(5000));
    ESP_RETURN_ON_ERROR(esp_wifi_set_max_tx_power(34), TAG_WIFI, "Cannot set WiFi max power to 34");

    ESP_LOGI(TAG_WIFI, "    - Configuration completed");
    // Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
    // number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler()
    EventBits_t bits = xEventGroupWaitBits(wifiEventGroup, WIFI_BIT_CONNECTED|WIFI_BIT_FAIL, pdFALSE, pdFALSE, portMAX_DELAY);

    strcpy(ip, "");
    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually happened */
    if (bits & WIFI_BIT_CONNECTED) {
        ESP_LOGI(TAG_WIFI, "    - Connected to AP, SSID:%s", ssid);
        esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        if (netif) {
            esp_netif_ip_info_t ip_info;
            ESP_RETURN_ON_ERROR(esp_netif_get_ip_info(netif, &ip_info), TAG_WIFI, "Cannot get IP information from the interface");
            sprintf(ip, IPSTR, IP2STR(&ip_info.ip));
        }
        ESP_LOGI(TAG_WIFI, "    - Initialization completed (%s)", ip);
        wifiStatus  = 1;
        result = ESP_OK;
    } else if (bits & WIFI_BIT_FAIL) {
        ESP_LOGE(TAG_WIFI, "    - Failed to connect to SSID:%s", ssid);
        result = ESP_FAIL;
    } else {
        ESP_LOGE(TAG_WIFI, "    - Unknown event: %u", (unsigned int) bits);
        result = ESP_ERR_INVALID_STATE;
    }
    return result;
} /**/


esp_err_t wifiDisconnect() {
    if (wifiStatus == 0) {
        ESP_LOGW(TAG_WIFI, "WiFi not initialized, disconnection not needed");
        return ESP_OK;
    }
    // Stop Wifi
    wifiStatus = 0;
    ESP_LOGI(TAG_WIFI, "Disconnecting from WiFi");
    ESP_RETURN_ON_ERROR(esp_wifi_disconnect(),  TAG_WIFI, "Cannot disconnect WiFi");                // Network stack disconnection
    ESP_RETURN_ON_ERROR(esp_wifi_stop(),        TAG_WIFI, "Cannot stop WiFi network interface");    // Stop WiFi
    ESP_RETURN_ON_ERROR(esp_wifi_deinit(),      TAG_WIFI, "Cannot deinit WiFi network interface");  // Optional: Deinitialize WiFi driver (suggested even if optional)
    xEventGroupClearBits(wifiEventGroup, WIFI_BIT_CONNECTED | WIFI_BIT_FAIL);                       // Clear status bits
    
    // Removing handlers and default loop created from esp_event_loop_create_default()
    ESP_LOGI(TAG_WIFI, "    - Disconnected successfully");
    return ESP_OK;
} /**/
