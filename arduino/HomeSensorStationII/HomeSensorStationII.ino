// Tested with JeeLink v3 (2012-02-11)
// polling RFM12B to decode FSK iT+ with a JeeNode/JeeLink from Jeelabs.
// Supported devices: see FHEM wiki
// info    : http://forum.jeelabs.net/node/110
//           http://fredboboss.free.fr/tx29/tx29_sw.php
//           http://www.f6fbb.org/domo/sensors/
//           http://www.mikrocontroller.net/topic/67273 
//           benedikt.k org rinie,marf,joop 1 nov 2011, slightly modified by Rufik (r.markiewicz@gmail.com)
// Changelog: 2012-02-11: initial release 1.0
//            2014-03-14: I have this in SubVersion, so no need to do it here

#define PROGNAME1       "Home Sensor"
#define PROGNAME2       "Station"
#define PROGVERS        "v2.3" 

#include "Arduino.h"
#include "SensorBase.h"
#include "Sensors.h"
#include "EEPROM.h"
#include "SSD1306_minimal.h"
#include "wifi_config.h"
#include "myRFMxx.h"

#if WIFI_ENABLED == 1
  #include <ESP8266WiFi.h>  
  #include "SensorWebClient.h"
  
#endif


//Byte array of bitmap of 76 x 56 px:
const unsigned char img_logo [] PROGMEM = {
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x3f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x30, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x07, 0xc3, 0xe3, 0xe3, 0xe3, 0xe3, 0xc3, 0x07, 0x0f, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xf0, 0xe1, 0xc3, 0xc7, 0xc7, 0xc7, 0xc7, 0xc7, 0xff, 0xff, 0xff, 0xf0, 0xe0, 0xe3, 0xc7, 0xc7, 0xc7, 0xc7, 0xe3, 0xe0, 0xf0, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe3, 0xe3, 0xe3, 0xe3, 0xe3, 0xe3, 0x03, 0x7f, 0x7f, 0x7f, 0x7f, 0x03, 0xe3, 0x03, 0x7f, 0x7f, 0x7f, 0x03, 0xe3, 0x03, 0x7f, 0x7f, 0x7f, 0x7f, 0x03, 0xe3, 0xe3, 0xe3, 0xe3, 0xe3, 0xe3, 0x00, 0x00, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x0f, 0x8f, 0x8f, 0x8f, 0x8f, 0x8f, 0x8f, 0x0f, 0x1f, 0x7f, 0xff, 0xff, 0xff, 0xff, 0x8f, 0x8f, 0x8f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x9f, 0x8f, 0x8f, 0x8f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x8f, 0x8f, 0x8f, 0x8f, 0x8f, 0x8f, 0xc0, 0xc0, 0xf0, 0xff, 0xff, 0xf9, 0xf1, 0x01, 0x01, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf9, 0xf1, 0x01, 0x01, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x01, 0x03, 0xe3, 0xf1, 0xf1, 0xf1, 0xf1, 0x01, 0x03, 0xff, 0xff, 0xff, 0xff, 0x07, 0x03, 0xe1, 0xf1, 0xf1, 0xf1, 0xf1, 0xe1, 0x03, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe3, 0xe3, 0xe0, 0xe0, 0xe0, 0xe3, 0xe3, 0xf7, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe3, 0xe3, 0xe0, 0xe0, 0xe0, 0xe3, 0xe3, 0xf7, 0xff, 0xff, 0xf0, 0xe0, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xf8, 0xf0, 0xf1, 0xe3, 0xe3, 0xe3, 0xe3, 0xf1, 0xf0, 0xf8, 0xff, 0xff, 0xff, 0xff
};

//byte array of bitmap 5x24px
const unsigned char  img_thermometer[] PROGMEM = {
  
  0x00, 0xfe, 0x03, 0xfe, 0x50,
  0x00, 0xff, 0x00, 0xff, 0x55,
  0x60, 0x9f, 0x80, 0x9f, 0x65,
   
};

