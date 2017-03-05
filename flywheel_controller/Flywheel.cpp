/*=============================================================================

Module
Flywheel.cpp
Description
Library for motors

=============================================================================*/

/*---------------------------Dependencies------------------------------------*/
#include "Flywheel.h"
#include <Pulse.h>


/*---------------------------Module Variables--------------------------------*/

char motorPins[PINS_PER_MOTOR] = {
  PIN_FLY_EN, 
  PIN_FLY_A, 
  PIN_FLY_B
};

char motorSpeed;
static uint8_t signalToggle;

/*===========================Module Code=====================================*/

void applyMotorSettings(void) {
    // write speed
    // analogWrite(motorPins[0], abs(motorSpeed));
    updatePWM();
}

char getFlywheelMotorSpeed(void) {
  return motorSpeed;
}

void setFlywheelMotorSpeed(char val) {
  motorSpeed = constrain(val, 10, 1000);
}

void stopFlywheelMotor(void) {
  motorSpeed = 0;
}

void setupMotorPins(void) {
    signalToggle = 0;

    pinMode(PIN_FLY_EN, OUTPUT);
    pinMode(PIN_FLY_A, OUTPUT);
    pinMode(PIN_FLY_B, OUTPUT);

    digitalWrite(PIN_FLY_A, LOW);
    digitalWrite(PIN_FLY_B, HIGH);
}

void updatePWM(void) {
  if(IsPulseFinished()) { 
      InitPulse(PIN_FLY_EN, motorSpeed);
      Pulse(10);
  }

}

/******************************************************************************
  Function:    SetupTimerInterrupt
  Contents:    This function sets up the necessary registers to use Timer2 to set
               resolution at which the timer operates
  Parameters:  None
  Returns:     None
  Notes:       None
******************************************************************************/
void SetupTimerInterrupt() {
  cli();                               // Stop interrupts
  TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM12) | _BV(WGM11) | _BV(WGM10);
  OCR1A = 180;
  sei();                               //Allow interrupts
}

ISR(TIMER2_COMPA_vect) {
  if (signalToggle) {
    digitalWrite(PIN_FLY_EN, HIGH);
    signalToggle = 0;
  }
  else {
    digitalWrite(PIN_FLY_EN, LOW);
    signalToggle = 1;
  }
}

