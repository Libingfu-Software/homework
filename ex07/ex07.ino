#include <WiFi.h>
#include <WebServer.h>

const int ledPin = 2;
const int pwmFrequency = 5000;
const int pwmResolution = 8;

const char* wifiSsid = "lisi";
const char* wifiPassword = "1564922632";

WebServer server(80);
int brightness = 0;

String buildPage() {
  return R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 Web Dimmer</title>
  <style>
    body {
      margin: 0;
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      background: linear-gradient(135deg, #0f172a, #1e293b);
      color: #e2e8f0;
      font-family: Arial, sans-serif;
    }
    .card {
      width: min(92vw, 420px);
      padding: 24px;
      border-radius: 20px;
      background: rgba(15, 23, 42, 0.86);
      border: 1px solid rgba(148, 163, 184, 0.18);
      box-shadow: 0 20px 60px rgba(0, 0, 0, 0.35);
    }
    h1 {
      margin: 0 0 10px;
      font-size: 1.4rem;
    }
    p {
      margin: 0 0 18px;
      color: #94a3b8;
    }
    .value {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 14px;
    }
    input[type="range"] {
      width: 100%;
      accent-color: #38bdf8;
    }
    .tip {
      margin-top: 14px;
      font-size: 0.9rem;
      color: #cbd5e1;
    }
  </style>
</head>
<body>
  <div class="card">
    <h1>ESP32 Web Dimmer</h1>
    <p>Drag the slider to change the LED brightness in real time.</p>
    <div class="value">
      <span>Brightness</span>
      <strong id="val">0</strong>
    </div>
    <input id="slider" type="range" min="0" max="255" value="0">
    <div class="tip">The webpage controls the ESP32 PWM output directly.</div>
  </div>
  <script>
    const slider = document.getElementById('slider');
    const val = document.getElementById('val');
    let timer = null;

    function sendBrightness(value) {
      fetch('/set?value=' + value).catch(() => {});
    }

    slider.addEventListener('input', () => {
      val.textContent = slider.value;
      clearTimeout(timer);
      timer = setTimeout(() => sendBrightness(slider.value), 20);
    });
  </script>
</body>
</html>
)rawliteral";
}

void handleRoot() {
  server.send(200, "text/html; charset=utf-8", buildPage());
}

void handleSet() {
  if (server.hasArg("value")) {
    brightness = constrain(server.arg("value").toInt(), 0, 255);
    ledcWrite(ledPin, brightness);
  }

  server.send(200, "text/plain; charset=utf-8", String(brightness));
}

void setup() {
  Serial.begin(115200);

  ledcAttach(ledPin, pwmFrequency, pwmResolution);
  ledcWrite(ledPin, brightness);

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSsid, wifiPassword);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.begin();

  Serial.println("Web dimmer ready");
  Serial.print("Open this address in your browser: http://");
  Serial.println(WiFi.localIP());
}

void loop() {
  server.handleClient();
}
