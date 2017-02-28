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
#define PIN_POT 3             // Input pin for potentiometer

#define PIN_MENCODER_DRIVE 9
#define PIN_MENCODER_A 3
#define PIN_MENCODER_B 2

/*---------------Module Function Prototypes-----------------*/
void setupPins(void);
void setupInterrupts(void);
void quadratureA(void);
void quadratureB(void);

/*---------------Module Variables---------------------------*/

int timeA;
int timeB;

/*---------------Main Functions-----------------------------*/
void setup() {
  Serial.begin(9600);
  setupPins();
  setupInterrupts();
}

void loop() { 
  int potVal = analogRead(PIN_POT);
  int val = potVal/4;
  analogWrite(PIN_MENCODER_DRIVE, val);
}

void quadratureA() {
  timeA = micros();
}

void quadratureB() {
  timeB = micros();
  int time = timeB - timeA;
  float hz = 1000000.0 / (time * 4);
  int rpm = hz * 60;
  Serial.println(rpm);
}

void setupPins() {
  pinMode(PIN_MENCODER_DRIVE, OUTPUT);
  
  pinMode(PIN_POT, INPUT);
  pinMode(PIN_MENCODER_A, INPUT);
  pinMode(PIN_MENCODER_B, INPUT);
}

void setupInterrupts() {
  attachInterrupt(digitalPinToInterrupt(PIN_MENCODER_A), quadratureA, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_MENCODER_B), quadratureB, FALLING);
}

