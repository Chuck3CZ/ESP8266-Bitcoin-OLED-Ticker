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

const IPAddress apIP(192, 168, 1, 1);
const char* apSSID = "Ticker_SETUP";
const char* host = "api.coindesk.com";
const int httpPort = 80;

boolean settingMode;
String ssidList;
DNSServer dnsServer;
ESP8266WebServer webServer(80);

void showText(const char* title, const char* content, int textSize = 1) {
    display.clearDisplay();
    display.setTextSize(textSize);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println(title);
    display.println(content);
    display.display();
}

void showLoadingAnimation() {
    showText("  BITCOIN   TICKER", " .  ", 1);
    delay(500);
    showText("  BITCOIN   TICKER", "  .  ", 1);
    delay(500);
    showText("  BITCOIN   TICKER", "   .  ", 1);
    delay(500);
    display.clearDisplay();
}

void setup() {
    Serial.begin(115200);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.display();
    delay(2000);
    display.clearDisplay();
    
    EEPROM.begin(512);
    delay(10);
    if (restoreConfig() && checkConnection()) {
        settingMode = false;
        startWebServer();
        return;
    }
    settingMode = true;
    setupMode();
}

void loop() {
    if (settingMode) {
        dnsServer.processNextRequest();
    }
    webServer.handleClient();
    fetchBitcoinPrice();
}

void fetchBitcoinPrice() {
    WiFiClient client;
    if (!client.connect(host, httpPort)) {
        Serial.println("Connection to API failed.");
        return;
    }
    
    client.print(String("GET /v1/bpi/currentprice.json HTTP/1.1\r\n") + "Host: " + host + "\r\nConnection: close\r\n\r\n");
    delay(100);
    
    String answer;
    while (client.available()) {
        answer += client.readStringUntil('\r');
    }
    client.stop();
    
    int jsonIndex = answer.indexOf('{');
    if (jsonIndex == -1) {
        Serial.println("Invalid JSON response.");
        return;
    }
    
    String jsonAnswer = answer.substring(jsonIndex);
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, jsonAnswer);
    if (error) {
        Serial.println("JSON parsing failed.");
        return;
    }
    
    float price = doc["bpi"]["USD"]["rate_float"];
    Serial.println("Bitcoin price: " + String(price));
    
    if (price > 0) {
        showText("  BITCOIN   TICKER", String(price).c_str(), 2);
    } else {
        showText("  BITCOIN   TICKER", "Price ERROR", 1);
    }
    delay(5000);
}

boolean restoreConfig() {
    Serial.println("Reading EEPROM...");
    String ssid, pass;
    if (EEPROM.read(0) == 0) return false;
    
    for (int i = 0; i < 32; ++i) ssid += char(EEPROM.read(i));
    for (int i = 32; i < 96; ++i) pass += char(EEPROM.read(i));
    
    WiFi.begin(ssid.c_str(), pass.c_str());
    return true;
}

boolean checkConnection() {
    Serial.print("Waiting for Wi-Fi connection");
    for (int count = 0; count < 30; count++) {
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nWiFi connected");
            return true;
        }
        showLoadingAnimation();
    }
    Serial.println("Timed out.");
    return false;
}

void startWebServer() {
    Serial.println(settingMode ? "Starting AP mode Web Server" : "Starting STA mode Web Server");
    webServer.on("/settings", []() {
        String s = "<h1>Wi-Fi Settings</h1><form method='get' action='setap'><label>SSID: </label><select name='ssid'>" + ssidList + "</select><br>Password: <input name='pass' type='password'><input type='submit'></form>";
        webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
    });
    webServer.on("/setap", []() {
        String ssid = webServer.arg("ssid");
        String pass = webServer.arg("pass");
        for (int i = 0; i < 96; ++i) EEPROM.write(i, 0);
        for (int i = 0; i < ssid.length(); ++i) EEPROM.write(i, ssid[i]);
        for (int i = 0; i < pass.length(); ++i) EEPROM.write(32 + i, pass[i]);
        EEPROM.commit();
        webServer.send(200, "text/html", makePage("Wi-Fi Settings", "<h1>Setup complete.</h1><p>Device will restart.</p>"));
        ESP.restart();
    });
    webServer.begin();
}

void setupMode() {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(apSSID);
    dnsServer.start(53, "*", apIP);
    startWebServer();
}

String makePage(String title, String contents) {
    return "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width,user-scalable=0'><title>" + title + "</title></head><body>" + contents + "</body></html>";
}
