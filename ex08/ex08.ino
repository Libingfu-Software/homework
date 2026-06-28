#include <WiFi.h>
#include <WebServer.h>

const int ledPin = 2;
const int touchPin = T0;

const int touchThreshold = 300;
const unsigned long debounceDelay = 50;
const unsigned long alarmBlinkInterval = 120;

// 替换为 STA 模式的 WiFi 凭据
const char* wifiSsid = "lisi";
const char* wifiPassword = "1564922632";

WebServer server(80);

bool armed = false;
bool alarmActive = false;
bool lastRawTouchState = false;
bool stableTouchState = false;
unsigned long lastDebounceTime = 0;
unsigned long lastBlinkTime = 0;
bool ledState = LOW;

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
    <p>Press Arm to enable alarm mode. Touch the sensor to trigger a latched alarm.</p>
    <div class="status" id="status">Status: loading...</div>
    <div class="buttons">
      <button class="arm" onclick="setState('arm')">Arm</button>
      <button class="disarm" onclick="setState('disarm')">Disarm</button>
    </div>
  </div>
  <script>
    const statusBox = document.getElementById('status');

    function refreshStatus() {
      fetch('/status')
        .then(response => response.text())
        .then(text => { statusBox.textContent = text; })
        .catch(() => { statusBox.textContent = 'Status: offline'; });
    }

    function setState(action) {
      fetch('/' + action)
        .then(() => refreshStatus())
        .catch(() => {});
    }

    refreshStatus();
    setInterval(refreshStatus, 1000);
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

  // 改为 STA 模式并连接指定 WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSsid, wifiPassword);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/arm", handleArm);
  server.on("/disarm", handleDisarm);
  server.begin();

  Serial.println("Security alarm ready");
  Serial.print("Open: http://");
  Serial.println(WiFi.localIP());
}

void loop() {
  server.handleClient();

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