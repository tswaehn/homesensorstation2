# homesensorstation2

## requirements
*  NodeMCU Board
   * Arduino compatible ESP8266 controller board (4MB flash, I2C, SPI, WiFi, ... )
   * doc in german https://www.mikrocontroller-elektronik.de/nodemcu-esp8266-tutorial-wlan-board-arduino-ide/
*  RFM12B
   * https://www.ebay.de/itm/RFM12B-868Mhz-Wireless-Transceiver-from-HopeRF-RFM12B-868S2P-/171670876052
*  HomeSensor2 Base board
   * this board hosts the RFM12B, SSD1306 display and NodeMCU
   * schematics is in the repo 
*  SSD1306 display (128x64 pixels)
   * any compatible display with I2C will do
*  Arduino IDE 1.8.9
   * I personally prefer exactly this version, others may do it as well


## installation
*  install ESP8266 boards (follow instructions https://github.com/esp8266/Arduino)
*  load the arduino project
*  select "Tools" - "Board" - "NodeMCU (ESP 12E Module)" 
*  select "Tools" - "Port" 
*  run compile button in Arduino
*  upload to NodeMCU
*  done


## pin-out
*  SSD1306 Display 
   * SDA: D2
   * SCL: D1
*  RFM12B
   * MOSI: D7
   * MISO: D6
   * SCK: D5
   * SS: D8
   * IRQ: D0
   
