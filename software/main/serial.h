#ifndef SERIAL__H
#define SERIAL__H

#include <stdint.h>
#include <string.h>                                             // IWYU pragma: keep
#include <stdbool.h>
#include "freertos/FreeRTOS.h"                                  // IWYU pragma: keep
#include "driver/uart.h"                                        // IWYU pragma: keep
#include "driver/usb_serial_jtag.h"                             // IWYU pragma: keep

#include "cJSON.h"                                              // IWYU pragma: keep
#include "configuration.h"                                      // IWYU pragma: keep


#define BUFFER_READ             256
#define BUFFER_SIZE             4096                            // Configuration stream buffer
#define PATTERN_LENGTH          3                               // Length of JSON start pattern (e.g., "{")
#define TAG_SERIAL              "serial"
#define PATTERN_BEGIN           "#CONFIG#BEGIN#"
#define PATTERN_END             "#CONFIG#END#"


void taskSerial(void *pvParameters);
void loadConfiguration(char *config);
void _loadMapping(cJSON* buttonList, profiles* configurations, size_t current);

#endif
