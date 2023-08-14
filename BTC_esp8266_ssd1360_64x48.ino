

/*********************************************************************
This is a Minimalist ESP8266 Oled Bitcoin price ticker 

from site:
https://github.com/Chuck3CZ/esp8266-oled-bitcoin-ticker

This project is for ESP8266 with 64x48 size OLED display using I2C to communication 
3 pins are required to interface (2 I2C and one reset)

Written by Martin "Chuck3CZ" Gabrhel
*********************************************************************/


#include "ESP8266WiFi.h"
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include "WiFiClient.h"
#include "WiFiServer.h"
#include "SPI.h"
#include "Wire.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "ArduinoJson.h"

const IPAddress apIP(192, 168, 1, 1);
const char* apSSID = "Ticker_SETUP";
boolean settingMode;
String ssidList;

DNSServer dnsServer;
ESP8266WebServer webServer(80);

//WiFiServer server(80);
String header;

 // SCL GPIO5 
 // SDA GPIO4

#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);

 const char* host = "api.coindesk.com";

void setup()   {  
  Serial.begin(115200);

  
  // by default, generating the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  // init done
  
  // Show image buffer on the display hardware. (Bitcoin logo) -- in adafruit Library
  // internally, this will display the splashscreen
  display.display();
  delay(2000); 

  // Clear the buffer.
  display.clearDisplay();
  
  // init text display
  
  // display.setTextColor(BLACK, WHITE); // 'inverted' text
  
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
  display.setCursor(0,0);
  display.println("  BITCOIN   TICKER");
  display.println();
  display.setTextSize(2);
  display.setCursor(2,24);
  display.println("88888");
  display.display();
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0,0);
 display.println("  BITCOIN   TICKER");
  display.setTextSize(2);
  display.println("     ");
  display.display();
  delay(400);
  display.clearDisplay();


  Serial.println();
  Serial.println();
  Serial.print("Connecting to WiFi");
  

//WiFi.begin(ssid, password);
  
  
//  while (WiFi.status() != WL_CONNECTED) 
//  {
    Serial.print(".");
  delay(50);
    Serial.print(".");
  delay(50);
    Serial.print(".");
  
// Main text
   display.setTextSize(1);
   display.setCursor(0,0);
   display.println("  BITCOIN   TICKER");
   display.display();
 delay(500);
    
// Loading Dot 1    
   display.setCursor(0,24); 
   display.setTextSize(2);
   display.println(" .  ");
   display.display();
 delay(500);
    
// Loading Dot 2       
   display.setCursor(0,24);
   display.println("  .  ");
   display.display();
 delay(500);
        
// Loading Dot 3   
   display.setCursor(0,24);
   display.println("   .  ");
   display.display();
 delay(500);
 
  display.clearDisplay();
 // }

  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("  BITCOIN   TICKER");
  display.setCursor(0,24);
  display.println("Created AP          Setup WiFi");
  display.display();
  
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}
void loop() {

 if (settingMode) {
    dnsServer.processNextRequest();
  }
  webServer.handleClient();


   
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    return;
  }
  
  String url = "/v1/bpi/currentprice.json";
  
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(100);
  

  String answer;
  while(client.available()){
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

  // JSON data reading
  jsonAnswer = answer.substring(jsonIndex);
  jsonAnswer.trim();

  // Obtaining prices like float value
  int rateIndex = jsonAnswer.indexOf("rate_float");
  String priceString = jsonAnswer.substring(rateIndex + 12, rateIndex + 18);
    priceString.trim();
  float price = priceString.toFloat();

    Serial.println();
    Serial.println("Bitcoin price: ");
    Serial.println(price);

  if(price > 0){
    display.clearDisplay();  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("  BITCOIN   TICKER");
  display.println();
  display.setCursor(2,24);
  display.setTextSize(2);
  display.println(priceString);
  display.display();
  delay(5000);
  }
  
  if(price == 0){
    display.clearDisplay(); 
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(" BITCOIN   TICKER");
  display.println(" Price     ERROR");
  delay(5000);
  }
}
 

boolean restoreConfig() {
  Serial.println("Reading EEPROM...");
  String ssid = "";
  String pass = "";
  if (EEPROM.read(0) != 0) {
    for (int i = 0; i < 32; ++i) {
      ssid += char(EEPROM.read(i));
    }
    Serial.print("SSID: ");
    Serial.println(ssid);
    for (int i = 32; i < 96; ++i) {
      pass += char(EEPROM.read(i));
    }
    Serial.print("Password: ");
    Serial.println(pass);
    WiFi.begin(ssid.c_str(), pass.c_str());
    return true;
  }
  else {
    Serial.println("Config not found.");
    return false;
  }
}

