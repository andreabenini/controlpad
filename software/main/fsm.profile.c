#include "fsm.profile.h"
#include "fsm.h"
#include "configuration.h"
#include "mainloop.h"
#include "display.h"


extern keyboardStatus keyboard;

typedef struct {
    uint8_t profileNumber;
} objProfile;
objProfile userProfile;


void statusProfileInit() {
    ESP_LOGI(TAG_PROFILE, "Entering in user profile status");
    displayBackground(LCD_COLOR_BLACK);
} /**/

void statusProfileInitProfile(uint8_t number) {
    // Load required configuration
    userProfile.profileNumber = number;
    profiles configuration;
    char label[CONFIG_LEN_NAME+3];
    configurationLoad(&configuration, userProfile.profileNumber);
    // Display useful information to the user
    sprintf(label, " %-19s", configuration.name);
    displayTextBackground(0, 0, label, &font8x16, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
    memcpy(label, configuration.remote, CONFIG_LEN_NAME);
    label[CONFIG_LEN_NAME] = '\0';
    displayTextBackground(0, 16, label, &font8x12, LCD_COLOR_WHITE, LCD_COLOR_BLACK);

    displayBox(0, 116, LCD_WIDTH, LCD_HEIGHT, LCD_COLOR_WHITE);
    char type[5];
    switch (configuration.type) {
        case TYPE_TCP:
            strcpy(type, "TCP");
            break;
        case TYPE_UDP:
            strcpy(type, "UDP");
            break;
        case TYPE_HTTP:
            strcpy(type, "HTTP");
            break;
        default:
            strcpy(type, "????");
    }
    displayText(LCD_WIDTH-(strlen(type)+1)*8, LCD_HEIGHT-12, type, &font8x12, LCD_COLOR_BLACK);
    // Network connection
    if (configuration.type <= TYPE_HTTP) {              // TCP, UDP, HTTP
        displayTextBackground(10, LCD_HEIGHT-12*2-2, "connecting...", &font8x12, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
        if (wifiConnect(configuration.username, configuration.password, label) != ESP_OK) {
            statusProfileInitError(" WIFI  ERROR ",
                                   " CONNECTION  ",
                                   "   FAILED    ");
            return;
        }
        displayTextBackground(30, 55, " CONNECTED  ", &font8x16, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
        displayTextBackground(10, LCD_HEIGHT-12*2-2, label, &font8x12, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
        displayText(10, LCD_HEIGHT-12,     "IP",      &font8x12, LCD_COLOR_BLACK);
    }
} /**/


/**
 * Init error message Box
 */
void statusProfileInitError(char *line1, char *line2, char *line3) {
    // Waiting window
    displayBackground(LCD_COLOR_RED);
    displayWindow(LCD_WIDTH/10, LCD_HEIGHT/4, LCD_WIDTH/10*8, LCD_HEIGHT/2, LCD_COLOR_BLACK, LCD_COLOR_WHITE, 2);
    displayTextBackground(26, 40, line1, &font8x12, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
    displayTextBackground(26, 55, line2, &font8x12, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
    displayTextBackground(26, 70, line3, &font8x12, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
    vTaskDelay(pdMS_TO_TICKS(5000));
    statusChange(STATUS_MENU);
} /**/


void statusProfileEvent() {
    // ESP_LOGI(TAG_PROFILE, "I2C "                    // FIXME: Convert it to a Display function
    //     "A[%d%d%d%d%d%d%d%d] "
    //     "B[%d%d%d%d%d%d%d%d]  "
    //     "[%4d,%4d] [%4d,%4d]",
    //     keyboard.button[0].stateCurrent,        // A0..7, B0..7 I2C buttons
    //     keyboard.button[1].stateCurrent,
    //     keyboard.button[2].stateCurrent,
    //     keyboard.button[3].stateCurrent,
    //     keyboard.button[4].stateCurrent,
    //     keyboard.button[5].stateCurrent,
    //     keyboard.button[6].stateCurrent,
    //     keyboard.button[7].stateCurrent,
    //     keyboard.button[8].stateCurrent,
    //     keyboard.button[9].stateCurrent,
    //     keyboard.button[10].stateCurrent,
    //     keyboard.button[11].stateCurrent,
    //     keyboard.button[12].stateCurrent,
    //     keyboard.button[13].stateCurrent,
    //     keyboard.button[14].stateCurrent,
    //     keyboard.button[15].stateCurrent,
    //     keyboard.joystick1_X, keyboard.joystick1_Y,
    //     keyboard.joystick2_X, keyboard.joystick2_Y
    // );
} /**/

/**
 * USER PROFILE keypress event, return to menu when START/SYSTEM button is pressed
 */
void statusProfileKeypress(uint8_t button, bool status) {
    if (button==B_START && status==1) {
        ESP_LOGI(TAG_PROFILE, "Closing user profile status, returning to main menu");
        wifiDisconnect();
        statusChange(STATUS_MENU);
    }
} /**/



/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

typedef uint8_t byte_t;

byte_t* parseCStringToBinary(const char* input, size_t* output_size) {
    size_t input_len = strlen(input);
    byte_t* result = NULL;
    size_t result_size = 0;
    size_t result_capacity = 0;

    for (size_t i = 0; i < input_len; ++i) {
        if (input[i] == '\\' && i + 1 < input_len) {
            i++; // Move past the backslash
            byte_t byte_value;
            switch (input[i]) {
                case 'n':
                    byte_value = 0x0A; // Newline
                    break;
                case 't':
                    byte_value = 0x09; // Tab
                    break;
                case 'r':
                    byte_value = 0x0D; // Carriage return
                    break;
                case 'b':
                    byte_value = 0x08; // Backspace
                    break;
                case 'f':
                    byte_value = 0x0C; // Form feed
                    break;
                case '\\':
                    byte_value = '\\'; // Literal backslash
                    break;
                case '\'':
                    byte_value = '\''; // Single quote
                    break;
                case '\"':
                    byte_value = '\"'; // Double quote
                    break;
                case '0':
                    byte_value = 0x00; // Null terminator
                    break;
                case 'x':
                    // Handle hexadecimal escape sequence (\xhh)
                    if (i + 2 < input_len && isxdigit((unsigned char)input[i + 1]) && isxdigit((unsigned char)input[i + 2])) {
                        char hex_str[3];
                        strncpy(hex_str, &input[i + 1], 2);
                        hex_str[2] = '\0';
                        byte_value = (byte_t)strtol(hex_str, NULL, 16);
                        i += 2; // Move past the two hex digits
                    } else {
                        // Invalid hex escape, treat backslash and 'x' literally
                        if (result_size >= result_capacity) {
                            result_capacity = (result_capacity == 0) ? 1 : result_capacity * 2;
                            byte_t* temp = (byte_t*)realloc(result, result_capacity * sizeof(byte_t));
                            if (temp == NULL) {
                                free(result);
                                return NULL; // Memory allocation failed
                            }
                            result = temp;
                        }
                        result[result_size++] = '\\';
                        byte_value = 'x';
                    }
                    break;
                default:
                    // Unrecognized escape sequence, treat backslash and the character literally
                    if (result_size >= result_capacity) {
                        result_capacity = (result_capacity == 0) ? 1 : result_capacity * 2;
                        byte_t* temp = (byte_t*)realloc(result, result_capacity * sizeof(byte_t));
                        if (temp == NULL) {
                            free(result);
                            return NULL; // Memory allocation failed
                        }
                        result = temp;
                    }
                    result[result_size++] = '\\';
                    byte_value = input[i];
                    break;
            }
            if (input[i] != 'x' || (input[i] == 'x' && !(i - 2 >= 0 && input[i - 2] == '\\' && i + 1 < input_len && isxdigit((unsigned char)input[i + 1]) && isxdigit((unsigned char)input[i + 2])))) {
                if (result_size >= result_capacity) {
                    result_capacity = (result_capacity == 0) ? 1 : result_capacity * 2;
                    byte_t* temp = (byte_t*)realloc(result, result_capacity * sizeof(byte_t));
                    if (temp == NULL) {
                        free(result);
                        return NULL; // Memory allocation failed
                    }
                    result = temp;
                }
                result[result_size++] = byte_value;
            }
        } else {
            // Regular character, just add its ASCII value
            if (result_size >= result_capacity) {
                result_capacity = (result_capacity == 0) ? 1 : result_capacity * 2;
                byte_t* temp = (byte_t*)realloc(result, result_capacity * sizeof(byte_t));
                if (temp == NULL) {
                    free(result);
                    return NULL; // Memory allocation failed
                }
                result = temp;
            }
            result[result_size++] = (byte_t)input[i];
        }
    }

    // Resize the result array to the actual size
    byte_t* final_result = (byte_t*)realloc(result, result_size * sizeof(byte_t));
    if (final_result == NULL && result_size > 0) {
        free(result);
        return NULL; // Reallocation failed
    }
    result = final_result;
    *output_size = result_size;
    return result;
}

int main() {
    const char* cString = "abc\\n123\\t\\\\mAB \\x31\\x32\\x33";
    byte_t* binaryData = NULL;
    size_t binaryDataSize = 0;

    binaryData = parseCStringToBinary(cString, &binaryDataSize);

    if (binaryData != NULL) {
        printf("Original C String: %s\n", cString);
        printf("Binary Data (hex): ");
        for (size_t i = 0; i < binaryDataSize; ++i) {
            printf("%02X ", binaryData[i]);
        }
        printf("\n");
        free(binaryData); // Don't forget to free the allocated memory
    } else {
        fprintf(stderr, "Error parsing C string or allocating memory.\n");
    }

    return 0;
}
*/
