// Program Includes
#include "main.h"
#include "keyboard.h"


// System includes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>


void app_main(void) {
    ESP_LOGW(TAG_APP, "Program Started");
    // Create the I/O peripherals management task
    xTaskCreate(taskKeyboard, "taskKeyboard", 4096, NULL, 5, NULL);
    return;
} /**/
