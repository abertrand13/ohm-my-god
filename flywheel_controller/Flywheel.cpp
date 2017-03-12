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
int ballsFed;
int ballsToFeed;
bool continueFeeding;

/*===========================Module Code=====================================*/

void applyMotorSettings(void) {
    updateDutyCycle();
	/*analogWrite(PIN_FEED_EN, feedMotorSpeed);
	digitalWrite(PIN_FEED_A, feedMotorSpeed > 0 ? HIGH : LOW);
	digitalWrite(PIN_FEED_B, feedMotorSpeed > 0 ? LOW : HIGH);*/
    updateFeedMotor();
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

/*void setFeedMotorSpeed(char val) {
  feedMotorSpeed = constrain(val, 0, 100);
}

void stopFeedMotor() {
  feedMotorSpeed = 0;
}*/

void feedBalls(int numBalls) {
  ballsFed = 0;
  ballsToFeed = numBalls;
  continueFeeding = true;
}

bool doneFeeding() {
  return !continueFeeding;
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

void updateFeedMotor() {
  int time = millis() % FEED_PERIOD;
  if(time < FEED_TIME && continueFeeding) {
    digitalWrite(PIN_FEED_EN, HIGH);
  } else {
    if(digitalRead(PIN_FEED_EN)) {
      ballsFed++;
    }
    if(ballsFed == ballsToFeed) {
      continueFeeding = false;
    }
    digitalWrite(PIN_FEED_EN, LOW);
  }
}

