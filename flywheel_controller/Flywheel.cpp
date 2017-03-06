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
    updateDutyCycle();
}

char getFlywheelMotorSpeed(void) {
  return motorSpeed;
}

void setFlywheelMotorSpeed(char val) {
  motorSpeed = constrain(val, 0, 100);
}

void stopFlywheelMotor(void) {
  motorSpeed = 0;
}

void setupMotorPins(void) {
    pinMode(PIN_FLY_EN, OUTPUT);
    pinMode(PIN_FLY_A, OUTPUT);
    pinMode(PIN_FLY_B, OUTPUT);

    digitalWrite(PIN_FLY_A, LOW);
    digitalWrite(PIN_FLY_B, HIGH);
}

void updateDutyCycle() {
  int time = millis() % 100;
  if(time < motorSpeed) {
    digitalWrite(PIN_FLY_EN, HIGH);
  } else {
    digitalWrite(PIN_FLY_EN, LOW);
  }
}



