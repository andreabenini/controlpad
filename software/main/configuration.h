#ifndef CONFIGURATION__H
#define CONFIGURATION__H

#include <esp_err.h>



// Library defines and structs
#define TAG_CONFIGURATION           "config"
#define CONFIG_MAX                  20
#define CONFIG_LEN_NAME             19
#define CONFIG_LEN_ADDRESS          30
#define CONFIG_LEN_OPTION           30

#define STORAGE_NAMESPACE           "config"


typedef struct {
    char name[CONFIG_LEN_NAME];
    uint8_t type;
    char address[CONFIG_LEN_ADDRESS];
    char option[CONFIG_LEN_OPTION];
} profiles;


// Exported functions
esp_err_t configurationInit();
esp_err_t configurationList(char names[][CONFIG_LEN_NAME], size_t *count);


#endif
