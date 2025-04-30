#include "display.h"
#include "esp_log.h"
#include "fonts.h"
#include <stdint.h>
#include <string.h>


spi_device_handle_t spi;


/**
 * Send command to the LCD
 */
void lcdSendCommand(const uint8_t cmd) {
    esp_err_t ret;
    spi_transaction_t t;
    gpio_set_level(LCD_PIN_A0, 0);              // Command mode (A0/DC pin low)
    memset(&t, 0, sizeof(t));
    t.length = 8;                               // 8 bits command
    t.tx_buffer = &cmd;                         // Data (the command itself)
    t.user = (void*)0;                          // D/C needs to be set to low for command
    ret = spi_device_polling_transmit(spi, &t);
    ESP_ERROR_CHECK(ret);
} /**/


/**
 * Send data to the LCD
 */
void lcdSendData(const uint8_t *data, int len) {
    if (len == 0) return;
    esp_err_t ret;
    spi_transaction_t t;

    gpio_set_level(LCD_PIN_A0, 1);              // Data mode (A0/DC pin high)
    memset(&t, 0, sizeof(t));
    t.length = len * 8;                         // Len is in bytes, transaction length is in bits
    t.tx_buffer = data;                         // Data
    t.user = (void*)1;                          // D/C needs to be high for data
    ret = spi_device_polling_transmit(spi, &t);
    ESP_ERROR_CHECK(ret);
} /**/

/**
 * Send a single byte of data
 */
void lcdSendDataByte(uint8_t data) {
    lcdSendData(&data, 1);
} /**/

/**
 * Send a 16-bit value as data
 */
void lcdSendDataWord(uint16_t data) {
    uint8_t buf[2] = {(data >> 8) & 0xFF, data & 0xFF};
    lcdSendData(buf, 2);
} /**/


/**
 * Set the display address window
 */
void lcdWindowSet(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    lcdSendCommand(LCD_CMD_CASET);
    lcdSendDataByte(0x00);
    lcdSendDataByte(x0);
    lcdSendDataByte(0x00);
    lcdSendDataByte(x1);
    lcdSendCommand(LCD_CMD_RASET);
    lcdSendDataByte(0x00);
    lcdSendDataByte(y0);
    lcdSendDataByte(0x00);
    lcdSendDataByte(y1);
    lcdSendCommand(LCD_CMD_RAMWR);
} /**/


/**
 * Initialize the ST7735 display
 */
void lcdInit_st7735() {
    ESP_LOGI(TAG_DISPLAY, "    - Initialize ST7735 display (command mode)");
    // Reset the display
    gpio_set_level(LCD_PIN_RESET, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(LCD_PIN_RESET, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    // Software reset
    lcdSendCommand(LCD_CMD_SWRESET);
    vTaskDelay(pdMS_TO_TICKS(150));
    // Out of sleep mode
    lcdSendCommand(LCD_CMD_SLPOUT);
    vTaskDelay(pdMS_TO_TICKS(120));
    // Set color mode to 16-bit color (5-6-5)
    lcdSendCommand(LCD_CMD_COLMOD);
    lcdSendDataByte(0x05);
    vTaskDelay(pdMS_TO_TICKS(10));
    // Frame rate control
    lcdSendCommand(LCD_CMD_FRMCTR1);
    lcdSendDataByte(0x01);
    lcdSendDataByte(0x2C);
    lcdSendDataByte(0x2D);
    // Set rotation to landscape mode
    lcdSendCommand(LCD_CMD_MADCTL);
    lcdSendDataByte(0xA8);                      // MX=1, MY=0, MV=1, BGR=1: Landscape mode (90 degrees)
    // Inversion control, invert colors [LCD_CMD_INVON, LCD_CMD_INVOFF]
    // lcdSendCommand(LCD_CMD_INVOFF);
    // Power control
    lcdSendCommand(LCD_CMD_PWCTR1);
    lcdSendDataByte(0xA2);
    lcdSendDataByte(0x02);
    lcdSendDataByte(0x84);
    lcdSendCommand(LCD_CMD_PWCTR2);
    lcdSendDataByte(0xC5);
    // VCOM control
    lcdSendCommand(LCD_CMD_VMCTR1);
    lcdSendDataByte(0x0A);
    lcdSendDataByte(0x00);
    // Gamma adjustment positive
    lcdSendCommand(LCD_CMD_GMCTRP1);
    uint8_t gamma_pos[] = {0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00};
    lcdSendData(gamma_pos, sizeof(gamma_pos));
    // Gamma adjustment negative
    lcdSendCommand(LCD_CMD_GMCTRN1);
    uint8_t gamma_neg[] = {0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F};
    lcdSendData(gamma_neg, sizeof(gamma_neg));
    // Normal display mode on
    lcdSendCommand(LCD_CMD_NORON);
    vTaskDelay(pdMS_TO_TICKS(10));
    // Display on
    lcdSendCommand(LCD_CMD_DISPON);
    vTaskDelay(pdMS_TO_TICKS(100));
    ESP_LOGI(TAG_DISPLAY, "    - Initialized in landscape mode (%dx%d)", LCD_WIDTH, LCD_HEIGHT);
}

/**
 * Init the display and its related hardware 
 */
esp_err_t displayInit() {
    // Configure GPIO pins
    ESP_LOGI(TAG_DISPLAY, "displayInit()");
    ESP_LOGI(TAG_DISPLAY, "    - Configure GPIO");
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LCD_PIN_A0) | (1ULL << LCD_PIN_RESET),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // Configure SPI bus
    ESP_LOGI(TAG_DISPLAY, "    - Configure SPI bus");
    spi_bus_config_t buscfg = {
        .miso_io_num = -1,                          // MISO not used (write-only)
        .mosi_io_num = LCD_PIN_MOSI,
        .sclk_io_num = LCD_PIN_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 16 * 320 * 2,            // 16 lines of max width (320) in 16-bit colors
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // Configuring SPI device
    ESP_LOGI(TAG_DISPLAY, "    - Configure SPI device");
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 24 * 1000 * 1000,         // 24 MHz clock frequency (adjust as needed)
        .mode = 0,                                  // SPI mode 0
        .spics_io_num = LCD_PIN_CS,                 // CS pin
        .queue_size = 7,                            // Queue 7 transactions at a time
        .flags = SPI_DEVICE_NO_DUMMY,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi));

    lcdInit_st7735();                               // Initialize display with custom commands
    ESP_LOGI(TAG_DISPLAY, "    Completed");
    return ESP_OK;
} /**/


