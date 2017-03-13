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
char motorSpeedsQueue[MOTORS];

bool braking;
int brakeTime;
bool queued;

/*===========================Module Code=====================================*/

void applyMotorSettings() {
  for(int i = 0;i < MOTORS;i++) {
    // write speed
    analogWrite(motorPins[i][0], abs(motorSpeeds[i]) * 2);
    // write direction
    digitalWrite(motorPins[i][1], getMotorForward(i) ? HIGH : LOW);
    digitalWrite(motorPins[i][2], getMotorForward(i) ? LOW : HIGH);
  }

  if(braking && ((millis() - brakeTime) > BRAKE_TIME)) {  
    braking = false;
    stopDriveMotors();

    // after braking is done, copy values from queue
    if(queued) {
      for(int i = 0;i < MOTORS;i++) {
        motorSpeeds[i] = motorSpeedsQueue[i];
      }
      queued = false;
    }
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
  if(braking) {
    motorSpeedsQueue[MFRONT] = -speedVal;
    motorSpeedsQueue[MBACK] = -speedVal;
    queued = true;
  } else {
    motorSpeeds[MFRONT] = -speedVal;
    motorSpeeds[MBACK] = -speedVal;
  } 
}

void moveRight(char val) {
  char speedVal = constrain(val, -127, 127);
  if(braking) {
    motorSpeedsQueue[MFRONT] = speedVal;
    motorSpeedsQueue[MBACK] = speedVal;
    queued = true;
  } else {
    motorSpeeds[MFRONT] = speedVal;
    motorSpeeds[MBACK] = speedVal;
  }
}

void moveBack(char val) {
  char speedVal = constrain(val, -127, 127);
  if(braking) { 
    motorSpeedsQueue[MLEFT] = -speedVal;
    motorSpeedsQueue[MRIGHT] = -speedVal;
    queued = true;
  } else { 
    motorSpeeds[MLEFT] = -speedVal;
    motorSpeeds[MRIGHT] = -speedVal;
  }
}

void moveForward(char val) {
  char speedVal = constrain(val, -127, 127);
  if(braking) {
    motorSpeedsQueue[MLEFT] = speedVal;
    motorSpeedsQueue[MRIGHT] = speedVal;
    queued = true;
  } else {
    motorSpeeds[MLEFT] = speedVal;
    motorSpeeds[MRIGHT] = speedVal;
  }
}

void turnCW(char val) {
  char speedVal = constrain(val, -127, 127);
  motorSpeeds[MLEFT] = speedVal;
  motorSpeeds[MFRONT] = speedVal;
  motorSpeeds[MRIGHT] = -speedVal;
  motorSpeeds[MBACK] = -speedVal;
}

void turnCCW(char val) {
  char speedVal = constrain(val, -127, 127);
  motorSpeeds[MLEFT] = -speedVal;
  motorSpeeds[MFRONT] = -speedVal;
  motorSpeeds[MRIGHT] = speedVal;
  motorSpeeds[MBACK] = speedVal;
}

void stopDriveMotors(void) {
  for(int i = 0;i < MOTORS;i++) {
    motorSpeeds[i] = 0;
  }
}

void hardBrake() {
  // send a short reverse pulse
  for(int i = 0;i < MOTORS;i++) {
    flipMotorDirection(i);
  }
  brakeTime = millis(); 
  braking = true;
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

  braking = false;
}

