#include <WiFi.h>
#include <WebServer.h>

// ---------- 三个 LED 引脚 ----------
const int ledPins[3] = {18, 4, 5};     
const int pwmFrequency = 5000;
const int pwmResolution = 8;

const char* apSsid = "ESP32_2024117201";
const char* apPassword = "12345678";

WebServer server(80);
int brightness[3] = {0, 0, 0};


const char index_html[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 Triple LED Dimmer</title>
  <style>
    * { box-sizing: border-box; }
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
      width: min(92vw, 480px);
      padding: 24px;
      border-radius: 20px;
      background: rgba(15, 23, 42, 0.86);
      border: 1px solid rgba(148, 163, 184, 0.18);
      box-shadow: 0 20px 60px rgba(0, 0, 0, 0.35);
    }
    h1 {
      margin: 0 0 6px;
      font-size: 1.5rem;
    }
    .sub {
      margin: 0 0 24px;
      color: #94a3b8;
      font-size: 0.95rem;
    }
    .led-group {
      display: flex;
      flex-direction: column;
      gap: 20px;
      margin-bottom: 12px;
    }
    .led-item {
      display: flex;
      align-items: center;
      gap: 12px;
    }
    .led-item label {
      width: 60px;
      font-weight: 600;
      color: #cbd5e1;
    }
    .led-item input[type="range"] {
      flex: 1;
      accent-color: #38bdf8;
      cursor: pointer;
    }
    .led-item .val {
      min-width: 36px;
      text-align: center;
      font-weight: 700;
      color: #38bdf8;
    }
    .tip {
      margin-top: 18px;
      font-size: 0.85rem;
      color: #64748b;
      text-align: center;
      border-top: 1px solid #1e293b;
      padding-top: 14px;
    }
  </style>
</head>
<body>
  <div class="card">
    <h1>🔦 Triple LED Control</h1>
    <div class="sub">Drag sliders to adjust brightness (0-255)</div>

    <div class="led-group">
      <div class="led-item">
        <label>LED 1</label>
        <input type="range" id="led0" min="0" max="255" value="0">
        <span class="val" id="val0">0</span>
      </div>
      <div class="led-item">
        <label>LED 2</label>
        <input type="range" id="led1" min="0" max="255" value="0">
        <span class="val" id="val1">0</span>
      </div>
      <div class="led-item">
        <label>LED 3</label>
        <input type="range" id="led2" min="0" max="255" value="0">
        <span class="val" id="val2">0</span>
      </div>
    </div>

    <div class="tip">Changes are sent to ESP32 when you release the slider.</div>
  </div>

  <script>
    const leds = [
      { input: document.getElementById('led0'), display: document.getElementById('val0'), index: 0 },
      { input: document.getElementById('led1'), display: document.getElementById('val1'), index: 1 },
      { input: document.getElementById('led2'), display: document.getElementById('val2'), index: 2 }
    ];
    function sendValue(index, value) {
      fetch('/set?led=' + index + '&value=' + value).catch(() => {});
    }

    leds.forEach((led, i) => {
      led.input.addEventListener('input', () => {
        const v = parseInt(led.input.value);
        led.display.textContent = v;
      });

      led.input.addEventListener('change', () => {
        sendValue(led.index, parseInt(led.input.value));
      });
    });
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html; charset=utf-8", index_html);
}

void handleSet() {
  if (server.hasArg("led") && server.hasArg("value")) {
    int ledIndex = server.arg("led").toInt();
    int val = constrain(server.arg("value").toInt(), 0, 255);
    if (ledIndex >= 0 && ledIndex < 3) {
      brightness[ledIndex] = val;
      ledcWrite(ledPins[ledIndex], val);
    }
  }
  char resp[16];
  snprintf(resp, sizeof(resp), "%d,%d,%d", brightness[0], brightness[1], brightness[2]);
  server.send(200, "text/plain; charset=utf-8", resp);
}

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 3; i++) {
    ledcAttach(ledPins[i], pwmFrequency, pwmResolution);
    ledcWrite(ledPins[i], 0);
  }

  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP(apSsid, apPassword)) {
    Serial.println("AP 启动失败�?");
    while (1) delay(100);
  }
  Serial.println("AP 模式已启�?");

  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.begin();

  Serial.println("Web dimmer ready");
  Serial.print("连接到热�?: ");
  Serial.println(apSsid);
  Serial.print("浏览器访�?: http://");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  server.handleClient();
}