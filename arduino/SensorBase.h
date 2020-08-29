#ifndef _SENSORBASE_h
#define _SENSORBASE_h

#include "Arduino.h"

class SensorBase {
public:
  static byte CalculateCRC(byte *data, byte len);
  static void SetDebugMode(boolean mode);
  
  static void registerSensorData( char ID, float temperature, char humidity, char batteryLow );

protected:
  static bool m_debug;

};

#endif

