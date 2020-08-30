
#pragma once

#include "RFMxx.h"
#include "LaCrosse.h"


class MyRFMxx {

public:
  MyRFMxx(byte mosi, byte miso, byte sck, byte ss, byte irq, bool isPrimary);
  
  struct Frame {
    byte  ID;    
    bool  NewBatteryFlag;
    float Temperature;
    bool  WeakBatteryFlag;
    byte  Humidity;
    bool  IsValid;
  };

public:
  void setup();
  void process( struct Frame &frame );

private:
  RFMxx rfm;
  LaCrosse::Frame LaCrosse_frame;

  void SetDebugMode(boolean mode);
  void HandleReceivedLaCrosseData();
  void HandleDataRateToggle(unsigned long *lastToggle, unsigned long *dataRate, uint16_t interval, byte toggleMode);
 
};
