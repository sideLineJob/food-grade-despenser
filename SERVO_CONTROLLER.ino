#include <Arduino.h>
#include "SERVO_CONTROLLER.h"

void SERVO_CONTROLLER::servoInit() {
  containerServo.attach(3);
  containerServo2.attach(A4);
  containerServo3.attach(A5);
  dispenseServo.attach(A3);
  // set container 2..

  closeContainer();
  closeDispenseContainer();
  // set container 2 to close 
}

void SERVO_CONTROLLER::closeContainer() {
  containerServo.write(90);
  containerServo2.write(90);
  containerServo3.write(90);
  delay(15);
}

void SERVO_CONTROLLER::openContainer() {
  containerServo.write(2);
  delay(15);
}

void SERVO_CONTROLLER::openContainer2() {
  containerServo2.write(2);
  delay(15);
}

void SERVO_CONTROLLER::openContainer3() {
  containerServo3.write(2);
  delay(15);
}

void SERVO_CONTROLLER::closeDispenseContainer() {
  dispenseServo.write(90);
  delay(15);
}

void SERVO_CONTROLLER::openDispenseContainer() {
  dispenseServo.write(2);
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

boolean SERVO_CONTROLLER::stopDispensing(float loadValue, float stopValue) {
  if (loadValue >= stopValue) {
     closeContainer();
     return true;
  }
  return false;
}

boolean SERVO_CONTROLLER::stopMainDispensing(float loadValue) {
  if (loadValue < 5) {
     closeDispenseContainer();
     return true;
  }
  return false;
}
