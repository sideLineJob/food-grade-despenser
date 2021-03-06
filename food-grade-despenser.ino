#include "HX711_ADC.h"
#include <EEPROM.h>
#include <Servo.h>

const int HX711_dout = 4; //mcu > HX711 dout pin
const int HX711_sck = 5; //mcu > HX711 sck pin

HX711_ADC LoadCell(HX711_dout, HX711_sck);
Servo containerServo;

const int calVal_eepromAdress = 0;
unsigned long t = 0;
float loadValue = 0; // in mg
float loadStopValue = 65; // load test stop value

void setup() {
  Serial.begin(57600); 
  delay(10);
  Serial.println();
  Serial.println("Starting...");

  float calibrationValue;
  calibrationValue = 696.0;
  EEPROM.get(calVal_eepromAdress, calibrationValue);

  LoadCell.begin();
  unsigned long stabilizingtime = 2000; // tare preciscion can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
  
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
  } else {
    LoadCell.setCalFactor(calibrationValue); // set calibration factor (float)
    Serial.println("Startup is complete");
  }
  
  while (!LoadCell.update());
  
  Serial.print("Calibration value: ");
  Serial.println(LoadCell.getCalFactor());
  Serial.print("HX711 measured conversion time ms: ");
  Serial.println(LoadCell.getConversionTime());
  Serial.print("HX711 measured sampling rate HZ: ");
  Serial.println(LoadCell.getSPS());
  Serial.print("HX711 measured settlingtime ms: ");
  Serial.println(LoadCell.getSettlingTime());
  Serial.println("Note that the settling time may increase significantly if you use delay() in your sketch!");
  
  if (LoadCell.getSPS() < 7) {
    Serial.println("!!Sampling rate is lower than specification, check MCU>HX711 wiring and pin designations");
  } else if (LoadCell.getSPS() > 100) {
    Serial.println("!!Sampling rate is higher than specification, check MCU>HX711 wiring and pin designations");
  }

  servoInit();
}

void loop() {
  updateLoadValue();

  testSerialControlDispenser();

  stopDispensing();

  /**
   * Temporary comment
   */
  // receive command from serial terminal, send 't' to initiate tare operation:
//  if (Serial.available() > 0) {
//    char inByte = Serial.read();
//    if (inByte == 't') LoadCell.tareNoDelay();
//  }
//
//  // check if last tare operation is complete:
//  if (LoadCell.getTareStatus() == true) {
//    Serial.println("Tare complete");
//  }
  

}

void updateLoadValue() {
  static boolean newDataReady = 0;
  const int serialPrintInterval = 250; //increase value to slow down serial print activity

  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // get smoothed value from the dataset:
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      loadValue = LoadCell.getData();
      
      Serial.print("Load_cell output val: ");
      Serial.println(loadValue);
      
      newDataReady = 0;
      t = millis();
    }
  }
}

void servoInit() {
  containerServo.attach(2);
  // set container 2..

  closeContainer();
  // set container 2 to close 
}

void closeContainer() {
  containerServo.write(90);
  delay(15);
}

void openContainer() {
  containerServo.write(0);
  delay(15);
}

void testSerialControlDispenser() {
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    
    if (inByte == 'o') {
      // set container to open
      openContainer();
    }
    
    if (inByte == 'c') {
      // force container to close
      closeContainer();
    }
  }
}

void stopDispensing() {
  if (loadValue >= loadStopValue) {
    closeContainer();
  }
}
