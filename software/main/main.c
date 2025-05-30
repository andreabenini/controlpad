// Program Includes
#include "main.h"
#include "mainloop.h"
#include "serial.h"


// System includes
#include <freertos/FreeRTOS.h>              // IWYU pragma: keep
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_system.h>
#include <string.h>


QueueHandle_t queueMessage;



void app_main(void) {
    ESP_LOGW(TAG_APP, "Program Started");
    queueMessage = xQueueCreate(1, sizeof(uint32_t));
    if (queueMessage == NULL) {
        ESP_LOGE(TAG_APP, "Cannot create the message queue");
        return;
    }
    // Create the I/O peripherals management task
    //      - taskFunction
    //      - task name for debugging
    //      - stack size
    //      - task parameters
    //      - task priority (lower number = lower priority)
    //      - task handle (optional)
    xTaskCreate(taskMainLoop, "taskLoop",   4096, NULL,  5, NULL);
    // Create serial task for receiving async incoming configurations from console (I've a 10K buffer for data stream in it)
    xTaskCreate(taskSerial,   "taskSerial", 4096, NULL, 12, NULL);
} /**/
