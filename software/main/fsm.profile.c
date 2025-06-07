#include "fsm.profile.h"
#include "mainloop.h"
#include "display.h"


extern keyboardStatus keyboard;


/**
 * (constructor) Create object profile
 */
void* profileCreate(void* objects) {
    objectProfile* self = (objectProfile*)malloc(sizeof(objectProfile));
    if (self != NULL) {
        self->base.type        = STATUS_PROFILE;
        self->base.init        = profileInit;
        self->base.eventStatus = profileEventStatus;
        self->base.eventButton = profileEventButton;
        self->profileNumber    = UINT8_MAX;
        self->displayTitle     = (void*)profileDisplayTitle;
        self->displayStatusBar = (void*)profileDisplayStatusBar;
        self->displayError     = (void*)profileDisplayError;
        self->connect          = (void*)profileConnect;
        self->disconnect       = (void*)profileDisconnect;
        self->keySend          = (void*)profileKeySend;
    }
    return self;
} /**/


void profileInit(void* object) {
    objectProfile* self = object;
    // Loading profile
    ESP_LOGI(TAG_PROFILE, "Loading profile [%d]", self->profileNumber);
    if (self->profileNumber == UINT8_MAX || configurationLoad(&(self->configuration), self->profileNumber) != ESP_OK) {
        ESP_LOGE(TAG_PROFILE, "Cannot load user profile N:%d", self->profileNumber);
        statusChange(STATUS_MENU);
        return;
    }
    ESP_LOGI(TAG_PROFILE, "    - '%s', connection type:%d", self->configuration.name, self->configuration.type);

    // Display information on screen
    self->displayTitle(self);
    self->displayStatusBar(self);
    // Peer connection and display information
    if (self->connect(self) != ESP_OK) {
        statusChange(STATUS_MENU);
        return;
    }
}

void profileDisplayTitle(objectProfile* self) {
    char line[CONFIG_LEN_NAME+3];
    displayBackground(LCD_COLOR_BLACK);
    sprintf(line, " %-19s", self->configuration.name);              // Profile name
    displayTextBackground(0, 0, line, &font8x16, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
    memcpy(line, self->configuration.remote, CONFIG_LEN_NAME);      // Remote host address
    line[CONFIG_LEN_NAME] = '\0';
    displayTextBackground(0, 16, line, &font8x12, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
    displayBox(0, 116, LCD_WIDTH, LCD_HEIGHT, LCD_COLOR_WHITE);
}

void profileDisplayStatusBar(objectProfile* self) {
    char type[5];
    switch (self->configuration.type) {
        case TYPE_TCP:
            strcpy(type, "TCP");
            break;
        case TYPE_UDP:
            strcpy(type, "UDP");
            break;
        case TYPE_HTTP:
            strcpy(type, "HTTP");
            break;
        case TYPE_BT:
            strcpy(type, "BT");
            break;
        case TYPE_BTLE:
            strcpy(type, "BTLE");
            break;
        default:
            strcpy(type, "????");
    }
    displayText(LCD_WIDTH-(strlen(type)+1)*8, LCD_HEIGHT-12, type, &font8x12, LCD_COLOR_BLACK);
}

/**
 * display message box error
 */
void profileDisplayError(objectProfile* self, char *line1, char *line2, char *line3) {
    // Waiting window
    displayBackground(LCD_COLOR_RED);
    displayWindow(LCD_WIDTH/10, LCD_HEIGHT/4, LCD_WIDTH/10*8, LCD_HEIGHT/2, LCD_COLOR_BLACK, LCD_COLOR_WHITE, 2);
    displayTextBackground(26, 40, line1, &font8x12, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
    displayTextBackground(26, 55, line2, &font8x12, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
    displayTextBackground(26, 70, line3, &font8x12, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
    vTaskDelay(pdMS_TO_TICKS(5000));
} /**/


/**
 * Connect to remote device, configuration dependant
 */
esp_err_t profileConnect(objectProfile* self) {
    displayTextBackground(10, LCD_HEIGHT-12*2-2, "connecting...", &font8x12, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
    self->socket = -1;
    if (self->configuration.type <= TYPE_HTTP) {                // TCP, UDP, HTTP
        // WiFi connection
        displayText(10, LCD_HEIGHT-12, "IP", &font8x12, LCD_COLOR_BLACK);
        if (wifiConnect(self->configuration.username, self->configuration.password, self->configuration.local) != ESP_OK) {
            self->disconnect(self);
            self->displayError(self,
                               " WIFI ERROR, ",
                               " CONNECTION  ",
                               "   FAILED    ");
            return ESP_ERR_WIFI_NOT_INIT;
        }
        displayTextBackground(10, LCD_HEIGHT-12*2-2, self->configuration.local, &font8x12, LCD_COLOR_WHITE, LCD_COLOR_BLACK);

        // Create TCP socket to remote
        self->socket = tcpSocket(self->configuration.remote);
        if (self->socket < 0) {
            self->disconnect(self);
            self->displayError(self,
                               "   CANNOT    ",
                               " CONNECT TO  ",
                               " REMOTE HOST ");
            return ESP_ERR_WIFI_NOT_CONNECT;
        }
        displayTextBackground(10, 55, "C O N N E C T E D", &font8x16, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
        // displayTextBackground(10, 55, "  DISCONNECTED ! ", &font8x16, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
        return ESP_OK;
    }
    self->displayError(self,
            " CONNECTION  ",
            "TYPE NOT YET ",
            " IMPLEMENTED ");
    return ESP_ERR_NOT_SUPPORTED;
} /**/


/**
 * Network disconnection
 */
esp_err_t profileDisconnect(objectProfile* self) {
    wifiDisconnect();
    displayTextBackground(10, 55, "                 ", &font8x16, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
    return ESP_OK;
} /**/



void profileEventStatus(void* object) {
} /**/

/**
 * USER PROFILE keypress event, return to menu when START/SYSTEM button is pressed
 */
void profileEventButton(void* object, uint8_t button, bool status) {
    objectProfile* self = object;
    ESP_LOGI(TAG_PROFILE, "eventButton() Button: %d, status: %d", button, status);    // DEBUG:
    if (status==1) {
        if (button==B_START) {
            ESP_LOGI(TAG_PROFILE, "Closing user profile status, returning to main menu");
            self->disconnect(self);
            statusChange(STATUS_MENU);
        } else {
            self->keySend(self, button);
        }
    }
} /**/

/**
 * Send a key to remote
 */
esp_err_t profileKeySend(objectProfile *self, uint8_t button) {
    ESP_LOGW(TAG_PROFILE, "keySend() Button: %d", button);
    tcpDataSend(self->socket, "Hello\n");
    return ESP_OK;
} /**/
