

/*********************************************************************
This is a Minimalist ESP8266 Oled Bitcoin price ticker 

from site:
https://github.com/Chuck3CZ/esp8266-oled-bitcoin-ticker

This project is for ESP8266 with 64x48 size OLED display using I2C to communication 
3 pins are required to interface (2 I2C and one reset)

Written by Martin "Chuck3CZ" Gabrhel
*********************************************************************/


#include <strings_en.h>
#include <WiFiManager.h>
#include "ESP8266WiFi.h"
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include "WiFiClient.h"
#include "WiFiServer.h"
#include "SPI.h"
#include "Wire.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "ArduinoJson.h"

WiFiServer server(80);
String header;

 // SCL GPIO5 
 // SDA GPIO4

#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);

 const char* ssid     = "YOUR_WIFI_SSID";
 const char* password = "YOUR_WIFI_PASSWORD";
 const char* host = "api.coindesk.com";

void setup()   {
  Serial.begin(9600);

  WiFi.begin();

  WiFiManager wifiManager;
  wifiManager.autoConnect();

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
  Serial.print("Connecting to ");
  Serial.println(ssid);


  //WiFi.begin(ssid, password);
  
  
  while (WiFi.status() != WL_CONNECTED) 
  {
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
  }

  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("  BITCOIN   TICKER");
  display.setCursor(0,24);
  display.println(" Connected");
  display.display();
  
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}



void loop() {
   
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
  
  if(price = 0){
    display.clearDisplay(); 
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(" BITCOIN   TICKER");
  display.println(" Price     ERROR");
  delay(5000);
  }
}