/**
 * Fill the screen background with a color (RGB565 format)
 */
 void displayBackground(uint16_t color) {
    lcdWindowSet(0, 0, LCD_WIDTH-1, LCD_HEIGHT-1);
    for (unsigned int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        lcdSendDataWord(color);
    }
} /**/


/**
 * Draw a filled rectangle
 */
void displayRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color) {
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) {
        return;
    }
    // Clip the rectangle to the screen boundaries
    if ((x + w - 1) >= LCD_WIDTH) {
        w = LCD_WIDTH - x;
    }
    if ((y + h - 1) >= LCD_HEIGHT) {
        h = LCD_HEIGHT - y;
    }
    lcdWindowSet(x, y, x+w-1, y+h-1);
    for (int i = 0; i < w * h; i++) {
        lcdSendDataWord(color);
    }
} /**/


/**
 * Display a character in the display
 * @see Do NOT use this function directly, use displayText() instead
 */
void lcdDisplayChar(uint16_t x, uint16_t y, char character, const Font *font, uint16_t color) {
    uint8_t pos = (character < ' ' || character > 127) ? '?' : character - ' ';
    for (uint8_t row=0; row < font->height; row++) {
        uint8_t line = font->data[pos][row];
        for (uint8_t col=0; col<font->width; col++) {
            // if (line & (0x80 >> col)) {  // Reversed
            if (line & (1 << col)) {        // Straight
                lcdWindowSet(x+col, y+row, x+col, y+row);
                lcdSendDataWord(color);
            }
        }
    }
} /**/


/**
 * Faster version of lcdDisplayChar(), setting lcdWindowSet() once and draw inside it is faster than redefining it each single time (but background must always be set)
 */
void lcdDisplayCharBackground(uint8_t x, uint8_t y, char character, const Font *font, uint16_t colorForeground, uint16_t colorBackground) {
    uint8_t pos = (character < ' ' || character > 127) ? '?' : character - ' ';
    lcdWindowSet(x, y, x+font->width-1, y+font->height-1);
    for (uint8_t row=0; row < font->height; row++) {
        uint8_t line = font->data[pos][row];
        for (uint8_t col=0; col<font->width; col++) {
            if (line & (1 << col)) {
                lcdSendDataWord(colorForeground);
            } else {
                lcdSendDataWord(colorBackground);
            }
        }
    }
} /**/


/**
 * Display some text on the screen
 * x     (uint8_t)      X position (top left)
 * y     (uint8_t)      Y position (top left)
 * str   (const char *) Text string to display
 * font  (Font)         Font to use for that string
 * color (uint8_t)      Foreground color for the string
 */
void displayText(uint8_t x, uint8_t y, const char *str, const Font *font, uint16_t color) {
    uint8_t cursorX = x;
    uint8_t cursorY = y;
    while (*str) {
        lcdDisplayChar(cursorX, cursorY, *str, font, color);
        if (cursorX + font->width * 2 > LCD_WIDTH) {
            cursorX = x;
            cursorY += font->height + 1;
            if (cursorY + font->height + 1 > LCD_HEIGHT) {
                break;
            }
        } else {
            cursorX += font->width + 1;
        }
        str++;
    }
} /**/
void displayTextBackground(uint8_t x, uint8_t y, const char *str, const Font *font, uint16_t colorForeground, uint16_t colorBackground) {
    uint8_t cursorX = x;
    uint8_t cursorY = y;
    while (*str) {
        lcdDisplayCharBackground(cursorX, cursorY, *str, font, colorForeground, colorBackground);
        if (cursorX + font->width * 2 > LCD_WIDTH) {
            cursorX = x;
            cursorY += font->height + 1;
            if (cursorY + font->height + 1 > LCD_HEIGHT) {
                break;
            }
        } else {
            cursorX += font->width;
        }
        str++;
    }
} /**/


