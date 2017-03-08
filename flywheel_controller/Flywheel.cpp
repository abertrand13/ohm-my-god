/*=============================================================================

Module
Flywheel.cpp
Description
Library for motors

=============================================================================*/

/*---------------------------Dependencies------------------------------------*/
#include "Flywheel.h"

/*---------------------------Module Variables--------------------------------*/

char motorSpeed;
char feedMotorSpeed;

/*===========================Module Code=====================================*/

void applyMotorSettings(void) {
    updateDutyCycle();
	analogWrite(PIN_FEED_EN, feedMotorSpeed);
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

void setFeedMotorSpeed(char val) {
  feedMotorSpeed = constrain(val, 0, 100);
}

void stopFeedMotor() {
  feedMotorSpeed = 0;
}

void setupMotorPins(void) {
    pinMode(PIN_FLY_EN, OUTPUT);
    pinMode(PIN_FLY_A, OUTPUT);
    pinMode(PIN_FLY_B, OUTPUT);

    digitalWrite(PIN_FLY_A, LOW);
    digitalWrite(PIN_FLY_B, HIGH);

	pinMode(PIN_FEED_EN, OUTPUT);
	pinMode(PIN_FEED_A, OUTPUT);
	pinMode(PIN_FEED_B, OUTPUT);

	digitalWrite(PIN_FEED_A, HIGH);
	digitalWrite(PIN_FEED_B, LOW);
}

void updateDutyCycle() {
  int time = millis() % 100;
  if(time < motorSpeed) {
    digitalWrite(PIN_FLY_EN, HIGH);
  } else {
    digitalWrite(PIN_FLY_EN, LOW);
  }
}



