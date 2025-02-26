/* Optimized ESP8266 Oled Bitcoin price ticker */

#include "ESP8266WiFi.h"
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include "WiFiClient.h"
#include "SPI.h"
#include "Wire.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "ArduinoJson.h"

#define OLED_RESET 0
Adafruit_SSD1306 display(OLED_RESET);

const char* host = "api.coindesk.com";

void setup() {
    Serial.begin(115200);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.display();
    delay(2000);
    display.clearDisplay();
}

void loop() {
    WiFiClient client;
    if (!client.connect(host, 80)) return;
    
    client.print("GET /v1/bpi/currentprice.json HTTP/1.1\r\n" "Host: api.coindesk.com\r\n" "Connection: close\r\n\r\n");
    delay(100);
    
    String answer, jsonAnswer;
    while (client.available()) {
        answer += client.readStringUntil('\r');
    }
    client.stop();
    
    int jsonIndex = answer.indexOf('{');
    if (jsonIndex != -1) {
        jsonAnswer = answer.substring(jsonIndex);
    }
    
    int rateIndex = jsonAnswer.indexOf("rate_float");
    String priceString = jsonAnswer.substring(rateIndex + 12, rateIndex + 18);
    priceString.trim();
    float price = priceString.toFloat();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println(" BITCOIN TICKER");
    
    display.setCursor(2, 24);
    display.setTextSize(2);
    if (price >= 100000) {
        int shortPrice = static_cast<int>(price) / 1000; // Získání prvních třech číslic
        display.print(shortPrice);
        display.println("K");
    } else {
        display.println(priceString);
    }
    
    display.display();
    delay(5000);
}
