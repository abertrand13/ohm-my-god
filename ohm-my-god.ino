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
#define PIN_POT 3           // Input pin for potentiometer

#define PIN_MLEFT_ENABLE 11    // Output pin for H Bridge
#define PIN_MLEFT_A 12
#define PIN_MLEFT_B 13


#define PIN_MRIGHT_ENABLE 10
#define PIN_MRIGHT_A 9
#define PIN_MRIGHT_B 8

// Constants
#define MOTORS 2
#define PINS_PER_MOTOR 3

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
void applyMotorSettings(void);

/*---------------Module Variables---------------------------*/

char motorPins[MOTORS][PINS_PER_MOTOR] = {
  {PIN_MLEFT_ENABLE, PIN_MLEFT_A, PIN_MLEFT_B},
  {PIN_MRIGHT_ENABLE, PIN_MRIGHT_A, PIN_MRIGHT_B}
};
char motorSpeeds[MOTORS];

/*---------------Lab 1 Main Functions-----------------------*/
void setup() {
  Serial.begin(9600);
  setupPins();

  digitalWrite(PIN_MLEFT_A, HIGH);
  digitalWrite(PIN_MLEFT_B, LOW);
}

void loop() {
  /*int pot_val = analogRead(PIN_POT);
  int val = map(pot_val, 0, 1023, 0, 127);
  setMotorSpeed(MLEFT, val);*/
  
  if(Serial.available()) {
    char motor = Serial.read();
    enum motorID select;
    if(motor == 'l') {
      select = MLEFT;
    } else {
      select = MRIGHT;
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

  applyMotorSettings();
}

/* MOTOR API */
// setMotorSpeed(

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

void flipMotorDirection(int motor) {
  motorSpeeds[motor] = map(motorSpeeds[motor], -127, 127, 127, -127);
}

void setupPins() {
  pinMode(PIN_POT,        INPUT);

  for(int i = 0;i < MOTORS;i++) {
    for(int j = 0;j < PINS_PER_MOTOR;j++) {
      pinMode(motorPins[i][j], OUTPUT);
    }
  }
}
