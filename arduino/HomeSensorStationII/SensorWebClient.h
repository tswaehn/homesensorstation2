#ifndef __WIFI_CLIENT_H__
#define __WIFI_CLIENT_H__

#include <Arduino.h>
#include <ESP8266WiFi.h>

class SensorWebClient {

  public:
    SensorWebClient();
  
    void setup(char * _ssid, char * _password);
    void send( char * token, int sensorValue );

  private:
    WiFiClient client;
    char * ssid;
    char * password;
    char * host;
    
};

extern SensorWebClient sensorWebClient;



#endif