/**
 * Display an empty window
 */
void displayWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t colorForeground, uint16_t colorBackground, uint8_t borderWidth) {
    // Border Top
    lcdWindowSet(x, y, x+width, y+borderWidth);
    for (unsigned int i=0; i < width*borderWidth*2; i++) {
        lcdSendDataWord(colorForeground);
    }
    // Border Bottom
    lcdWindowSet(x, y+height-borderWidth, x+width, y+height);
    for (unsigned int i=0; i < width*borderWidth*2; i++) {
        lcdSendDataWord(colorForeground);
    }
    // Border Left
    lcdWindowSet(x, y, x+borderWidth, y+height);
    for (unsigned int i=0; i < borderWidth*height*2; i++) {
        lcdSendDataWord(colorForeground);
    }
    // Border Right
    lcdWindowSet(x+width-borderWidth, y, x+width, y+height);
    for (unsigned int i=0; i < borderWidth*height*2; i++) {
        lcdSendDataWord(colorForeground);
    }
    // Inner window
    displayBox(x+borderWidth, y+borderWidth, x+width-borderWidth, y+height-borderWidth, colorBackground);
    // lcdWindowSet(x+borderWidth, y+borderWidth, x+width-borderWidth, y+height-borderWidth);
    // for (unsigned int i=0; i < (width-borderWidth)*(height-borderWidth); i++) {
    //     lcdSendDataWord(colorBackground);
    // }
} /**/

/**
 * Just fill a standard box
 */
void displayBox(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
    lcdWindowSet(x1, y1, x2, y2);
    for (unsigned int i=0; i < (x2-x1)*(y2-y1); i++) {
        lcdSendDataWord(color);
    }
} /**/

/**
 * Display a menu
 */
void menu(char *items, uint8_t itemsNumber, uint8_t itemsSize, uint8_t itemsPage, uint8_t itemsFirst, uint8_t itemSelected, uint8_t x, uint8_t y, const Font *font, uint16_t colorForeground, uint16_t colorBackground) {
    char line[itemsSize+2];
    for (uint8_t i=0; i<itemsPage && i+itemsFirst<itemsNumber; i++) {
        sprintf(line, " %-*.*s ", itemsSize-1, itemsSize-1, items+((i+itemsFirst) * itemsSize));
        if (line[0] != '\0') {
            if (itemsFirst+i == itemSelected) {
                displayTextBackground(x, y, line, font, colorBackground, colorForeground); 
            } else {
                displayTextBackground(x, y, line, font, colorForeground, colorBackground); 
            }
        }
        y += font->height;
    }
} /**/


/**
 * Draw colored rectangles
 */
void displayTestPattern() {
    ESP_LOGI(TAG_DISPLAY, "Drawing test pattern");
    displayRectangle(0, 0, LCD_WIDTH/2, LCD_HEIGHT/4, LCD_COLOR_RED);
    displayRectangle(LCD_WIDTH/2, 0, LCD_WIDTH/2, LCD_HEIGHT/4, LCD_COLOR_GREEN);
    displayRectangle(0, LCD_HEIGHT/4, LCD_WIDTH/2, LCD_HEIGHT/4, LCD_COLOR_BLUE);
    displayRectangle(LCD_WIDTH/2, LCD_HEIGHT/4, LCD_WIDTH/2, LCD_HEIGHT/4, LCD_COLOR_YELLOW);
    displayRectangle(0, LCD_HEIGHT/2, LCD_WIDTH/2, LCD_HEIGHT/4, LCD_COLOR_MAGENTA);
    displayRectangle(LCD_WIDTH/2, LCD_HEIGHT/2, LCD_WIDTH/2, LCD_HEIGHT/4, LCD_COLOR_CYAN);
    displayRectangle(0, 3*LCD_HEIGHT/4, LCD_WIDTH/2, LCD_HEIGHT/4, LCD_COLOR_BLACK);
    displayRectangle(LCD_WIDTH/2, 3*LCD_HEIGHT/4, LCD_WIDTH/2, LCD_HEIGHT/4, LCD_COLOR_WHITE);
    ESP_LOGI(TAG_DISPLAY, "    - test pattern completed");
} /**/

void displayTestText() {
    // displayText(10, 50, "H", &font8x12, LCD_COLOR_WHITE);
    // displayText(10, 50, "Hello World", &font8x12, LCD_COLOR_WHITE);
    // displayText(10, 10, " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", &font8x12, LCD_COLOR_WHITE);
    displayText(10, 10, " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", &font8x14, LCD_COLOR_WHITE);

    displayTextBackground(10, 45, "Text&Background", &font8x12, LCD_COLOR_WHITE, LCD_COLOR_BLUE);
} /**/
