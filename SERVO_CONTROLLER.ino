#include <Arduino.h>
#include "SERVO_CONTROLLER.h"

void SERVO_CONTROLLER::servoInit() {
  containerServo.attach(3);
  // set container 2..

  closeContainer();
  // set container 2 to close 
}

void SERVO_CONTROLLER::closeContainer() {
  containerServo.write(90);
  delay(15);
}

void SERVO_CONTROLLER::openContainer() {
  containerServo.write(2);
  delay(15);
}

void SERVO_CONTROLLER::serialControlDispenser() {
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

boolean SERVO_CONTROLLER::stopDispensing(float loadValue , float stopValue) {
//  if (loadValue >= loadStopValue) {
//    closeContainer();
//  }
  if (loadValue >= stopValue) {
     closeContainer();
     return true;
  }

  return false;
}
