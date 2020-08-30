 
#ifndef _SENSORS_h
#define _SENSORS_h

#include "Arduino.h"

#define SENS_COUNT 10
    
class Sensors {

public:
  
  struct sSensor {
    char name[20];
    char rxID;
    float temperature;
    char humidity;
    char errors;    
    unsigned int gracetime;
    bool batteryLow;
    bool updated;
    bool isValid;


  };

  Sensors();

  // function used for sensors to update this meta-data
  static void HandleSensorData( char ID, float temperature, char humidity, char batteryLow );
  
  // setup sensor
  void addSensor( char idx, char Name[], char rxID );
  
  // return meta-data for a specific sensor
  sSensor * getSensor( char idx );

  // print all meta-data
  void print();
  
  // 
  void process();
  
private:

    // meta-data 
    sSensor s[SENS_COUNT];  

    // new sensors
    unsigned char newSensIDs[SENS_COUNT];
    unsigned char newSensCount;

    // 
    bool loadNeeded;


    char loadFromEEPROM();
    void storeToEEPROM();
    void clearEEPROM();
    
    // re-organization    
    void reorg();
    
};

extern Sensors sensors;

#endif
