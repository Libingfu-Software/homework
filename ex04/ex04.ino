#include <Arduino.h>

// ex04 - Touch sensor self-lock switch for ESP32 + Arduino
// Touch once to toggle the LED, then keep the new state until the next valid touch.

const int touchPin = T0;  // ESP32 touch channel T0 (GPIO4 on most boards)
const int ledPin = 2;     // Onboard LED

const int touchThreshold = 300;
const unsigned long debounceDelay = 50;

bool ledState = LOW;
bool lastRawTouchState = false;
bool stableTouchState = false;
unsigned long lastDebounceTime = 0;

void setup() {
	Serial.begin(115200);
	pinMode(ledPin, OUTPUT);

	// Initialize the touch state from the current reading to avoid a false toggle at boot.
	int initialTouchValue = touchRead(touchPin);
	stableTouchState = (initialTouchValue < touchThreshold);
	lastRawTouchState = stableTouchState;

	digitalWrite(ledPin, ledState);

	Serial.println("ex04 touch self-lock ready");
	Serial.print("Initial touch value: ");
	Serial.println(initialTouchValue);
}

void loop() {
	int touchValue = touchRead(touchPin);
	bool rawTouchState = (touchValue < touchThreshold);

	if (rawTouchState != lastRawTouchState) {
		lastDebounceTime = millis();
		lastRawTouchState = rawTouchState;
	}

	if ((millis() - lastDebounceTime) > debounceDelay) {
		if (rawTouchState != stableTouchState) {
			bool previousStableTouchState = stableTouchState;
			stableTouchState = rawTouchState;

			if (stableTouchState && !previousStableTouchState) {
				ledState = !ledState;
				digitalWrite(ledPin, ledState);

				Serial.print("Touch toggled LED to: ");
				Serial.println(ledState ? "ON" : "OFF");
			}
		}
	}

	delay(5);
}
