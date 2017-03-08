#include <Signals.h>
#include <Timers.h>


enum signal inputSignal;

void setup() {
	pinMode(13, OUTPUT);
	Serial.println('Setup!');
	digitalWrite(13, HIGH);
	delay(100);
	digitalWrite(13, LOW);
	inputSignal = NONE;
}

void loop() {
	inputSignal = receiveSignal();
	if(inputSignal == ALIGNED) {
		digitalWrite(13, HIGH);
		delay(100);
		digitalWrite(13, LOW);
		delay(100);
		digitalWrite(13, HIGH);
		delay(100);
		digitalWrite(13, LOW);
	}
}