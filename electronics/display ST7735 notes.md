## Display pins

1. **LED** Backlight control pin. Typically, applying a voltage (often the same as VCC)
    turns on the backlight. You might need a current-limiting resistor.
2. **SCK** Serial Clock. Clock signal for SPI communication, generated by your microcontroller.
3. **SDA** Serial Data In / Master Out Slave In (MOSI). Data line for SPI communication
    (data sent from your microcontroller to the LCD).
4. **A0** Data/Command Select (also often labeled DC). Controls whether the data sent is a 
    command or pixel data. High for data, low for command.
5. **RESET** Reset pin. Active low. Used to hardware reset the LCD.
6. **CS** Chip Select. Active low. Used to enable communication with the LCD.
7. **GND** Ground pin. Connect to the ground of your microcontroller.
8. **VCC** Power supply pin. Typically accepts 3.3V or 5V. Crucially, check your module's
    specifications for the correct voltage.

## General considerations
Key Differences from the Generic Pinout:
- LED is the first pin: This confirms the presence of a dedicated backlight control pin.
- Order of SPI and Control Pins: The order of SCK, SDA, A0, RESET, and CS is different from the
    more generic example. You must follow this specific order when connecting to your microcontroller.
- GND and VCC at the end: This is just the physical arrangement of the pins on your module.

Next Steps with this Specific Pinout:
- Backlight Control (LED): To control the brightness or turn the backlight on/off, 
    you'll connect this pin to a digital output pin on your microcontroller. You might need to
    include a current-limiting resistor in series with this pin to prevent damage to the LED.
    The required resistor value depends on the LED's forward voltage and current rating, as well
    as the voltage you are supplying. If you simply want the backlight on all the time, you can
    connect this pin directly to VCC (again, possibly with a resistor).
- SPI Connections (SCK, SDA, CS): Connect these pins to the corresponding SPI pins on your
    microcontroller. Remember that SDA on the LCD module corresponds to the MOSI (Master Out Slave
    In) pin on your microcontroller.
- Data/Command Select (A0): Connect this to a digital output pin on your microcontroller.
- Reset (RESET): Connect this to a digital output pin on your microcontroller. You will typically
    pull this pin high during normal operation and pull it low briefly to reset the display.
- Ground (GND): Connect this to the ground pin of your microcontroller.
- Power (VCC): Connect this to the appropriate power supply voltage (3.3V or 5V, depending on your
    module's specification). Double-check the voltage requirements!

## Display to ESP32C3 Connection
| LCD Pin | ESP32-C3 Mini Pin | ESP32-C3 GPIO | Description | Notes
|---------|-------------------|---------------|-------------|------------
| LED     | VCC               | -             | Backlight (Always On) | check resistor, should be 220
| SCK     | SCK               | GPIO4         | SPI Clock   |
| SDA     | MOSI              | GPIO6         | SPI Data In | MOSI
| A0      | -                 | GPIO5         | Data/Command Select | control pin from GPIO5
| RESET   | -                 | GPIO10        | Reset       | control pin from GPIO10
| CS      | SS                | GPIO7         | Chip Select | control pin from GPIO7
| GND     | GND               | GND           | Ground      | connect to a common ground
| VCC     | 3.3V              | VCC (on ESP32)| Power Supply| 
