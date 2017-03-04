/*=============================================================================

Module
Flywheel.cpp
Description
Library for motors

=============================================================================*/

/*---------------------------Dependencies------------------------------------*/
#include "Flywheel.h"


/*---------------------------Module Variables--------------------------------*/

char motorPins[PINS_PER_MOTOR] = {
  PIN_FLY_EN, 
  PIN_FLY_A, 
  PIN_FLY_B
};

char motorSpeed;

/*===========================Module Code=====================================*/

void applyMotorSettings(void) {
    // write speed
    analogWrite(motorPins[0], abs(motorSpeed) * 2);
    // write direction
    digitalWrite(motorPins[1], getFlywheelMotorForward() ? HIGH : LOW);
    digitalWrite(motorPins[2], getFlywheelMotorForward() ? LOW : HIGH);
}

bool getFlywheelMotorForward(void) {
  return motorSpeed > 0;
}

char getFlywheelMotorSpeed(void) {
  return motorSpeed;
}

void setFlywheelMotorSpeed(char val) {
  motorSpeed = constrain(val, -127, 127);
}

void stopFlywheelMotor(void) {
  motorSpeed = 0;
}

void flipFlywheelDirection(void) {
  motorSpeed -= motorSpeed;
}

void setupMotorPins(void) {
  for(int i = 0;i < PINS_PER_MOTOR;i++) {
    pinMode(motorPins[i], OUTPUT);
  }
}

