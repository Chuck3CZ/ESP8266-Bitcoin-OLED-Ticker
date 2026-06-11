/*
  ESP8266 Bitcoin OLED Ticker
  Wemos D1 Mini + 0.66" 64x48 OLED (SSD1306, I2C)

  SCL → GPIO5 (D1)
  SDA → GPIO4 (D2)
  RST → GPIO0 (D3)

  Original: https://github.com/Chuck3CZ/ESP8266-Bitcoin-OLED-Ticker

  Fixed & improved:
  - Replaced dead CoinDesk API with CoinGecko (HTTPS)
  - Added WiFiClientSecure for TLS
  - Proper ArduinoJson parsing instead of fragile string search
  - Fixed setup() control flow (AP mode no longer falls through)
  - Fixed display.display() missing in error branch
  - Added 60-second refresh interval (was: hammer API in tight loop)
  - Fixed EEPROM read stopping at null bytes
  - Fixed urlDecode wrong mappings (%30, %31, %3B, %5F, duplicate %3A/%2C)
  - Fixed price + header text overflow on 64px-wide display
  - Adaptive price formatting ("67.2k") so textSize(2) always fits
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClientSecure.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

// AP config
const IPAddress apIP(192, 168, 1, 1);
const char* apSSID = "Ticker_SETUP";
bool settingMode = false;
String ssidList;

DNSServer dnsServer;
ESP8266WebServer webServer(80);

#define OLED_RESET 0
Adafruit_SSD1306 display(OLED_RESET);

// CoinGecko free API — no key required, ~30 calls/min allowed
const char* apiHost  = "api.coingecko.com";
const char* apiPath  = "/api/v3/simple/price?ids=bitcoin&vs_currencies=usd";
const unsigned long FETCH_INTERVAL_MS = 60000UL;
unsigned long lastFetchMs = 0;

// ── Forward declarations ────────────────────────────────────────────────────
boolean restoreConfig();
boolean checkConnection();
void    startWebServer();
void    setupMode();
void    showLoadingAnimation();
void    fetchAndShowPrice();
void    showOledError(const char* line1, const char* line2);
String  formatPrice(float price);
String  makePage(String title, String body);
String  urlDecode(String input);

// ── setup() ────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();   // built-in Adafruit splash
  delay(2000);
  display.clearDisplay();

  EEPROM.begin(512);
  delay(10);

  if (restoreConfig() && checkConnection()) {
    settingMode = false;
    startWebServer();
    showLoadingAnimation();
    return;   // ← normal (STA) path ends here
  }

  // No config or connection failed → start AP setup mode
  settingMode = true;
  setupMode();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(4, 0);
  display.println("WiFi Setup");
  display.setCursor(0, 12);
  display.println("Connect to:");
  display.setCursor(0, 24);
  display.println(apSSID);
  display.display();
}

// ── loop() ─────────────────────────────────────────────────────────────────
void loop() {
  if (settingMode) {
    dnsServer.processNextRequest();
    webServer.handleClient();
    return;
  }

  webServer.handleClient();

  unsigned long now = millis();
  if (lastFetchMs == 0 || (now - lastFetchMs) >= FETCH_INTERVAL_MS) {
    lastFetchMs = now;
    fetchAndShowPrice();
  }
}

// ── Price fetch & display ───────────────────────────────────────────────────
void fetchAndShowPrice() {
  WiFiClientSecure client;
  client.setInsecure();   // skip cert validation — fine for a hobby display

  Serial.println("Connecting to CoinGecko...");
  if (!client.connect(apiHost, 443)) {
    Serial.println("Connection failed");
    showOledError("Connect", "failed");
    return;
  }

  client.printf("GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
                apiPath, apiHost);

  // Skip HTTP response headers
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break;
  }

  String body = client.readString();
  client.stop();

  // Parse: {"bitcoin":{"usd":67234.12}}
  StaticJsonDocument<96> doc;
  DeserializationError err = deserializeJson(doc, body);
  if (err) {
    Serial.printf("JSON error: %s\n", err.c_str());
    showOledError("JSON", "error");
    return;
  }

  float price = doc["bitcoin"]["usd"];
  if (price <= 0) {
    showOledError("Price", "error");
    return;
  }

  String priceStr = formatPrice(price);
  Serial.printf("BTC: $%.2f → %s\n", price, priceStr.c_str());

  display.clearDisplay();
  display.setTextColor(WHITE);

  // Header: "BTC/USD" centred on 64px
  display.setTextSize(1);
  int hdrX = (64 - 7 * 6) / 2;   // 7 chars × 6px = 42px → x=11
  display.setCursor(hdrX, 0);
  display.println("BTC/USD");

  // Price in large text — formatPrice() guarantees it fits at textSize(2)
  display.setTextSize(2);
  int priceW = priceStr.length() * 12;
  int priceX = max(0, (64 - priceW) / 2);
  display.setCursor(priceX, 22);
  display.println(priceStr);

  display.display();
}

// Format price to always fit 5 chars max at textSize(2) on 64px wide display.
// textSize(2): each char = 12px → 5 chars = 60px ≤ 64px ✓
String formatPrice(float price) {
  if (price >= 100000.0f) {
    // e.g. 105000 → "105k"  (4 chars)
    return String((int)(price / 1000.0f)) + "k";
  } else if (price >= 10000.0f) {
    // e.g. 67234 → "67.2k"  (5 chars)
    return String(price / 1000.0f, 1) + "k";
  } else {
    // e.g. 9500 → "9500"  (4 chars)
    return String((int)price);
  }
}

void showOledError(const char* line1, const char* line2) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(4, 0);
  display.println("BTC/USD");
  display.setCursor(0, 20);
  display.println(line1);
  display.setCursor(0, 32);
  display.println(line2);
  display.display();   // ← was missing in the original
}

void showLoadingAnimation() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(4, 0);
  display.println("BTC/USD");
  display.display();

  for (int i = 1; i <= 3; i++) {
    String dots(i, '.');
    display.fillRect(0, 22, 64, 20, BLACK);
    display.setTextSize(2);
    display.setCursor(4, 24);
    display.println(dots);
    display.display();
    delay(500);
  }
  display.clearDisplay();
}

// ── WiFi / EEPROM helpers ───────────────────────────────────────────────────
boolean restoreConfig() {
  Serial.println("Reading EEPROM...");
  if (EEPROM.read(0) == 0) {
    Serial.println("No config stored.");
    return false;
  }
  String ssid, pass;
  for (int i = 0; i < 32; i++) {
    char c = (char)EEPROM.read(i);
    if (c == 0) break;
    ssid += c;
  }
  for (int i = 32; i < 96; i++) {
    char c = (char)EEPROM.read(i);
    if (c == 0) break;
    pass += c;
  }
  Serial.printf("SSID: %s\n", ssid.c_str());
  WiFi.begin(ssid.c_str(), pass.c_str());
  return true;
}

boolean checkConnection() {
  Serial.print("Waiting for Wi-Fi");
  for (int count = 0; count < 30; count++) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("\nConnected: %s\n", WiFi.localIP().toString().c_str());
      return true;
    }
    delay(500);
    Serial.print('.');
  }
  Serial.println("\nTimed out.");
  return false;
}

// ── Web server ──────────────────────────────────────────────────────────────
void startWebServer() {
  if (settingMode) {
    Serial.printf("AP Web Server at %s\n", WiFi.softAPIP().toString().c_str());

    webServer.on("/settings", []() {
      String s = "<h1>Wi-Fi Settings</h1>"
                 "<p>Select your network and enter the password.</p>"
                 "<form method=\"get\" action=\"/setap\">"
                 "SSID: <select name=\"ssid\">" + ssidList + "</select><br>"
                 "Password: <input name=\"pass\" length=64 type=\"password\"><br>"
                 "<input type=\"submit\" value=\"Save\"></form>";
      webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
    });

    webServer.on("/setap", []() {
      String ssid = urlDecode(webServer.arg("ssid"));
      String pass = urlDecode(webServer.arg("pass"));
      for (int i = 0; i < 96; i++) EEPROM.write(i, 0);
      for (int i = 0; i < (int)ssid.length() && i < 32; i++) EEPROM.write(i,      ssid[i]);
      for (int i = 0; i < (int)pass.length() && i < 64; i++) EEPROM.write(32 + i, pass[i]);
      EEPROM.commit();
      Serial.printf("Saved SSID: %s\n", ssid.c_str());
      String s = "<h1>Setup complete.</h1><p>Device will connect to &ldquo;" +
                 ssid + "&rdquo; after restart.</p>";
      webServer.send(200, "text/html", makePage("Done", s));
      delay(1000);
      ESP.restart();
    });

    webServer.onNotFound([]() {
      String s = "<h1>BTC Ticker Setup</h1>"
                 "<p><a href=\"/settings\">Wi-Fi Settings</a></p>";
      webServer.send(200, "text/html", makePage("BTC Ticker", s));
    });

  } else {
    Serial.printf("Web Server at %s\n", WiFi.localIP().toString().c_str());

    webServer.on("/", []() {
      webServer.send(200, "text/html",
        makePage("BTC Ticker", "<h1>BTC Ticker</h1><p><a href=\"/reset\">Reset Wi-Fi</a></p>"));
    });

    webServer.on("/reset", []() {
      for (int i = 0; i < 96; i++) EEPROM.write(i, 0);
      EEPROM.commit();
      webServer.send(200, "text/html",
        makePage("Reset", "<h1>Wi-Fi settings cleared.</h1><p>Please restart the device.</p>"));
    });
  }

  webServer.begin();
}

void setupMode() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    ssidList += "<option value=\"" + WiFi.SSID(i) + "\">" + WiFi.SSID(i) + "</option>";
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID);
  dnsServer.start(53, "*", apIP);
  startWebServer();
  Serial.printf("AP started: \"%s\"\n", apSSID);
}

// ── Utilities ───────────────────────────────────────────────────────────────
String makePage(String title, String body) {
  return "<!DOCTYPE html><html><head>"
         "<meta name=\"viewport\" content=\"width=device-width,user-scalable=0\">"
         "<title>" + title + "</title></head><body>" + body + "</body></html>";
}

String urlDecode(String s) {
  s.replace("+",    " ");
  s.replace("%20",  " ");
  s.replace("%21",  "!");
  s.replace("%22",  "\"");
  s.replace("%23",  "#");
  s.replace("%24",  "$");
  s.replace("%25",  "%");
  s.replace("%26",  "&");
  s.replace("%27",  "'");
  s.replace("%28",  "(");
  s.replace("%29",  ")");
  s.replace("%2A",  "*");
  s.replace("%2B",  "+");
  s.replace("%2C",  ",");
  s.replace("%2D",  "-");
  s.replace("%2E",  ".");
  s.replace("%2F",  "/");
  s.replace("%3A",  ":");
  s.replace("%3B",  ";");   // was: duplicate %3A
  s.replace("%3C",  "<");
  s.replace("%3D",  "=");
  s.replace("%3E",  ">");
  s.replace("%3F",  "?");
  s.replace("%40",  "@");
  s.replace("%5B",  "[");
  s.replace("%5C",  "\\");
  s.replace("%5D",  "]");
  s.replace("%5E",  "^");
  s.replace("%5F",  "_");   // was: "-"
  s.replace("%60",  "`");
  return s;
}
