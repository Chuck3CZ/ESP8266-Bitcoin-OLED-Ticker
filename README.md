# ⏲ ESP8266 Bitcoin OLED Ticker
Minimalist ESP8266 Oled Bitcoin price ticker

ESP8266 Wemos D1 Mini +  0.66 inch 64X48 OLED 
<br>
📎 STL file for 3D print: https://www.thingiverse.com/thing:3450373

🛒 🌎 <a href="https://www.aliexpress.com/item/32801063529.html?spm=a2g0o.productlist.0.0.6b454a3eJbsmM2&algo_pvid=46c4b22a-6cae-4d43-bcb1-94dd0afa92f1&algo_exp_id=46c4b22a-6cae-4d43-bcb1-94dd0afa92f1-0&pdp_ext_f=%7B%22sku_id%22%3A%2264085874119%22%7D">WEMOS D1 Mini (ESP8266) </a><br>
🛒 🌎 <a href="https://www.aliexpress.com/item/32631693796.html?spm=a2g0o.productlist.0.0.399a7da35S0mqC&algo_pvid=8f103ca4-3b0d-486b-a064-ab03522a1fee&algo_exp_id=8f103ca4-3b0d-486b-a064-ab03522a1fee-2&pdp_ext_f=%7B%22sku_id%22%3A%2259445695492%22%7D">WEMOS MODULE OLED 0.66" 64X48 </a>

🛒 🇨🇿 <a href="https://www.laskarduino.cz/wemos-d1-mini-esp8266-wifi-modul/">WEMOS D1 Mini (ESP8266) </a><br>
🛒 🇨🇿 <a href="https://www.laskarduino.cz/wemos-d1-mini-64x48-oled-displej-shield--i2c/">WEMOS MODULE OLED 0.66" 64X48 </a>

![Hnet com-image (2)](https://user-images.githubusercontent.com/31049131/141833478-1086a5fb-7206-426d-87f4-52940d3da9ad.jpg)
![Hnet com-image](https://user-images.githubusercontent.com/31049131/141833324-8143dc63-2829-4f13-8739-5dce29162fc5.jpg)

<br>

## 📚 Libraries

Install the **ESP8266 board package** in Arduino IDE (Boards Manager). It already contains `ESP8266WiFi`, `ESP8266WebServer`, `DNSServer`, `EEPROM`, `Wire` and `SPI`.

Then add these libraries:

| Library | Where from |
|---|---|
| `Adafruit_SSD1306` | this repo – a patched version 1.1.0 with 64×48 support, copy the folder to `Arduino/libraries` |
| `Adafruit_GFX` | this repo – version 1.0.2 that matches the patched SSD1306 driver, copy it the same way |
| `ArduinoJson` 6.x | Library Manager (7.x also compiles, with deprecation warnings) |

The price comes from the free [CoinGecko API](https://www.coingecko.com/en/api) (no key needed) and refreshes every 60 seconds.

<br>

## 🔧 SETUP with Wi-Fi AP

On the first start (or when the saved Wi-Fi is not reachable) the ticker opens its own access point **`Ticker_SETUP`**. Connect to it, open **http://192.168.1.1/settings** (a captive portal usually opens on its own), select your network and enter its password. The ticker saves it to EEPROM and restarts.

Wi-Fi SSID             |  Wi-FI AP Server
:-------------------------:|:-------------------------:
![IMG_6171](https://user-images.githubusercontent.com/31049131/207057544-a75e4c42-614c-46e2-9d7f-6b5912996904.PNG)  |  ![IMG_6172](https://user-images.githubusercontent.com/31049131/207057540-f3878239-6cdc-499a-b816-5a96851d8e16.PNG)





<br>

Settings for flashing with Arduino IDE  
<a href="https://user-images.githubusercontent.com/31049131/141855094-01dc5a28-63c7-4538-a6fb-2701678cd886.png">Image</a>

<br>

## ✅ To-Do list
   ✅ Easy WiFi configurator
