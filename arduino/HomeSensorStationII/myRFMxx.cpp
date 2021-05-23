
#include "myRFMxx.h"

#include "Wire.h"


// --- Configuration ---------------------------------------------------------------------------------------------------
#define RECEIVER_ENABLED       1                     // Set to 0 if you don't want to receive 
#define USE_OLD_IDS            0                     // Set to 1 to use the old ID calcualtion

// The following settings can also be set from FHEM
#define ENABLE_ACTIVITY_LED    1         // <n>a     set to 0 if the blue LED bothers
bool DEBUG                   = 1;        // <n>d     set to 1 to see debug messages
bool ANALYZE_FRAMES          = 1;        // <n>z     set to 1 to display analyzed frame data instead of the normal data
unsigned long DATA_RATE_R1   = 17241ul;  // <n>r     use one of the possible data rates (for RFM #1)
unsigned long DATA_RATE_R2   = 9579ul;   // <n>R     use one of the possible data rates (for RFM #2)
uint16_t TOGGLE_INTERVAL_R1  = 0;        // <n>t     0=no toggle, else interval in seconds (for RFM #1)
uint16_t TOGGLE_INTERVAL_R2  = 0;        // <n>T     0=no toggle, else interval in seconds (for RFM #2)
byte TOGGLE_MODE_R1          = 3;        // <n>m     bits 1: 17.241 kbps, 2 : 9.579 kbps, 4 : 8.842 kbps (for RFM #1)
byte TOGGLE_MODE_R2          = 3;        // <n>M     bits 1: 17.241 kbps, 2 : 9.579 kbps, 4 : 8.842 kbps (for RFM #2)
unsigned long INITIAL_FREQ   = 868300;   // <n>f     initial frequency in kHz (5 kHz steps, 860480 ... 879515) 
bool RELAY                   = 0;        // <n>r     if 1 all received packets will be retransmitted  
byte PASS_PAYLOAD            = 0;        // <n>p     transmitted the payload on the serial port 1: all, 2: only undecoded data


// --- Variables -------------------------------------------------------------------------------------------------------
unsigned long lastToggleR1 = 0;
unsigned long lastToggleR2 = 0;
byte commandData[32];
byte commandDataPointer = 0;

// mosi, miso, sck, ss, irq 
//RFMxx rfm1(D7, D6, D5, D8, D0);

MyRFMxx::MyRFMxx(byte mosi, byte miso, byte sck, byte ss, byte irq, bool isPrimary):rfm(mosi, miso, sck, ss, irq, isPrimary){

  
}


void MyRFMxx::SetDebugMode(boolean mode) {
  DEBUG = mode;
  rfm.SetDebugMode(mode);

}




void MyRFMxx::HandleReceivedLaCrosseData() {
  rfm.EnableReceiver(false);

  byte payload[PAYLOADSIZE];
  rfm.GetPayload(payload);

  // ---
  // analyze the payload
  
  if (ANALYZE_FRAMES) {
    LaCrosse::AnalyzeFrame(payload);
    Serial.println();

    LaCrosse::DecodeFrame(payload, &LaCrosse_frame);
    
  } else if (PASS_PAYLOAD == 1) {

    for (int i = 0; i < PAYLOADSIZE; i++) {
      Serial.print(payload[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    
  } else {

    if (DEBUG) {
      Serial.print("\nEnd receiving, HEX raw data: ");
      for (int i = 0; i < 16; i++) {
        Serial.print(payload[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
    }

    byte frameLength = 0;
  

    // Try LaCrosse like TX29DTH
    if (LaCrosse::IsValidDataRate(rfm.GetDataRate()) && LaCrosse::TryHandleData(payload)) {
      frameLength = LaCrosse::FRAME_LENGTH;
    } else if (PASS_PAYLOAD == 2) {
      for (int i = 0; i < PAYLOADSIZE; i++) {
        Serial.print(payload[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
    }


    if (RELAY && frameLength > 0) {
      delay(64);
      rfm.SendArray(payload, frameLength);
      if (DEBUG) { Serial.println("Relayed"); }
    }

  }
  rfm.EnableReceiver(true);
}

void MyRFMxx::HandleDataRateToggle(unsigned long *lastToggle, unsigned long *dataRate, uint16_t interval, byte toggleMode) {
  if (interval > 0) {
    // After about 50 days millis() will overflow to zero 
    if (millis() < *lastToggle) {
      *lastToggle = 0;
    }

    if (millis() > *lastToggle + interval * 1000) {
      // Bits 1: 17.241 kbps, 2 : 9.579 kbps, 4 : 8.842 kbps

      if (*dataRate == 8842ul) {
        if (toggleMode & 2) {
          *dataRate = 9579ul;
        }
        else if (toggleMode & 1) {
          *dataRate = 17241ul;
        }
      }
      else if (*dataRate == 9579ul) {
        if (toggleMode & 1) {
          *dataRate = 17241ul;
        }
        else if (toggleMode & 4) {
          *dataRate = 8842ul;
        }
      }
      else if (*dataRate == 17241ul) {
        if (toggleMode & 4) {
          *dataRate = 8842ul;
        }
        else if (toggleMode & 2) {
          *dataRate = 9579ul;
        }
      }

      rfm.SetDataRate(*dataRate);
      *lastToggle = millis();

    }
  }
}

void MyRFMxx::setup(){

  
  SetDebugMode(DEBUG);
  LaCrosse::USE_OLD_ID_CALCULATION = USE_OLD_IDS;

  lastToggleR1 = millis();
  
  rfm.InitialzeLaCrosse();
  rfm.SetFrequency(INITIAL_FREQ);
  rfm.SetDataRate(DATA_RATE_R1);
  rfm.EnableReceiver(true);
  
  
  if (DEBUG) {
    Serial.println("Radio setup complete. Starting to receive messages");
  }
}

void MyRFMxx::process( struct Frame &frame ){

  // preset 
  frame.IsValid= 0;

  // Handle the data reception
  // -------------------------
  if (RECEIVER_ENABLED) {
    rfm.Receive();

    if (rfm.PayloadIsReady()) {
      // decode rfm-payload
      HandleReceivedLaCrosseData();

      if (LaCrosse_frame.IsValid){
        frame.ID= LaCrosse_frame.ID;
        frame.NewBatteryFlag= LaCrosse_frame.NewBatteryFlag;
        frame.Temperature= LaCrosse_frame.Temperature;
        frame.WeakBatteryFlag= LaCrosse_frame.WeakBatteryFlag;
        frame.Humidity= LaCrosse_frame.Humidity;
        frame.IsValid= 1;
            
        Serial.print(">>recived good data ");
      }
    }
  }
 
  // Handle the data rate
  // --------------------
  HandleDataRateToggle(&lastToggleR1, &DATA_RATE_R1, TOGGLE_INTERVAL_R1, TOGGLE_MODE_R1);
  

}
