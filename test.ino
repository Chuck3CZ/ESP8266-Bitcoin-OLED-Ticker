#include "ESP8266WiFi.h"
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include "WiFiClient.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "ArduinoJson.h"

const IPAddress apIP(192, 168, 1, 1);
const char* apSSID = "Ticker_SETUP";
boolean settingMode;
String ssidList;

DNSServer dnsServer;
ESP8266WebServer webServer(80);

#define OLED_RESET 0
Adafruit_SSD1306 display(OLED_RESET);

const char* host = "api.coindesk.com";

void setup() {
  Serial.begin(115200);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(2000);
  display.clearDisplay();

  EEPROM.begin(512);
  delay(10);
  if (restoreConfig()) {
    if (checkConnection()) {
      settingMode = false;
      startWebServer();
      return;
    }
  }
  settingMode = true;
  setupMode();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("  BITCOIN   TICKER");
  display.println();
  display.setTextSize(2);
  display.setCursor(2, 24);
  display.println("88888");
  display.display();
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("  BITCOIN   TICKER");
  display.setTextSize(2);
  display.println("     ");
  display.display();
  delay(400);
  display.clearDisplay();

  Serial.println();
  Serial.println();
  Serial.print("Connecting to WiFi");

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("  BITCOIN   TICKER");
  display.display();
  delay(500);

  // Loading Dots
  for (int i = 0; i < 3; i++) {
    display.setCursor(0, 24);
    display.setTextSize(2);
    display.println(" .  ");
    display.display();
    delay(500);
    display.setCursor(0, 24);
    display.println("  .  ");
    display.display();
    delay(500);
    display.setCursor(0, 24);
    display.println("   .  ");
    display.display();
    delay(500);
  }

  display.clearDisplay();
}

void loop() {
  if (settingMode) {
    dnsServer.processNextRequest();
  }
  webServer.handleClient();
  
  if (WiFi.status() == WL_CONNECTED) {
    float bitcoinPrice = getBitcoinPrice();
    if (bitcoinPrice > 0) {
      displayBitcoinPrice(bitcoinPrice);
    } else {
      displayPriceError();
    }
    delay(5000);
  }
}

float getBitcoinPrice() {
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    return 0.0;
  }
  
  String url = "/v1/bpi/currentprice.json";
  
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(100);

  String answer;
  while (client.available()) {
    String line = client.readStringUntil('\r');
    answer += line;
  }
  client.stop();
  
  String jsonAnswer;
  int jsonIndex;

  for (int i = 0; i < answer.length(); i++) {
    if (answer[i] == '{') {
      jsonIndex = i;
      break;
    }
  }

  jsonAnswer = answer.substring(jsonIndex);
  jsonAnswer.trim();

  int rateIndex = jsonAnswer.indexOf("rate_float");
  String priceString = jsonAnswer.substring(rateIndex + 12, rateIndex + 18);
  priceString.trim();
  return priceString.toFloat();
}

void displayBitcoinPrice(float price) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("  BITCOIN   TICKER");
  display.setCursor(2, 24);
  display.setTextSize(2);
  display.println(price, 2); // Display with 2 decimal places
  display.display();
}

void displayPriceError() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(" BITCOIN   TICKER");
  display.println(" Price     ERROR");
  display.display();
}