// byte array of bitmap 32x32 => 32 x 4
const unsigned char img_fridge[] PROGMEM = {
    0x00, 0x00, 0x80, 0xa0, 0xe0, 0xc0, 0xf0, 0x00, 0x00, 0xc0, 0x00, 0x40, 0x42, 0x84, 0x88, 0xff, 0xff, 0x88, 0x84, 0x42, 0x40, 0x00, 0xc0, 0x00, 0x00, 0xf0, 0xc0, 0xe0, 0xa0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x09, 0x0b, 0x0e, 0x0f, 0x18, 0x30, 0x60, 0xc4, 0x88, 0x7f, 0x7f, 0x88, 0xc4, 0x60, 0x30, 0x18, 0x0f, 0x0e, 0x0b, 0x09, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x90, 0xd0, 0x70, 0xf0, 0x18, 0x0c, 0x06, 0x23, 0x11, 0xfe, 0xfe, 0x11, 0x23, 0x06, 0x0c, 0x18, 0xf0, 0x70, 0xd0, 0x90, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x05, 0x07, 0x03, 0x0f, 0x00, 0x00, 0x03, 0x00, 0x02, 0x42, 0x21, 0x11, 0xff, 0xff, 0x11, 0x21, 0x42, 0x02, 0x00, 0x03, 0x00, 0x00, 0x0f, 0x03, 0x07, 0x05, 0x01, 0x00, 0x00
};

// byte array of bitmap 32x32 => 32 x 4
const unsigned char img_weather[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x30, 0x60, 0xc0, 0x80, 0x06, 0x0c, 0x38, 0xa0, 0x80, 0xc0, 0xc0, 0xd0, 0xde, 0x83, 0x80, 0x00, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xe0, 0x00, 0x80, 0xc0, 0xe1, 0xcc, 0x9e, 0x3f, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfc, 0xf0, 0x06, 0x82, 0xc3, 0x41, 0x40, 0x00, 0x80, 0xc0, 0x60, 0xf0, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfc, 0xf9, 0xf3, 0x67, 0xcf, 0x9f, 0x1f, 0x0f, 0x27, 0x63, 0xc0, 0x84, 0x0c, 0x0c, 0x18, 0x18, 0x01, 0x01, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x01, 0x01, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00
};

