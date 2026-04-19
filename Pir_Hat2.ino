#include <M5StickCPlus2.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Wi-Fi設定
const char* ssid = "********";
const char* password = "********";

// 統括M5のIP
String serverIP = "********";  

// PIRセンサピン
#define PIR_PIN 36

bool lastState = false;

void sendMotion() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        String url = "http://" + serverIP + "/motion";
        http.begin(url);

        int httpCode = http.GET();
        Serial.println("Sent motion: " + String(httpCode));

        http.end();
    }
}

void setup() {
    M5.begin();
    Serial.begin(115200);

    pinMode(PIR_PIN, INPUT);

    M5.Lcd.setRotation(1);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("PIR Sensor");

    // Wi-Fi接続
    WiFi.begin(ssid, password);
    M5.Lcd.println("Connecting...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        M5.Lcd.print(".");
    }

    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("Connected");
}

void loop() {
    bool motion = digitalRead(PIR_PIN);

    // 動作検知時のみ送信
    if (motion && !lastState) {
        Serial.println("Motion detected");

        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.println("Motion!");
        
        sendMotion();
    }

    lastState = motion;
    delay(200);
}
