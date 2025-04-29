# set(SOURCES     "main.c" "mainloop.c" "i2c.c" "joystick.c" "fsm.c" "fsm.profile.c" "display.c" "fonts.c" "configuration.c" "serial.c" "cJSON.c")
# set(SOURCES     "main.c")
set(SOURCES     "main.c" "mainloop.c" "i2c.c" "joystick.c" "fsm.c" "fsm.profile.c" "display.c" "fonts.c" "configuration.c" "serial.c" "cJSON.c")

set(REQUIRES    nvs_flash esp_adc driver)

idf_component_register(
    SRCS            "${SOURCES}"
    INCLUDE_DIRS    "."
    REQUIRES        "${REQUIRES}"
)
