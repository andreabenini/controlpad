set(SOURCES     "main.c" "keyboard.c")
set(REQUIRES    "")

idf_component_register(
    SRCS            "${SOURCES}"
    INCLUDE_DIRS    "."
)
#    REQUIRES        "${REQUIRES}"
