/*=============================================================================
 
  Motors.h

=============================================================================*/

#include "Arduino.h"

/*---------------Module Defines-----------------------------*/

// Pinout
#define PIN_POT 3           	// Input pin for potentiometer

#define PIN_MBACK_EN 11    	// Output pin for H Bridge
#define PIN_MBACK_A 12			// One side of drive
#define PIN_MBACK_B 13			// Other side of drive

#define PIN_MRIGHT_EN 10
#define PIN_MRIGHT_A 9
#define PIN_MRIGHT_B 8

#define PIN_MFRONT_EN 5
#define PIN_MFRONT_A 6
#define PIN_MFRONT_B 7

#define PIN_MLEFT_EN 3
#define PIN_MLEFT_A 2
#define PIN_MLEFT_B 4

// Constants
#define MOTORS 4
#define PINS_PER_MOTOR 3

// Timers
#define TIMER_0 0
#define INTERVAL_0 2000

/*---------------------------Public Functions--------------------------------*/
// Motor Reference
enum motorID {
  MLEFT,
  MRIGHT,
  MFRONT,
  MBACK
};

void applyMotorSettings(void);
bool getMotorForward(int motor);
void setMotorSpeed(int motor, char val);
char getMotorSpeed(int motor);
void moveLeft(char val);
void moveRight(char val);
void moveBack(char val);
void moveForward(char val);
void stopDriveMotors(void);
void flipMotorDirection(int motor);
void setupMotorPins(void);
