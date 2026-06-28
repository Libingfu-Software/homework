// ex05 - Touch-controlled multi-speed breathing light

const int touchPin = T0;  // ESP32 touch channel T0 (GPIO4 on most boards)
const int ledPin = 2;     // Onboard LED

const int touchThreshold = 300;
const unsigned long debounceDelay = 50;

const int maxLevel = 3;
const int stepSizes[maxLevel] = {1, 3, 6};

bool lastRawTouchState = false;
bool stableTouchState = false;
unsigned long lastDebounceTime = 0;

int currentLevel = 0;   // 0, 1, 2 -> levels 1, 2, 3
int brightness = 0;
int direction = 1;

void setup() {
	Serial.begin(115200);
	pinMode(ledPin, OUTPUT);

	int initialTouchValue = touchRead(touchPin);
	stableTouchState = (initialTouchValue < touchThreshold);
	lastRawTouchState = stableTouchState;

	ledcAttach(ledPin, 5000, 8);
	ledcWrite(ledPin, 0);

	Serial.println("ex05 touch breathing light ready");
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
				currentLevel = (currentLevel + 1) % maxLevel;
				Serial.print("Speed level changed to: ");
				Serial.println(currentLevel + 1);
			}
		}
	}

	int step = stepSizes[currentLevel];
	brightness += direction * step;

	if (brightness >= 255) {
		brightness = 255;
		direction = -1;
	} else if (brightness <= 0) {
		brightness = 0;
		direction = 1;
	}

	ledcWrite(ledPin, brightness);
	delay(5);
}
