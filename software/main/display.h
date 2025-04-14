#ifndef DISPLAY__H
#define DISPLAY__H

#include <stdint.h>
#include <driver/gpio.h>


// Define your GPIO pin assignments
#define LCD_SCK_PIN   GPIO_NUM_4
#define LCD_MOSI_PIN  GPIO_NUM_6
#define LCD_CS_PIN    GPIO_NUM_7
#define LCD_A0_PIN    GPIO_NUM_5
#define LCD_RESET_PIN GPIO_NUM_10



void taskDisplay(void *pvParameter);


#define TAG_DISPLAY     "display"

#endif
