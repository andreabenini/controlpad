#ifndef DISPLAY__H
#define DISPLAY__H

#include <stdint.h>
#include <string.h>                                             // IWYU pragma: keep
#include "driver/gpio.h"                                        // IWYU pragma: keep
#include "driver/spi_master.h"                                  // IWYU pragma: keep
#include <esp_err.h>
#include <esp_log.h>


#define TAG_DISPLAY             "display"


// Display specs (landscape mode)
#define LCD_WIDTH               160
#define LCD_HEIGHT              128

/** Colors on a 565 bit mask:   BBBBB GGGGGG RRRRR      [ BGR Schema ]
 *                              00000 000000 00000      0x0000 - Black
 *                              00000 000000 11111      0xF800 - Blue
 *                              11111 000000 00000      0x001F - Red
 */
#define LCD_COLOR_BLACK         0x0000
#define LCD_COLOR_BLUE          0xF800
#define LCD_COLOR_RED           0x001F
#define LCD_COLOR_GREEN         0x07E0
#define LCD_COLOR_CYAN          0xFFE0
#define LCD_COLOR_MAGENTA       0xF81F
#define LCD_COLOR_YELLOW        0x07FF
#define LCD_COLOR_WHITE         0xFFFF

// GPIO pin assignments
#define LCD_PIN_SCK             GPIO_NUM_4
#define LCD_PIN_MOSI            GPIO_NUM_6
#define LCD_PIN_CS              GPIO_NUM_7
#define LCD_PIN_A0              GPIO_NUM_5                      // MISO is not used for write-only, using A0 instead
#define LCD_PIN_RESET           GPIO_NUM_10

// ST7735 commands (these might need to be verified for your specific controller)
#define LCD_CMD_NOP             0x00
#define LCD_CMD_SWRESET         0x01
#define LCD_CMD_SLPIN           0x10
#define LCD_CMD_SLPOUT          0x11
#define LCD_CMD_PTLON           0x12
#define LCD_CMD_NORON           0x13
#define LCD_CMD_INVOFF          0x20
#define LCD_CMD_INVON           0x21
#define LCD_CMD_DISPOFF         0x28
#define LCD_CMD_DISPON          0x29
#define LCD_CMD_CASET           0x2A
#define LCD_CMD_RASET           0x2B
#define LCD_CMD_RAMWR           0x2C
#define LCD_CMD_COLMOD          0x3A
#define LCD_CMD_MADCTL          0x36
#define LCD_CMD_FRMCTR1         0xB1
#define LCD_CMD_INVCTR          0xB4
#define LCD_CMD_DISSET5         0xB6
#define LCD_CMD_PWCTR1          0xC0
#define LCD_CMD_PWCTR2          0xC1
#define LCD_CMD_PWCTR3          0xC2
#define LCD_CMD_VMCTR1          0xC5
#define LCD_CMD_GMCTRP1         0xE0
#define LCD_CMD_GMCTRN1         0xE1


#include "fonts.h"                                              // IWYU pragma: keep


// Exported functions
esp_err_t displayInit();
void displayBackground(uint16_t color);                                                 // Set Background
void displayRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color);      // Draw a filled rectangle
void displayText(uint8_t x, uint8_t y, const char *str, const Font *font, uint16_t color);
void displayTextBackground(uint8_t x, uint8_t y, const char *str, const Font *font, uint16_t colorForeground, uint16_t colorBackground);
void displayWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t colorForeground, uint16_t colorBackground, uint8_t borderWidth);
void displayBox(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

void displayTestPattern();
void displayTestText();
void menu(char *items, uint8_t itemsNumber, uint8_t itemsSize, uint8_t itemsPage, uint8_t itemsFirst, uint8_t itemSelected, uint8_t x, uint8_t y, const Font *font, uint16_t colorForeground, uint16_t colorBackground);


// Internal functions, don't call 'hem directly
void lcdInit_st7735();
void lcdSendCommand(const uint8_t cmd);
void lcdSendData(const uint8_t *data, int len);
void lcdSendDataByte(uint8_t data);
void lcdSendDataWord(uint16_t data);
void lcdWindowSet(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void lcdDisplayChar(uint16_t x, uint16_t y, char character, const Font *font, uint16_t color);
void lcdDisplayCharBackground(uint8_t x, uint8_t y, char character, const Font *font, uint16_t colorForeground, uint16_t colorBackground);


#endif
