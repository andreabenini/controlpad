#include "fsm.menu.h"
#include "mainloop.h"

extern keyboardStatus keyboard;


/**
 * (constructor) Create object menu
 */
void* menuCreate(void* objects) {
    objectMenu* self = (objectMenu*)malloc(sizeof(objectMenu));
    if (self != NULL) {
        self->base.type        = STATUS_MENU;
        self->base.init        = menuInit;
        self->base.eventStatus = menuEventStatus;
        self->base.eventButton = menuEventButton;
        self->show             = (void*)menuShow;
        self->parent           = (void*)objects;
    }
    return self;
} /**/

void menuInit(void* object) {
    objectMenu* self = object;
    self->lockEvent = true;
    ESP_LOGI(TAG_STATUS_MENU, "Menu Init");
    memset(self->connections, 0x00, sizeof(self->connections));
    configurationList(self->connections, &self->connectionsNumber);
    ESP_LOGI(TAG_STATUS_MENU, "Found %d profiles in the list", self->connectionsNumber);

    // Adding [configuration menu] to the list
    memcpy(self->connections[self->connectionsNumber], MENU_CONFIGURATION, strlen(MENU_CONFIGURATION));
    self->connections[self->connectionsNumber][strlen(MENU_CONFIGURATION)] = '\0';
    self->connectionsNumber++;

    // Display some context
    displayBackground(LCD_COLOR_WHITE);
    displayTextBackground(18, 0, "SELECT  PROFILE", &font8x12, LCD_COLOR_RED, LCD_COLOR_WHITE);
    self->first =
    self->selected = 0;
    self->page = 8;
    self->show(self);
    self->lockEvent = false;
} /**/

/**
 * Display user main menu with current item highlighted
 */
void menuShow(objectMenu* self) {
    menu((char*) self->connections,                         // MenuList
         self->connectionsNumber,                           // MenuItems,
         CONFIG_LEN_NAME,                                   // ItemLength
         self->page,                                        // Items per page
         self->first,                                       // First visible item in the menu
         self->selected,                                    // Selected Item
         0, 16,                                             // X,Y
         &font8x14, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
} /**/

void menuEventStatus(void* object) {
    objectMenu* self = object;
    if (self->lockEvent) {
        return;
    }
    if (MENU_UP == 1) {
        self->lockEvent = true;
        if (self->selected <= 0) {
            self->selected = self->connectionsNumber-1;
            self->first    = self->selected-self->page < 0? 0: self->selected-self->page+1;
         } else {
            self->selected -= 1;
            self->first = self->first<=self->selected? self->first: self->first-1;
         }
         self->show(self);
    } else if (MENU_DOWN == 1) {
        self->lockEvent = true;
        if (self->selected >= self->connectionsNumber-1) {
            self->selected = 0;
            self->first    = 0;
        } else {
            self->selected = self->selected+1;
            self->first    = self->selected-self->page < self->first? self->first: self->first+1;
        }
        self->show(self);
    } else if (MENU_CONFIRM == 1) {
        self->lockEvent = true;
        if (self->selected == self->connectionsNumber-1) {
            statusChange(STATUS_SYSTEM);
        } else {
            objectProfile* profile = (objectProfile*)(((void**)self->parent)[STATUS_PROFILE]);
            profile->profileNumber = self->selected;
            statusChange(STATUS_PROFILE);
        }
    } else if (MENU_CANCEL == 1) {
        self->lockEvent = true;
        statusChange(STATUS_MENU);
    }
} /**/

void menuEventButton(void* object, uint8_t button, bool status) {
    if (MENU_UP == 0 || MENU_DOWN == 0 || MENU_CONFIRM == 0 || MENU_CANCEL == 0) {
        ((objectMenu*)object)->lockEvent = false;
    }
} /**/
