#include <WiFi.h>
#include <WebServer.h>

// 触摸引脚
const int touchPin = T0;

// WiFi 配置（根据实际环境修改）
const char* ssid = "lisi";
const char* password = "1564922632";

WebServer server(80);

// 网页 HTML
String buildPage() {
  return R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Touch Sensor Dashboard</title>
  <style>
    body {
      margin: 0;
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      background: radial-gradient(circle at center, #1e293b, #0f172a);
      font-family: 'Segoe UI', Arial, sans-serif;
      color: #e2e8f0;
    }
    .dashboard {
      text-align: center;
    }
    h1 {
      font-size: 1.8rem;
      margin-bottom: 2rem;
      letter-spacing: 0.05em;
      color: #94a3b8;
    }
    .value-box {
      font-size: 8rem;
      font-weight: 700;
      line-height: 1.2;
      background: rgba(255, 255, 255, 0.05);
      padding: 0.5rem 2rem;
      border-radius: 2rem;
      backdrop-filter: blur(10px);
      box-shadow: 0 10px 30px rgba(0, 0, 0, 0.5);
      display: inline-block;
      min-width: 200px;
    }
    .unit {
      font-size: 1.5rem;
      color: #64748b;
      margin-top: 1rem;
    }
    p {
      color: #64748b;
      margin-top: 2rem;
    }
  </style>
</head>
<body>
  <div class="dashboard">
    <h1>ESP32 Touch Sensor</h1>
    <div class="value-box" id="touchValue">--</div>
    <div class="unit">raw analog value</div>
    <p>Approach or touch pin T0 to see it drop</p>
  </div>

  <script>
    const valueDisplay = document.getElementById('touchValue');

    function fetchValue() {
      fetch('/touch')
        .then(response => response.text())
        .then(text => {
          valueDisplay.textContent = text;
        })
        .catch(() => {
          valueDisplay.textContent = 'ERR';
        });
    }

    // 立即获取一次，之后每 200ms 更新
    fetchValue();
    setInterval(fetchValue, 200);
  </script>
</body>
</html>
)rawliteral";
}

// 返回触摸模拟量（纯文本）
void handleTouch() {
  int touchValue = touchRead(touchPin);
  server.send(200, "text/plain", String(touchValue));
}

// 主页
void handleRoot() {
  server.send(200, "text/html", buildPage());
}

void setup() {
  Serial.begin(115200);

  // 连接 WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  // 注册 Web 路由
  server.on("/", handleRoot);
  server.on("/touch", handleTouch);
  server.begin();

  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}