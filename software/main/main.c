// Program Includes
#include "main.h"
#include "mainloop.h"


// System includes
#include "freertos/FreeRTOS.h"              // IWYU pragma: keep
#include "freertos/task.h"
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>


void app_main(void) {
    ESP_LOGW(TAG_APP, "Program Started");
    // Create the I/O peripherals management task
    //      - taskFunction
    //      - task name for debugging
    //      - stack size
    //      - task parameters
    //      - task priority (lower number = lower priority)
    //      - task handle (optional)
    xTaskCreate(taskMainLoop, "taskLoop", 4096, NULL, 5, NULL);
} /**/
