const int touchPin = T0;
const int ledPin = 2;

bool ledState = false;
bool lastTouchState = false;
bool currentTouchState = false;

const unsigned long debounceDelay = 50;
unsigned long lastDebounceTime = 0;

const int touchThreshold = 30;  // 根据打印值调整

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
}

void loop() {
  int touchValue = touchRead(touchPin);
  bool rawState = (touchValue < touchThreshold);

  // 打印调试信息
  Serial.print("Value: ");
  Serial.print(touchValue);
  Serial.print("  rawState: ");
  Serial.println(rawState ? "TOUCH" : "release");

  if (rawState != lastTouchState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (rawState != currentTouchState) {
      currentTouchState = rawState;
      
      if (currentTouchState == true && lastTouchState == false) {
        ledState = !ledState;
        digitalWrite(ledPin, ledState ? HIGH : LOW);
        Serial.println("--- LED TOGGLED ---");
      }
    }
  }

  lastTouchState = rawState;
  delay(200);  // 打印间隔，方便观察
}