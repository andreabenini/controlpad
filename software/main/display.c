#include "display.h"

// System includes
#include "freertos/FreeRTOS.h"
#include <freertos/task.h>
#include <esp_err.h>
#include <esp_log.h>




/**
 * Main task function for keyboard management
 */
void taskDisplay(void *pvParameter) {
    ESP_LOGI(TAG_DISPLAY, "Task created");
    vTaskDelete(NULL);                                  // Delete the task, without this I'm getting a guru meditation error with core0 in panic mode
} /**/
