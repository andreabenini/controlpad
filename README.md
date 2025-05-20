# controlpad - ESP32 controller for small robots and general appliances
<div align="center">
  <picture>
    <img src="logo.png" height="150" alt="Linux Logo">
  </picture><br>
All purpose robot and appliance controller
</div>


### _under development_
Project under development with C and ESP-IDF framework, I am uploading latest code
but it still has to be marked as effective alpha version.


### General Features
- PS5 button layout (same size too, effectively with a PS5 shell)  
  16 buttons and two analog joysticks. ESP32C3 core and I2C I/O Expander
- Multi host profile selection to manage multiple appliances
- Embedded display with status information, profile selection, diagnostics
- Configuration update with simple yaml files, firmware flashing with ESP32 utils
- Desktop configuration and upload utility for managing the device
- Macro mapping to single keys or actions with intuitive .yaml files
- LiPo battery, high capacity, durable, rechargeable
- Smartphone stand for independent video stream from the device (if any)


### Communication
- ESP Now
- Bluetooth
- Bluetooth LE
- WiFi (TCP/UDP)

ESP32C3 provides multiple radio options, each single profile can use its preferred
communication method even if for ESP32 based home robots ESP-Now is probably the
suggested method when power consumption, long distance and reliability are taken in
place. Meanwhile TCP and UDP are probably more common for other embedded boards
(RPi devices for example) or generic PCs while Bluetooth for generic embedded
appliances


### TODO List
- (-) 3D STL files definition for inner components (WiP)
- (-) WiP on: hardware, electronics, software (CLI util and builtin firmware)
- (x) ESP32 framework, project and general setup
- (x) Finite state machine and controller endless loop, status definition
- (x) Loading/Uploading configurations from USB (dedicated python utility on desktop)
- (-) Display management, profiles selection, wifi connection, utility menu, hw checkers
- (-) Configuration profiles, yaml parser, networking setup, sending keystrokes to remote
- ( ) Troubleshooting, test units, network stress tests
- ( ) Electronics assembly
- ( ) Final HW assembly. 3D printed internals, electronics, I/O devices
- ( ) Final tests on firmware v1.0 and overall tests with different testbeds (PC,
      Generic ESP32, Atmel, RPi devices)
