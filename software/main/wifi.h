#ifndef WIFI__H
#define WIFI__H

#include <string.h>                                             // IWYU pragma: keep
#include <freertos/FreeRTOS.h>                                  // IWYU pragma: keep
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_system.h>                                         // IWYU pragma: keep
#include <esp_wifi.h>                                           // IWYU pragma: keep
#include <esp_event.h>                                          // IWYU pragma: keep
#include <esp_log.h>
#include <esp_err.h>
#include <esp_check.h>

// Network
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>


// Program defines
#include "configuration.h"


// Header defines
#define TAG_WIFI                        "Wireless"
#define WIFI_RETRY_MAXIMUM              5                       // WiFi maximum reconnections before dropping it off
#define WIFI_RETRY_TIMEOUT              500                     // Delay between two retries (in ms)
/* The event group [wifiEvenGroup] allows multiple bits for each event, but I only care about two events:
 *      - I am connected to the AP with an IP
 *      - I failed to connect after the maximum amount of retries */
#define WIFI_BIT_CONNECTED              BIT0
#define WIFI_BIT_FAIL                   BIT1

#define TCP_RETRY_MAXIMUM               5                       // TCP connect() max retries


// Functions
void      wifiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
esp_err_t wifiInit();
esp_err_t wifiConnect(const char* ssid, const char* password, char *ip);
esp_err_t wifiDisconnect();

int  tcpSocket(char* remoteAddress);
void tcpSocketClose(int socket);

void tcpDataSend(int socket, char *data);
void tcpDataReceive(int socket, char* buffer);

#endif
