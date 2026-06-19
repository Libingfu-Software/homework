// 定义LED引脚（ESP32板载LED通常为GPIO 2）
const int ledPin = 2;

// ---------- 步骤结构体 ----------
struct Step {
  unsigned long duration; // 本步骤持续毫秒数
  int level;              // LED电平：HIGH 或 LOW
};

// ---------- SOS 序列 ----------
// 短闪：亮200ms，灭200ms；长闪：亮600ms，灭200ms
// 字母间隔：额外灭300ms（加上前一个灭200ms，总灭500ms）
// 单词间隔：额外灭1800ms（加上前一个灭200ms，总灭2000ms）
const Step steps[] = {
  {200, HIGH},   // 0  短亮1
  {200, LOW},    // 1  短灭1
  {200, HIGH},   // 2  短亮2
  {200, LOW},    // 3  短灭2
  {200, HIGH},   // 4  短亮3
  {200, LOW},    // 5  短灭3（第三个短闪的灭）
  {300, LOW},    // 6  字母间隔（灭300，与步骤5合计500）
  {600, HIGH},   // 7  长亮1
  {200, LOW},    // 8  长灭1
  {600, HIGH},   // 9  长亮2
  {200, LOW},    // 10 长灭2
  {600, HIGH},   // 11 长亮3
  {200, LOW},    // 12 长灭3
  {300, LOW},    // 13 字母间隔
  {200, HIGH},   // 14 短亮1（第二轮）
  {200, LOW},    // 15 短灭1
  {200, HIGH},   // 16 短亮2
  {200, LOW},    // 17 短灭2
  {200, HIGH},   // 18 短亮3
  {200, LOW},    // 19 短灭3
  {1800, LOW}    // 20 单词间隔（与步骤19合计2000）
};

const int stepCount = sizeof(steps) / sizeof(steps[0]); // 总步骤数

// ---------- 状态变量 ----------
unsigned long previousMillis = 0; // 当前步骤开始的时间点
int currentStep = 0;             // 当前步骤索引

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  // 从第一步开始
  digitalWrite(ledPin, steps[0].level);
  previousMillis = millis(); // 记录第一步的开始时刻
}

void loop() {
  unsigned long currentMillis = millis();

  // 检查当前步骤是否已超时
  if (currentMillis - previousMillis >= steps[currentStep].duration) {
    // 前进到下一步
    currentStep++;
    if (currentStep >= stepCount) {
      currentStep = 0; // 循环播放
    }

    // 设置新步骤的LED状态
    digitalWrite(ledPin, steps[currentStep].level);
    // 更新开始时刻
    previousMillis = currentMillis;
  }

  // 在这里可以添加其他非阻塞任务，它们将照常运行
}