#include <Arduino.h>

const int ledPinA = 18;
const int ledPinB = 19;

const int pwmFreq = 5000;
const int pwmResolution = 8;

int brightness = 0;
int direction = 1;

void setup() {
  ledcAttach(ledPinA, pwmFreq, pwmResolution);
  ledcAttach(ledPinB, pwmFreq, pwmResolution);

  ledcWrite(ledPinA, 0);
  ledcWrite(ledPinB, 255);
}

void loop() {
  ledcWrite(ledPinA, brightness);
  ledcWrite(ledPinB, 255 - brightness);

  brightness += direction;
  if (brightness >= 255) {
    brightness = 255;
    direction = -1;
  } else if (brightness <= 0) {
    brightness = 0;
    direction = 1;
  }

  delay(10);
}
