//////////////////////////
//                      //
// ME 210 FINAL PROJECT //
// -------------------- //
// Chris Kimes          //
// Eni Asebiomo         //
// Cuthbert Sun         //
// Alex Bertrand        //
//                      //
//////////////////////////

/*---------------Includes-----------------------------------*/

#include <Timers.h>

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

// Motor Reference
enum motorID {
  MLEFT,
  MRIGHT,
  MFRONT,
  MBACK
};

/*---------------Module Function Prototypes-----------------*/

void setupPins(void);
bool getMotorForward(int motor);
char getMotorSpeed(int motor);
void setMotorSpeed(int motor, char val);
void moveLeft(char val);
void moveRight(char val);
void moveBack(char val);
void moveForward(char val);
void stopDriveMotors(void);
void applyMotorSettings(void);
void handleTimer(void);

// Logic functions (other file)
bool checkIRAlign(void);
bool checkLeftLimitSwitchesAligned(void);
bool checkFrontLimitSwitchesAligned(void);
bool checkTape(void);
void handleIRAlign(void);
void handleTimerExpired(void);
void handleLeftLimitSwitchesAligned(void);
void handleFrontLimitSwitchesAligned(void);
void handleTape(void);
void handleReturnedLeft(void);
void handleReturnTimerExpired(void);
void handleRefillTimerExpired(void);
void setupSensorPins(void);

/*---------------Module Variables---------------------------*/

char motorPins[MOTORS][PINS_PER_MOTOR] = {
  {PIN_MLEFT_EN, PIN_MLEFT_A, PIN_MLEFT_B},
  {PIN_MRIGHT_EN, PIN_MRIGHT_B, PIN_MRIGHT_A},
  {PIN_MFRONT_EN, PIN_MFRONT_A, PIN_MFRONT_B},
  {PIN_MBACK_EN, PIN_MBACK_B, PIN_MBACK_A}
};
char motorSpeeds[MOTORS];


/*---------------Main Functions-----------------------------*/
void setup() {
  Serial.begin(9600);
  setupPins();
  setupSensorPins();

  //TMRArd_InitTimer(TIMER_0, INTERVAL_0);
  setMotorSpeed(MLEFT, 100);
  setMotorSpeed(MRIGHT, 100);
}

void loop() { 
  // control routine (temporary)
  // to operate, just enter commands in the serial port
  // eg:
  // l127 sets left motor to full forward speed
  // l-127 sets left motor to full reverse speed
  // f127 set front motor to full forward speed
  // b-128 reverses the back motor, whether it's going forward or backward
  if(Serial.available()) {
    char motor = Serial.read();
    enum motorID select;
    if(motor == 'l') {
      select = MLEFT;
    } else if(motor == 'r'){
      select = MRIGHT;
    } else if(motor == 'b'){
      select = MBACK;
    } else if(motor == 'f'){
      select = MFRONT;
    }
    int x = Serial.parseInt();
    Serial.println(x);
    if(x != -128) {
      setMotorSpeed(select, char(x));
    } else {
      flipMotorDirection(select);
    }
    Serial.read(); //newline
  }

  if(TMRArd_IsTimerExpired(TIMER_0)) {
    handleTimer();
  }

  checkEvents();

  applyMotorSettings();
}

void handleTimer() { 
  flipMotorDirection(MLEFT);
  flipMotorDirection(MRIGHT);
  TMRArd_ClearTimerExpired(TIMER_0);
  TMRArd_InitTimer(TIMER_0, INTERVAL_0);
}

/* MOTOR API */
// ---------
// applyMotorSettings(void)
// Writes to output pins to enforce the motor speeds and directions as they have been set.
// 
// setMotorSpeed(int motor, char val)
// Takes a motor (one of MLEFT, MRIGHT, MFRONT, MBACK) and a speed [-127, 127]


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

void stopDriveMotors() {
  for(int i = 0;i < MOTORS;i++) {
    motorSpeeds[i] = 0;
  }
}

void flipMotorDirection(int motor) {
  motorSpeeds[motor] = map(motorSpeeds[motor], -127, 127, 127, -127);
}

void setupPins() {
  pinMode(PIN_POT,      INPUT);

  for(int i = 0;i < MOTORS;i++) {
    for(int j = 0;j < PINS_PER_MOTOR;j++) {
      pinMode(motorPins[i][j], OUTPUT);
    }
  }
}
