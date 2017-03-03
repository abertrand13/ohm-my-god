/*=============================================================================

Module
Motors.cpp
Description
Library for motors

=============================================================================*/

/*---------------------------Dependencies------------------------------------*/
#include "Motors.h"


/*---------------------------Module Variables--------------------------------*/

char motorPins[MOTORS][PINS_PER_MOTOR] = {
  {PIN_MLEFT_EN, PIN_MLEFT_A, PIN_MLEFT_B},
  {PIN_MRIGHT_EN, PIN_MRIGHT_B, PIN_MRIGHT_A},
  {PIN_MFRONT_EN, PIN_MFRONT_B, PIN_MFRONT_A},
  {PIN_MBACK_EN, PIN_MBACK_B, PIN_MBACK_A}
};

char motorSpeeds[MOTORS];

/*===========================Module Code=====================================*/

void applyMotorSettings() {
  for(int i = 0;i < MOTORS;i++) {
    // write speed
    analogWrite(motorPins[i][0], abs(motorSpeeds[i]) * 2);
    // write direction
    digitalWrite(motorPins[i][1], getMotorForward(i) ? HIGH : LOW);
    digitalWrite(motorPins[i][2], getMotorForward(i) ? LOW : HIGH);
  }
}

bool getMotorForward(int motor) {
  return motorSpeeds[motor] > 0;
}

char getMotorSpeed(int motor) {
  return motorSpeeds[motor];
}

void setMotorSpeed(int motor, char val) {
  motorSpeeds[motor] = constrain(val, -127, 127);
}

void moveLeft(char val) {
  char speedVal = constrain(val, -127, 127);
  motorSpeeds[MFRONT] = -speedVal;
  motorSpeeds[MBACK] = -speedVal;
}

void moveRight(char val) {
  char speedVal = constrain(val, -127, 127);
  motorSpeeds[MFRONT] = speedVal;
  motorSpeeds[MBACK] = speedVal;
}

void moveBack(char val) {
  char speedVal = constrain(val, -127, 127);
  motorSpeeds[MLEFT] = -speedVal;
  motorSpeeds[MRIGHT] = -speedVal;
}

void moveForward(char val) {
  char speedVal = constrain(val, -127, 127);
  motorSpeeds[MLEFT] = speedVal;
  motorSpeeds[MRIGHT] = speedVal;
}

void stopDriveMotors(void) {
  for(int i = 0;i < MOTORS;i++) {
    motorSpeeds[i] = 0;
  }
}

void flipMotorDirection(int motor) {
  motorSpeeds[motor] = map(motorSpeeds[motor], -127, 127, 127, -127);
}

void setupMotorPins(void) {
  pinMode(PIN_POT, INPUT);

  for(int i = 0;i < MOTORS;i++) {
    for(int j = 0;j < PINS_PER_MOTOR;j++) {
      pinMode(motorPins[i][j], OUTPUT);
    }
  }
}

