#include <WiFi.h>
#include <WebServer.h>

const int ledPin = 2;
const int touchPin = T0;

const int touchThreshold = 300;
const unsigned long debounceDelay = 50;
const unsigned long alarmBlinkInterval = 120;

const char* apSsid = "ESP32-2024117201";
const char* apPassword = "12345678";

WebServer server(80);

bool armed = false;
bool alarmActive = false;
bool lastRawTouchState = false;
bool stableTouchState = false;
unsigned long lastDebounceTime = 0;
unsigned long lastBlinkTime = 0;
bool ledState = LOW;
int displayedTouchValue = -1;

String buildPage() {
  return R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 Security Alarm</title>
  <style>
    body {
      margin: 0;
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      background: linear-gradient(135deg, #111827, #1f2937);
      font-family: Arial, sans-serif;
      color: #e5e7eb;
    }
    .card {
      width: min(92vw, 420px);
      padding: 24px;
      border-radius: 20px;
      background: rgba(17, 24, 39, 0.9);
      border: 1px solid rgba(148, 163, 184, 0.2);
      box-shadow: 0 20px 60px rgba(0, 0, 0, 0.4);
    }
    h1 {
      margin: 0 0 10px;
      font-size: 1.5rem;
    }
    p {
      margin: 0 0 18px;
      color: #94a3b8;
      line-height: 1.5;
    }
    .status {
      margin-bottom: 16px;
      padding: 12px;
      border-radius: 12px;
      background: rgba(51, 65, 85, 0.65);
    }
    .buttons {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 12px;
    }
    .touch {
      margin-top: 12px;
      padding: 12px;
      border-radius: 12px;
      background: rgba(15, 23, 42, 0.85);
      color: #fbbf24;
      font-weight: 700;
    }
    button {
      border: 0;
      border-radius: 12px;
      padding: 14px 16px;
      font-size: 1rem;
      font-weight: 700;
      cursor: pointer;
      color: white;
    }
    .arm { background: #16a34a; }
    .disarm { background: #dc2626; }
  </style>
</head>
<body>
  <div class="card">
    <h1>ESP32 Security Alarm</h1>
    <p>Connect to the ESP32 AP and use the buttons below. Touch the sensor to trigger a latched alarm.</p>
    <div class="status" id="status">Status: loading...</div>
    <div class="touch" id="touch">Touch: waiting...</div>
    <div class="buttons">
      <button class="arm" onclick="setState('arm')">Arm</button>
      <button class="disarm" onclick="setState('disarm')">Disarm</button>
    </div>
  </div>
  <script>
    const statusBox = document.getElementById('status');
    const touchBox = document.getElementById('touch');

    function refreshStatus() {
      fetch('/status')
        .then(response => response.text())
        .then(text => { statusBox.textContent = text; })
        .catch(() => { statusBox.textContent = 'Status: offline'; });
    }

    function refreshTouch() {
      fetch('/touch')
        .then(response => response.text())
        .then(text => { touchBox.textContent = text; })
        .catch(() => { touchBox.textContent = 'Touch: offline'; });
    }

    function setState(action) {
      fetch('/' + action)
        .then(() => {
          refreshStatus();
          refreshTouch();
        })
        .catch(() => {});
    }

    refreshStatus();
    refreshTouch();
    setInterval(refreshStatus, 1000);
    setInterval(refreshTouch, 1000);
  </script>
</body>
</html>
)rawliteral";
}

void updateStatusLedOff() {
  ledState = LOW;
  digitalWrite(ledPin, ledState);
}

void handleRoot() {
  server.send(200, "text/html; charset=utf-8", buildPage());
}

void handleStatus() {
  String status = "Status: ";
  if (alarmActive) {
    status += "ALARM";
  } else if (armed) {
    status += "ARMED";
  } else {
    status += "DISARMED";
  }

  server.send(200, "text/plain; charset=utf-8", status);
}

void handleTouch() {
  String touch = "Touch: ";
  if (displayedTouchValue >= 0) {
    touch += String(displayedTouchValue);
  } else {
    touch += "waiting...";
  }

  server.send(200, "text/plain; charset=utf-8", touch);
}

void handleArm() {
  armed = true;
  server.send(200, "text/plain; charset=utf-8", "ARMED");
}

void handleDisarm() {
  armed = false;
  alarmActive = false;
  lastRawTouchState = false;
  stableTouchState = false;
  lastDebounceTime = 0;
  lastBlinkTime = 0;
  updateStatusLedOff();
  server.send(200, "text/plain; charset=utf-8", "DISARMED");
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  updateStatusLedOff();

  int initialTouchValue = touchRead(touchPin);
  stableTouchState = (initialTouchValue < touchThreshold);
  lastRawTouchState = stableTouchState;
  if (initialTouchValue < touchThreshold) {
    displayedTouchValue = initialTouchValue;
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSsid, apPassword);
  IPAddress apIP = WiFi.softAPIP();
  Serial.println("AP mode started");
  Serial.print("SSID: ");
  Serial.println(apSsid);
  Serial.print("IP address: ");
  Serial.println(apIP);

  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/touch", handleTouch);
  server.on("/arm", handleArm);
  server.on("/disarm", handleDisarm);
  server.begin();

  Serial.println("Security alarm ready");
  Serial.print("Open: http://");
  Serial.println(apIP);
}

void loop() {
  server.handleClient();

  int touchValue = touchRead(touchPin);
  bool rawTouchState = (touchValue < touchThreshold);

  if (touchValue < touchThreshold) {
    displayedTouchValue = touchValue;
  }

  if (rawTouchState != lastRawTouchState) {
    lastDebounceTime = millis();
    lastRawTouchState = rawTouchState;
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (rawTouchState != stableTouchState) {
      bool previousStableTouchState = stableTouchState;
      stableTouchState = rawTouchState;

      if (armed && stableTouchState && !previousStableTouchState) {
        alarmActive = true;
      }
    }
  }

  if (alarmActive) {
    if (millis() - lastBlinkTime >= alarmBlinkInterval) {
      lastBlinkTime = millis();
      ledState = !ledState;
      digitalWrite(ledPin, ledState);
    }
  } else {
    updateStatusLedOff();
  }
}