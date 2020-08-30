#include "Sensors.h" 
#include "EEPROM.h"

Sensors sensors;

Sensors::Sensors(){

  // load IDs from EEPROM
  loadNeeded= 1;
  
  for (char idx=0;idx<SENS_COUNT;idx++){
    s[idx].rxID= 0;
    s[idx].temperature= 0.0;
    s[idx].humidity= 0;
    s[idx].gracetime= 0;
    s[idx].batteryLow= 0;
    s[idx].updated= 0;
    s[idx].isValid= 0;     
    s[idx].errors= 0;
  }

  for (char idx=0;idx<SENS_COUNT;idx++){
    newSensIDs[idx]= 0;
  }
  newSensCount= 0;

}

void Sensors::addSensor( char idx, char Name[], char rxID ){
        
  if (idx >= SENS_COUNT){
    return;
  }
  if (idx < 0){
    return;
  }
  
  for (char i=0;i<20;i++){
        s[idx].name[i]= Name[i]; 
  }
  
  s[idx].rxID= rxID;
  s[idx].isValid= 1;
  
  
}

Sensors::sSensor * Sensors::getSensor( char idx ){
  
  if (idx >= SENS_COUNT){
    return 0;
  }
  if (idx < 0){
    return 0;
  }
  
  return &(s[idx]);
}

char Sensors::loadFromEEPROM(){

  unsigned char IDs[SENS_COUNT];
  unsigned char crc= 0;
  unsigned char crc_read= 0;
  unsigned char b;
  
  EEPROM.begin(SENS_COUNT+1);

  // load data from EEPROM
  for (char idx=0; idx<SENS_COUNT; idx++) {
      b= EEPROM.read(idx);
      IDs[idx]= b;
      crc+= b;

      Serial.print("Sens");
      Serial.print(idx, DEC);
      Serial.print(">");
      Serial.println(b, DEC);
  }   
  crc_read= EEPROM.read(SENS_COUNT);

  // check CRC
  if (crc != crc_read){
    Serial.print("CRC>");
    Serial.println( crc, DEC );
    Serial.println("CRC Failed");
    return 0;
  } else {
    Serial.println("CRC OK");    
  }

  // transfer IDs to sensor objects
  for (char idx=0; idx<SENS_COUNT; idx++) {
    s[idx].rxID= IDs[idx];  
  }
  
  Serial.println("Loaded IDs from EEPROM");

  return 1;
}

void Sensors::storeToEEPROM(){
  
  unsigned char crc= 0;
  unsigned char b;

  EEPROM.begin(SENS_COUNT+1);
  
  for (char idx=0; idx<SENS_COUNT; idx++) {
    b= s[idx].rxID;
    EEPROM.write(idx, b);
    crc +=b;
    
    Serial.print("Sens");
    Serial.print(idx, DEC);
    Serial.print(">");
    Serial.println(b, DEC);
    
  }     
  
  // write CRC
  EEPROM.write( SENS_COUNT, crc );
  EEPROM.commit();
  
  Serial.print("CRC>");
  Serial.println( crc, DEC );
  
  Serial.println("Stored IDs to EEPROM");  
  
}

void Sensors::clearEEPROM(){

  unsigned char crc= 0;
  unsigned char b;
  
  EEPROM.begin(SENS_COUNT+1);
  
  for (char idx=0; idx<SENS_COUNT; idx++) {
    b= idx;
    EEPROM.write(idx, b);
    crc +=b;  
    
    Serial.print("Sens");
    Serial.print(idx, DEC);
    Serial.print(">");
    Serial.println(b, DEC);
    
  }     
  
  // write invalid CRC
  EEPROM.write( SENS_COUNT, crc+1 );
  EEPROM.commit();

  Serial.println("Cleared IDs from EEPROM");
}