boolean checkConnection() {
  int count = 0;
  Serial.print("Waiting for Wi-Fi connection");
  while ( count < 30 ) {
    if (WiFi.status() == WL_CONNECTED) {
          Serial.print(".");
  delay(50);
    Serial.print(".");
  delay(50);
    Serial.print(".");
  
// Main text
   display.setTextSize(1);
   display.setCursor(0,0);
   display.println("  BITCOIN   TICKER");
   display.display();
 delay(500);
    
// Loading Dot 1    
   display.setCursor(0,24); 
   display.setTextSize(2);
   display.println(" .  ");
   display.display();
 delay(500);
    
// Loading Dot 2       
   display.setCursor(0,24);
   display.println("  .  ");
   display.display();
 delay(500);
        
// Loading Dot 3   
   display.setCursor(0,24);
   display.println("   .  ");
   display.display();
 delay(500);
 
  display.clearDisplay();
    Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
      return (true);
    }
    delay(500);
    Serial.print(".");
    count++;
  }
  Serial.println("Timed out.");
  return false;
}

void startWebServer() {
  if (settingMode) {
    Serial.print("Starting Web Server at ");
    Serial.println(WiFi.softAPIP());
    webServer.on("/settings", []() {
      String s = "<h1>Wi-Fi Settings</h1><p>Please enter your Wi-fi SSID and Password.</p>";
      s += "<form method=\"get\" action=\"setap\"><label>SSID: </label><select name=\"ssid\">";
      s += ssidList;
      s += "</select><br>Password: <input name=\"pass\" length=64 type=\"password\"><input type=\"submit\"></form>";
      webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
    });
    webServer.on("/setap", []() {
      for (int i = 0; i < 96; ++i) {
        EEPROM.write(i, 0);
      }
      String ssid = urlDecode(webServer.arg("ssid"));
      Serial.print("SSID: ");
      Serial.println(ssid);
      String pass = urlDecode(webServer.arg("pass"));
      Serial.print("Password: ");
      Serial.println(pass);
      Serial.println("Writing SSID to EEPROM...");
      for (int i = 0; i < ssid.length(); ++i) {
        EEPROM.write(i, ssid[i]);
      }
      Serial.println("Writing Password to EEPROM...");
      for (int i = 0; i < pass.length(); ++i) {
        EEPROM.write(32 + i, pass[i]);
      }
      EEPROM.commit();
      Serial.println("Write EEPROM done!");
      String s = "<h1>Setup complete.</h1><p>device will be connected to \"";
      s += ssid;
      s += "\" after the restart.";
      webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
      ESP.restart();
    });
    webServer.onNotFound([]() {
      String s = "<h1>Access point (AP) mode</h1><h2>BTC Ticker by <a href=\"https://github.com/Chuck3CZ\">Chuck3CZ</a></h2><p><a href=\"/settings\">Wi-Fi Settings</a></p>";
      webServer.send(200, "text/html", makePage("AP mode", s));
    });
  }
  else {
    Serial.print("Starting Web Server at ");
    Serial.println(WiFi.localIP());
    webServer.on("/", []() {
      String s = "<h1>STA mode</h1><p><a href=\"/reset\">Reset Wi-Fi Settings</a></p>";
      webServer.send(200, "text/html", makePage("STA mode", s));
    });
    webServer.on("/reset", []() {
      for (int i = 0; i < 96; ++i) {
        EEPROM.write(i, 0);
      }
      EEPROM.commit();
      String s = "<h1>Wi-Fi settings was reset.</h1><p>Please reset device.</p>";
      webServer.send(200, "text/html", makePage("Reset Wi-Fi Settings", s));
    });
  }
  webServer.begin();
}

void setupMode() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  delay(100);
  Serial.println("");
  for (int i = 0; i < n; ++i) {
    ssidList += "<option value=\"";
    ssidList += WiFi.SSID(i);
    ssidList += "\">";
    ssidList += WiFi.SSID(i);
    ssidList += "</option>";
  }
  delay(100);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID);
  dnsServer.start(53, "*", apIP);
  startWebServer();
  Serial.print("Starting Access Point at \"");
  Serial.print(apSSID);
  Serial.println("\"");
}

String makePage(String title, String contents) {
  String s = "<!DOCTYPE html><html><head>";
  s += "<meta name=\"viewport\" content=\"width=device-width,user-scalable=0\">";
  s += "<title>";
  s += title;
  s += "</title></head><body>";
  s += contents;
  s += "</body></html>";
  return s;
}

String urlDecode(String input) {
  String s = input;
  s.replace("%20", " ");
  s.replace("+", " ");
  s.replace("%21", "!");
  s.replace("%22", "\"");
  s.replace("%23", "#");
  s.replace("%24", "$");
  s.replace("%25", "%");
  s.replace("%26", "&");
  s.replace("%27", "\'");
  s.replace("%28", "(");
  s.replace("%29", ")");
  s.replace("%30", "*");
  s.replace("%31", "+");
  s.replace("%2C", ",");
  s.replace("%2E", ".");
  s.replace("%2F", "/");
  s.replace("%2C", ",");
  s.replace("%3A", ":");
  s.replace("%3A", ";");
  s.replace("%3C", "<");
  s.replace("%3D", "=");
  s.replace("%3E", ">");
  s.replace("%3F", "?");
  s.replace("%40", "@");
  s.replace("%5B", "[");
  s.replace("%5C", "\\");
  s.replace("%5D", "]");
  s.replace("%5E", "^");
  s.replace("%5F", "-");
  s.replace("%60", "`");
  return s;
}
