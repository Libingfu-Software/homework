// 定义LED引脚，ESP32通常板载LED连接在GPIO 2
const int ledPin = 2;

// 用于记录上次LED翻转的时刻（单位：毫秒）
unsigned long previousMillis = 0;
// 定义翻转周期：500ms（对应1Hz）
const unsigned long interval = 500;

// LED当前状态
bool ledState = LOW;

void setup() {
  // 初始化串口通信，设置波特率为115200
  Serial.begin(115200);
  // 将LED引脚设置为输出模式
  pinMode(ledPin, OUTPUT);
  // 初始状态为熄灭（可选）
  digitalWrite(ledPin, ledState);
}

void loop() {
  // 获取当前运行毫秒数
  unsigned long currentMillis = millis();

  // 判断是否到达翻转时刻
  if (currentMillis - previousMillis >= interval) {
    // 保存本次翻转的时刻，供下次比较
    previousMillis = currentMillis;

    // 翻转LED状态
    ledState = !ledState;
    digitalWrite(ledPin, ledState);

    // （可选）在串口监视器打印状态，方便观察
    // Serial.println(ledState ? "ON" : "OFF");
  }

  // 这里可以放心添加其他非阻塞任务，它们会照常执行
}