// re-organization
void Sensors::reorg(){

  unsigned char missingSens= 0;
  unsigned char missingSensIdx= 0;
  
  // check the number of missing sensors
  for (int idx= 0; idx < SENS_COUNT; idx++){
    if (s[idx].isValid){
      if (s[idx].errors > 0){
        missingSens++;       
        missingSensIdx= idx;
      }
    }
  }

  // if - and only if only 1 sensor is missing and exactly 1 new sensor exists, then we replace the ID
  if (missingSens != 1){
    Serial.println("Too many sensors missing - cannot replace");
    return;
  }
  if (newSensCount != 1){
    Serial.println("Too many new sensor - cannot find suiting matches");
    return;
  }

  // now replace
  Serial.print("Replacing ID ");
  Serial.print( s[missingSensIdx].rxID, DEC );
  Serial.print(" by ");
  Serial.println( newSensIDs[0], DEC );
    
  s[missingSensIdx].rxID= newSensIDs[0];
  storeToEEPROM();
  
  Serial.println("done");
  
}

void Sensors::process(){
        
  static unsigned long startTime= 0;
  unsigned long currentTime= millis();

/*
  // check if sensors need setup
  if (loadNeeded){
    loadNeeded= 0; // run this routine only once
      
    // for testing only
    //clearEEPROM();
    //storeToEEPROM();
    
    // try to load IDs from EEPROM
    if ( loadFromEEPROM() == 0){
      // if failed use IDs from FLASH
      storeToEEPROM();
    }
  }
*/       
  // check if 1min has passed
  if ((currentTime-startTime) > 60000){
    startTime= currentTime;
 
    // check all sensors if they were updated correctly
    for (char idx=0;idx<SENS_COUNT;idx++){
      if (s[idx].isValid==0){
        continue;
      }
      
        // sensor was not updated correctly
      if (s[idx].updated==0){
        // reset gracetime
        s[idx].gracetime= 0;
        // increase error count
        if (s[idx].errors <= 99){
          s[idx].errors++;
        }
        
        
      } else {
        // sensor was updated correctly  
        s[idx].gracetime++;
        
        // clear one error every 24hours 
        if (s[idx].gracetime >= 1440 ){ //[24hours]
          s[idx].gracetime= 0;
          // so we can check once a day for errors
          if (s[idx].errors > 0){
                s[idx].errors--; 
          }
        }
      }
      
      // reset update
      s[idx].updated= 0;
    }

    // if new sensors are attached
    if (newSensCount > 0){
      reorg();
                  
      Serial.print("New sensor count is ");
      Serial.println( newSensCount, DEC );
    }
        
    print();
  }
  
}


void Sensors::HandleSensorData( char ID, float temperature, char humidity, char batteryLow ){

  unsigned char i;
  for (i=0;i<SENS_COUNT;i++){
    if (sensors.s[i].rxID == ID ){
      sensors.s[i].temperature= temperature;
      sensors.s[i].humidity= humidity;
      sensors.s[i].batteryLow= batteryLow;
      sensors.s[i].updated= 1;
      
      return;
    }
  }

  // if we came to here then we found a new sensor
  for (i= 0; i<sensors.newSensCount; i++){
    // this new ID is already known 
    if (sensors.newSensIDs[i] == ID ){
      return;
    } 
  }

  // the new ID is unknown -- add the sensors ID
  if (sensors.newSensCount < SENS_COUNT){
    sensors.newSensIDs[sensors.newSensCount++]= ID;
    
    Serial.print("New sensor found with ID ");
    Serial.println( ID, DEC );
  }
  
}

void Sensors::print(){

  Serial.println("---");
  for (char i=0;i<SENS_COUNT;i++){
    if (s[i].isValid){
        Serial.print( s[i].name );
      Serial.print( " " );
      Serial.print( s[i].temperature );
      Serial.print( " " );
      Serial.print( s[i].humidity, DEC );
      Serial.print( " " );
      Serial.print( s[i].batteryLow, DEC );
      Serial.print( " " );
      Serial.print( s[i].errors, DEC );
      Serial.println();
      
    }
  }  
  
}

 
