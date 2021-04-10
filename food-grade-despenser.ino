#include "HX711_ADC.h"
#include "SERVO_CONTROLLER.h"
#include <EEPROM.h>
#include <LiquidCrystal.h>

const int HX711_dout = 4; //mcu > HX711 dout pin
const int HX711_sck = 5; //mcu > HX711 sck pin

HX711_ADC LoadCell(HX711_dout, HX711_sck);
SERVO_CONTROLLER containerCont; 

const int calVal_eepromAdress = 0;
unsigned long t = 0;
float loadValue = 0; // in mg
float loadStopValue = 0;
boolean startStopListening = false;
boolean startStopMainContListening = false;

// Coin slot
const int coinInt = 0;
volatile float coinsValue = 0.00;
int coinsChange = 0;

// LCD
LiquidCrystal lcd(12, 11, 7, 8, 9, 10);

// Button vars
boolean disableAllButtons = false;
int selectedButton = 0;


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

  containerCont.servoInit();
  // coin slot interrupt
  attachInterrupt(coinInt, coinInserted, RISING);
  // LCD screen setup
  lcd.begin(16, 2);
  /**
   * Button pins
   */
  pinMode(A0, INPUT);
  introText();
}

void loop() {
  
  updateLoadValue();
  containerCont.serialControlDispenser();

  if (startStopListening) {

    /**
     * ex.
     * loadStopValue = 100
     * get 5% from loadStopValue(Threshold)
     * threshold = 5%
     * loadStopValue = loadStopValue - (loadStopValue * 5%)
     */

    float threshold = 0.05; // 5 percent threshold

    float valueWithThreshold = loadStopValue - (loadStopValue * threshold);
    
    if (containerCont.stopDispensing(loadValue, valueWithThreshold)) { // replace "loadStopValue" with "valueWithThreshold"
      lcd.clear();
      lcd.print("Done Dispensing!");
      lcd.setCursor(0, 1);
      lcd.print("Thank You!");
      resetDispender();
      delay(1000);
      // open main container 
      containerCont.openDispenseContainer();
      startStopMainContListening = true;
      delay(3000);
      introText(); 
    }
  }

  if (startStopMainContListening) {
    if (containerCont.stopMainDispensing(loadValue)) {
      startStopMainContListening = false;
    }
  }

  coinSlotAction();

  if (!disableAllButtons) {
    buttonActions();
  } else {
    dispenserActions();
  }
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

void coinInserted() {
  coinsValue = coinsValue + 1;
  coinsChange = 1;
}

void coinSlotAction() {
  if (coinsChange == 1) {
    coinsChange = 0;
    Serial.println();
    Serial.print("Coins Value: ");
    Serial.println(coinsValue);
    Serial.println();
    lcd.clear();
    lcd.print("Amount: ");
    lcd.setCursor(4, 1);
    lcd.print("PHP");
    lcd.setCursor(8, 1);
    lcd.print(coinsValue);
  }
}

void buttonActions() {
  int button1 = digitalRead(A0);
  int button2 = digitalRead(A1);
  int button3 = digitalRead(A2);
  
  if (button1 == 1) {
    Serial.println("\nButton 1 pressed...\n");
    disableAllButtons = true;
    selectedButton = 1;
    
  } else if (button2 == 1) {
    Serial.println("\nButton 2 pressed...\n");
    disableAllButtons = true;
    selectedButton = 2;
    
  } else if (button3 == 1) {
    Serial.println("\nButton 3 pressed...\n");
    disableAllButtons = true;
    selectedButton = 3;
  }
}


void resetDispender() {
  disableAllButtons = false;
  startStopListening = false;
  coinsValue = 0;
}

void introText() {
  lcd.clear();
  lcd.print("Please Insert");
  lcd.setCursor(0, 1);
  lcd.print("   Coin!");
}

void dispenserActions() {
  if (coinsValue > 0) { 
    switch(selectedButton) {
      case 1:
        dispensePowderTest('A', 50);
        containerCont.openContainer();
        break;
      case 2:
        dispensePowderTest('B', 40);
        containerCont.openContainer2();
        break;
      case 3:
        dispensePowderTest('C', 30);
        containerCont.openContainer3();
        break;
    }
  } else {
    resetDispender();
  }

  selectedButton = 0;
}

void dispensePowderTest(char dType, float price) {
  // calculate load
  int price_per_kilo = price;
  int g_to_kg = 1000;

  float kgEquivalent = coinsValue / price_per_kilo;
  loadStopValue = kgEquivalent * g_to_kg;

  lcd.clear();
  lcd.print("Despensing ");
  lcd.print(dType);
  lcd.print("...");
  lcd.setCursor(0, 1);
  lcd.print(" = ");
  lcd.setCursor(4, 1);
  lcd.print(String(kgEquivalent) + " kg");

  delay(3000);
  
  startStopListening = true;
}
