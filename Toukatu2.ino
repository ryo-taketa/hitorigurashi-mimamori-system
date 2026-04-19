#include <M5StickCPlus2.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

// WiFi設定
const char* WIFI_SSID = "********";
const char* WIFI_PASS = "********";

// LINE Messaging API設定
const char* CHANNEL_ACCESS_TOKEN = "********";
const char* USER_ID = "********";

// サーバ設定
WebServer server(80);

// 状態管理
unsigned long lastMotionTime = 0;

bool isHome = true;              // 在宅状態
bool doorTriggered = false;     // ドアイベント発生
unsigned long doorEventTime = 0;

bool notice30Sent = false;
bool notice60Sent = false;

// 判定時間
const unsigned long EXIT_CHECK_TIME = 2 * 60 * 1000;   
const unsigned long NOTICE_30 = 30 * 60 * 1000;        
const unsigned long NOTICE_60 =  60 * 60 * 1000;        

// LINE送信関数
void sendLineMessage(String text) {
  HTTPClient http;
  http.begin("https://api.line.me/v2/bot/message/push");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + String(CHANNEL_ACCESS_TOKEN));

  String body = "{";
  body += "\"to\":\"" + String(USER_ID) + "\",";
  body += "\"messages\":[{\"type\":\"text\",\"text\":\"" + text + "\"}]";
  body += "}";

  int code = http.POST(body);
  Serial.println("LINE status: " + String(code));
  http.end();
}

// PIR受信
void handleMotion() {
  lastMotionTime = millis();

  // 帰宅判定
  if (!isHome && doorTriggered) {
    isHome = true;
    doorTriggered = false;
  }

  notice30Sent = false;
  notice60Sent = false;

  server.send(200, "text/plain", "motion ok");
}

// ドア受信
void handleDoor() {
  doorTriggered = true;
  doorEventTime = millis();

  server.send(200, "text/plain", "door ok");
}

// 初期化
void setup() {
  M5.begin();
  Serial.begin(115200);

  M5.Lcd.setRotation(1);
  M5.Lcd.setTextSize(2);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  M5.Lcd.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    M5.Lcd.print(".");
  }

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("Server Ready");
  M5.Lcd.println(WiFi.localIP());

  // サーバルート
  server.on("/motion", handleMotion);
  server.on("/door", handleDoor);

  server.begin();

  lastMotionTime = millis();
}

// メインループ
void loop() {
  server.handleClient();

  unsigned long now = millis();

  // 外出判定
  if (doorTriggered && isHome) {
    if (now - doorEventTime > EXIT_CHECK_TIME) {
      isHome = false;
      doorTriggered = false;
    }
  }

  // 無動作判定（在宅時のみ）
  if (isHome) {
    unsigned long noMotion = now - lastMotionTime;

    if (noMotion > NOTICE_30 && !notice30Sent) {
      sendLineMessage("30分間動作がありません");
      notice30Sent = true;
    }

    if (noMotion > NOTICE_60 && !notice60Sent) {
      sendLineMessage("【異常】60分間動作がありません");
      notice60Sent = true;
    }
  }

  // 画面表示
  M5.Lcd.setCursor(0, 40);
  M5.Lcd.fillRect(0, 40, 240, 80, BLACK);

  if (isHome) {
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.println("STATUS: HOME");
  } else {
    M5.Lcd.setTextColor(RED);
    M5.Lcd.println("STATUS: AWAY");
  }

  delay(500);
}