// byte array of bitmap 32x32 => 32 x 4
const unsigned char img_dining[] PROGMEM = {
    0xe0, 0x00, 0xe0, 0x00, 0xe0, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x40, 0x40, 0x40, 0x40, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xe0, 0xc0, 0x00, 0x07, 0x0c, 0xff, 0xf8, 0xff, 0x0c, 0x07, 0x00, 0xf0, 0x0c, 0x62, 0xf9, 0xfc, 0xfe, 0xfe, 0xff, 0xff, 0xfe, 0xfe, 0xfc, 0xf9, 0x62, 0x0c, 0xf0, 0x00, 0x00, 0x1f, 0x3f, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x7f, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x03, 0x04, 0x09, 0x13, 0x17, 0x27, 0x2f, 0x2f, 0x27, 0x17, 0x13, 0x09, 0x04, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// byte array of bitmap 13x8 => 13 x 1
const unsigned char img_battery_low[] PROGMEM = {
    0xff, 0x81, 0xbd, 0xbd, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xff, 0x42, 0x3c
};

// byte array of bitmap 13x8 => 13 x 1
const unsigned char img_battery_full[] PROGMEM = {
    0xff, 0x81, 0xbd, 0xbd, 0xbd, 0xbd, 0xbd, 0xbd, 0xbd, 0x81, 0xff, 0x42, 0x3c
};





SSD1306_Mini oled;
// mosi, miso, sck, ss, irq 
//RFMxx rfm1(D7, D6, D5, D8, D0);
MyRFMxx myRFMxx(D7, D6, D5, D8, D0, true);




void splash(){

  oled.startScreen();
  oled.clear();
  
  oled.drawImage( img_logo, 20,0, 76, 7 );
  oled.cursorTo(0,7);
  oled.printString( "http://CoPiino.cc");
  
  delay(4000);

  oled.clear();
  oled.cursorTo(26, 3);
  oled.printString( PROGNAME1 );
  oled.cursorTo(40, 4);
  oled.printString( PROGNAME2 );
  oled.cursorTo(50, 5);
  oled.printString( PROGVERS );

  delay(4000);
  
  oled.clear();
  oled.startScreen();

}

void display(){

  char buf[50];
  #define DEG "\xa7" "C"
  
  static unsigned long startTime= 0;
  unsigned long currentTime= millis();
  
  if ((currentTime-startTime) < 4000){
        return; 
  }
  startTime= currentTime;
  
  
  
  oled.drawImage( img_thermometer, 45,4, 5, 3 );

  
  static char sensorNr= 0;
  
  Sensors::sSensor * pSensor= sensors.getSensor(sensorNr);
  //
  if (pSensor==0){
    return;
  }

  // draw battery symbol
  if (pSensor->batteryLow){
      oled.drawImage( img_battery_low, 114,0, 13, 1 );
  } else {
      oled.drawImage( img_battery_full, 114,0, 13, 1 );
  }

  // update temperature
  signed char temperature= (signed char) pSensor->temperature;
  dtostrf( temperature, 3, 0, buf );
  //sprintf(buf, "%+0.2d" , temperature);

  // print to display
  oled.printString2( 55, 3, buf );
  // set cursor
  oled.cursorTo(100, 3);
  oled.printString( DEG );

  
  // update humidity
  unsigned char humidity= (unsigned char) pSensor->humidity;
  itoa( humidity, buf, 10 );
  //sprintf(buf, "%0.2d%%rH", humidity);
  // set cursor
  oled.cursorTo(67, 6);
  // print to display
  oled.printString( buf );
  oled.printString( " %rH" );

  // update name
  oled.cursorTo(0, 0);
  oled.printString( pSensor->name );
  
  switch(sensorNr){
        
    case 0: oled.drawImage( img_fridge, 5,2, 32, 4 ); break;
    case 1: oled.drawImage( img_weather, 5,2, 32, 4 ); break;
    case 2: oled.drawImage( img_dining, 5,2, 32, 4 ); break;
    
    default:
          oled.drawImage( img_fridge, 10,2, 32, 4 );
    
  }

  // write error counter
  unsigned char errors= pSensor->errors;
  itoa( errors, buf, 10 );
  oled.cursorTo(0,7);
  oled.printString( buf );

    
  sensorNr++;
  if (sensorNr>=3){
        sensorNr=0;
  }

}


// **********************************************************************
void loop(void) {
  // Handle the commands from the serial port
  // ----------------------------------------
  if (Serial.available()) {
    //HandleSerialPort(Serial.read());
  }


  struct MyRFMxx::Frame frame;
  myRFMxx.process(frame);
  if (frame.IsValid){
    Serial.print(" ID:");
    Serial.print(frame.ID);
    Serial.print(" Temp:");
    Serial.print(frame.Temperature);
    Serial.print(" Hum:");
    Serial.print(frame.Humidity);
    Serial.println();

    Sensors::HandleSensorData( frame.ID, frame.Temperature, frame.Humidity, frame.WeakBatteryFlag );
  }
  
  //
  //
  sensors.process();
  
  //
  //
  display();

  static unsigned long lastTime= 0;
  unsigned long currentTime= millis();

  // upload every 10min to Internet
  /*
  if ((currentTime-lastTime) > (1000*60)){
    lastTime= currentTime;
    Sensors::sSensor * pSens;
    

    // sensor of fridge
    //Sensors::sSensor * pSens= sensors.getSensor( 0 );
    //sensorWebClient.send("8812ea0f0a57150e73596892218692c0", pSens->humidity );
    //sensorWebClient.send("80e3a73b12ca1951dde6ac1d421ee165", pSens->temperature );

    // sensor of weather
    pSens= sensors.getSensor( 1 );
    sensorWebClient.send("fa1299d150f525759ae8265cbe781362", pSens->humidity );
    sensorWebClient.send("c981a2c66d5e6757155e45726eda805d", pSens->temperature );

    // sensor of dining room
    pSens= sensors.getSensor( 2 );
    sensorWebClient.send("8812ea0f0a57150e73596892218692c0", pSens->humidity );
    sensorWebClient.send("80e3a73b12ca1951dde6ac1d421ee165", pSens->temperature );
    
  }
  */
  
}


void setup(void) {

  Serial.begin(115200);
  delay(200);
    Serial.println();
    Serial.println();
    Serial.println("*** LaCrosse weather station wireless receiver for IT+ sensors ***");

  
  // OLED
  oled.init(0x3c);
  oled.clear();

  delay(1000);
  
  splash();

  #if WIFI_ENABLED == 1
    // call the setup for wifi
    char * ssid= WIFI_SSID;
    char * password= WIFI_PSK;
    sensorWebClient.setup(ssid, password);
  
  #endif
    
  
  //
  
  sensors.addSensor( 0, "Fridge     ", 53 );//53
  sensors.addSensor( 1, "Weather    ", 30 );//4
  sensors.addSensor( 2, "Dining Room", 56 );//42

  myRFMxx.setup();
  


}





 
