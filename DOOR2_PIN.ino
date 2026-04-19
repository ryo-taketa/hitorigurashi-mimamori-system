#include <M5StickCPlus2.h>
#include <WiFi.h>
#include <HTTPClient.h>

// WiFi設定
const char* WIFI_SSID = "********";
const char* WIFI_PASS = "********";

// 統括M5のIP
const char* SERVER_IP = "********";  

// ドアセンサGPIO
#define DOOR_PIN 33   // 動作確認済みピン

// 状態管理
int lastState = HIGH;
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 3000; // 連続送信防止（3秒）

// ドア通知関数
void sendDoorSignal() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = "http://" + String(SERVER_IP) + "/door";

  http.begin(url);
  int code = http.GET();
  Serial.println("Door send: " + String(code));
  http.end();
}

// 初期化
void setup() {
  M5.begin();
  Serial.begin(115200);

  M5.Lcd.setRotation(1);
  M5.Lcd.setTextSize(2);

  pinMode(DOOR_PIN, INPUT);

  // WiFi接続
  M5.Lcd.print("Connecting WiFi");

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    M5.Lcd.print(".");
  }

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("Door Sensor");
  M5.Lcd.println("WiFi OK");
}

// メインループ
void loop() {
  int state = digitalRead(DOOR_PIN);

  // 状態変化検知
  if (state != lastState) {
    unsigned long now = millis();

    if (now - lastSendTime > sendInterval) {
      sendDoorSignal();
      lastSendTime = now;
    }

    // 画面表示
    M5.Lcd.fillRect(0, 60, 240, 60, BLACK);
    M5.Lcd.setCursor(0, 60);

    if (state == HIGH) {
  M5.Lcd.setTextColor(RED);
  M5.Lcd.println("DOOR CLOSED");
} else {
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.println("DOOR OPEN");
}

    lastState = state;
  }

  delay(100);
}


