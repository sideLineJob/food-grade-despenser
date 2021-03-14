#ifndef SERVO_CONTROLLER_h
#define SERVO_CONTROLLER_h

#include <Arduino.h>
#include <Servo.h>

class SERVO_CONTROLLER
{
  public:
    void servoInit();
    void closeContainer();
    void openContainer();
    void serialControlDispenser();
    boolean stopDispensing(float loadValue, float stopValue);
  private:
    Servo containerServo;
    float loadStopValue = 65; // load test stop value
};

#endif
