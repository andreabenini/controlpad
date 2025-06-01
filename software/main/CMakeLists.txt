# set(SOURCES     "main.c" "mainloop.c" "i2c.c" "joystick.c" "fsm.c" "fsm.menu.c" "fsm.profile.c" "fsm.configuration.c" "display.c" "fonts.c" "configuration.c" "serial.c" "cJSON.c" "wifi.c")
# set(SOURCES     "main.c")
set(SOURCES     "main.c" "mainloop.c" "i2c.c" "joystick.c" "fsm.c" "fsm.menu.c" "fsm.profile.c" "fsm.configuration.c" "display.c" "fonts.c" "configuration.c" "serial.c" "cJSON.c" "wifi.c")

set(REQUIRES    nvs_flash esp_adc esp_wifi driver)

idf_component_register(
    SRCS            "${SOURCES}"
    INCLUDE_DIRS    "."
    REQUIRES        "${REQUIRES}"
)
