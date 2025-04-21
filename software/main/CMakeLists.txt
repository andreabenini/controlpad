# set(SOURCES     "main.c" "mainloop.c" "i2c.c" "joystick.c" "fsm.c" "display.c")
# set(SOURCES     "main.c" "mainloop.c" "i2c.c" "joystick.c" "fsm.c" "display.c" "fonts.c")
# set(REQUIRES    "main.c")
set(SOURCES     "main.c" "mainloop.c" "i2c.c" "joystick.c" "fsm.c" "display.c" "fonts.c")

idf_component_register(
    SRCS            "${SOURCES}"
    INCLUDE_DIRS    "."
)
#    REQUIRES        "${REQUIRES}"
