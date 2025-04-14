// Program Includes
#include "main.h"
#include "display.h"
#include "keyboard.h"
#include "communication.h"


// System includes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>


void app_main(void) {
    ESP_LOGW(TAG_APP, "Program Started");
    // Create the I/O peripherals management task
    // xTaskCreate(taskDisplay,        "taskDisplay",          2048, NULL, 5, NULL);
    xTaskCreate(taskKeyboard,       "taskKeyboard",         4096, NULL, 5, NULL);
    // xTaskCreate(taskCommunication,  "taskCommunication",    2048, NULL, 5, NULL);
    return;
} /**/
