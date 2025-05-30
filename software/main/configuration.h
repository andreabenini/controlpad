#ifndef CONFIGURATION__H
#define CONFIGURATION__H

#include <esp_err.h>
#include <stdint.h>



// Library defines and structs
#define TAG_CONFIGURATION           "config"
#define CONFIG_MAX                  20
#define CONFIG_LEN_NAME             19
#define CONFIG_LEN_USERNAME         20
#define CONFIG_LEN_PASSWORD         40
#define CONFIG_LEN_LOCAL            20
#define CONFIG_LEN_REMOTE           40

#define STORAGE_NAMESPACE           "config"
#define NVS_ARRAY                   "configurations"
#define NVS_ARRAY_SIZE              "size"

#define TYPE_TCP                    0
#define TYPE_UDP                    1
#define TYPE_HTTP                   2
#define TYPE_BT                     3
#define TYPE_BTLE                   4


typedef struct {
    char name[CONFIG_LEN_NAME];             // Profile name
    uint8_t type;                           // Profile type (TYPE_*): TCP/UDP/HTTP/BT/BTLE
    char username[CONFIG_LEN_USERNAME];     // Username for the network (ssid for TCP/IP)
    char password[CONFIG_LEN_PASSWORD];     // Password for the network (secret for TCP/IP)
    char local[CONFIG_LEN_LOCAL];           // Local address. on TCP: NULL for DHCP
    char remote[CONFIG_LEN_REMOTE];         // Remote HOST/URL/..
} profiles;


// Exported functions
esp_err_t configurationInit();
esp_err_t configurationList(char names[][CONFIG_LEN_NAME], uint8_t *count);
esp_err_t configurationClear();
esp_err_t configurationSave(const profiles *profileArray, uint8_t sizeArray);
esp_err_t configurationLoad(profiles* profile, uint8_t profileNumber);

#endif
