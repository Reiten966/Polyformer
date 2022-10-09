# ESP8266 and ESP32 Arduino menu with Wifi control and a local OLED SSD1306.

 This example shows a more complex menu with the following components.
 
 * 128 x 32 OLED menu
 * Rotary encoder wired directly
 * I2C EEPROM device to load and save menu.
 * Wifi support using the inbuilt WiFi device.

This example is built for the ESP device. However, with a few changes it could probably be used for other boards. Although you'd be better off starting
with a more appropriate example for your hardware.

In order to connect using the Java API there are two possibilities, either use the connector UI that's shipped with TcMenu, or work with the tcMenu API directly. See [https://github.com/davetcc/tcMenu]. We'll be building out the API in other languages over time, but you can also write directly to the
protocol, which is very simple.

The authentication on this version is using the EEPROM based authenticator. This stores a number of keys in the EEPROM. However, its using the flash
based rom by default, so you need to choose save all from the setup menu after pairing. This commits the changes to ROM. BE VERY AWARE that you are
writing to FLASH memory when you do this, and the workable cycles are much lower than proper EEPROM. I prefer including EEPROM i2c devices which are
safer.

The files in this example are as follows:

 * `Greenhouse.emf` the controller system simulating our food growing greenhouse. Open this is the designer UI
 * `esp8266WifiOled.ino` the main sketch file, this will be modified by designer when new callbacks are added.
 * `EthernetTransport.cpp` and `.h` contain the ethernet plugin for tcMenu and should not be altered. Rewritten each code generation
 * `tcMenuAdaFruitGfx.cpp` and `.h` contain the graphics drawing plugin and should not be altered. Rewritten each code generation.
 * `esp8266WifiOled_menu.cpp` and `.h` contain the definitions of the menu item, and should not be altered. Rewritten each code generation.
 