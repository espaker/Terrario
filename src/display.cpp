#include "display.h"
#include "config.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>

#define OLED_WIDTH  128
#define OLED_HEIGHT  64
#define OLED_RESET   -1

static Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

static const uint8_t PROGMEM icon_temp[] = {
  0x01,0x00, 0x03,0xC0, 0x03,0xC0, 0x03,0xC0,
  0x03,0xC0, 0x03,0xC0, 0x03,0xC0, 0x03,0xC0,
  0x03,0xC0, 0x03,0xC0, 0x06,0x60, 0x0C,0x30,
  0x0C,0x30, 0x06,0x60, 0x03,0xC0, 0x01,0x80,
};
static const uint8_t PROGMEM icon_hum[] = {
  0x00,0x00, 0x00,0x80, 0x01,0xC0, 0x01,0xC0,
  0x03,0xE0, 0x07,0xF0, 0x07,0xF0, 0x0F,0xF8,
  0x0F,0xF8, 0x1F,0xFC, 0x1F,0xFC, 0x1F,0xFC,
  0x0F,0xF8, 0x07,0xF0, 0x03,0xE0, 0x00,0x00,
};
static const uint8_t PROGMEM icon_fan[] = {
  0x00,0xC0, 0x03,0xE0, 0x06,0x40, 0x0C,0xC0,
  0x0C,0xF8, 0x4C,0xFC, 0xFC,0x06, 0xDD,0x82,
  0x41,0xBB, 0x60,0x3E, 0x3F,0x32, 0x1F,0x30,
  0x03,0x30, 0x02,0x60, 0x07,0xC0, 0x01,0x00,
};
static const uint8_t PROGMEM icon_mist[] = {
  0x00,0x00, 0x00,0x00, 0x0F,0x1E, 0x7F,0xFE,
  0x30,0xE0, 0x00,0x00, 0x02,0x00, 0x4F,0xBE,
  0x7D,0xF2, 0x00,0x40, 0x00,0x00, 0x07,0x0C,
  0x7F,0xFE, 0x78,0xF0, 0x00,0x00, 0x00,0x00,
};
static const uint8_t PROGMEM icon_lamp[] = {
  0x00,0x00, 0x03,0x60, 0x03,0x60, 0x7B,0x66,
  0x79,0xB6, 0xC1,0x23, 0xD8,0x03, 0xC2,0x5B,
  0xC3,0xDB, 0xDB,0xDB, 0xC3,0xDB, 0xC0,0x03,
  0x7F,0xFE, 0x7F,0xFE, 0x30,0x0C, 0x00,0x00,
};
static const uint8_t PROGMEM icon_wifi[] = {
  0x00,0x00, 0x00,0x00, 0x01,0x80, 0x1F,0xF8,
  0x3C,0x3C, 0x60,0x06, 0x47,0xE2, 0x1F,0xF8,
  0x38,0x1C, 0x03,0xC0, 0x07,0xE0, 0x04,0x20,
  0x00,0x00, 0x01,0x80, 0x00,0x00, 0x00,0x00,
};
static const uint8_t PROGMEM icon_ap[] = {
  0x00,0x00, 0x60,0x00, 0x71,0x80, 0x3B,0xF8,
  0x3C,0x3C, 0x6E,0x06, 0x47,0x02, 0x1F,0xB8,
  0x39,0xDC, 0x03,0xE0, 0x07,0xF0, 0x04,0x38,
  0x00,0x1C, 0x01,0x8E, 0x00,0x06, 0x00,0x00,
};

bool initDisplay() {
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) return false;
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setCursor(0, 0);  oled.println("Terrario BCC");
  oled.setCursor(0, 12); oled.println("Iniciando...");
  oled.display();
  return true;
}

void displayApMode() {
  oled.clearDisplay();
  oled.drawBitmap(0, 0, icon_ap, 16, 16, SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setCursor(20, 4);  oled.println("Modo Config AP");
  oled.setCursor(0, 20);  oled.print("SSID: "); oled.println(apSSID);
  oled.setCursor(0, 34);  oled.print("Pass: "); oled.println(apPass);
  oled.setCursor(0, 48);  oled.println("IP: 192.168.4.1");
  oled.display();
}

static void drawScreenIndicator() {
  for (uint8_t i = 0; i < 3; i++) {
    if (i == currentScreen) oled.fillCircle(118 + i*5, 3, 2, SSD1306_WHITE);
    else                    oled.drawCircle(118 + i*5, 3, 2, SSD1306_WHITE);
  }
}

static void screenStatus() {
  oled.drawBitmap(0, 0, icon_temp, 16, 16, SSD1306_WHITE);
  oled.setTextSize(2);
  oled.setCursor(20, 2);
  oled.print(temperature, 1);
  oled.print("C");

  oled.drawBitmap(0, 20, icon_hum, 16, 16, SSD1306_WHITE);
  oled.setTextSize(2);
  oled.setCursor(20, 22);
  oled.print(humidity, 1);
  oled.print("%");

  oled.drawBitmap(0, 44, icon_fan, 16, 16, SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setCursor(18, 48);
  if (fanOn) {
    oled.print("ON");
  } else {
    oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    oled.print("OF");
    oled.setTextColor(SSD1306_WHITE);
  }

  oled.drawBitmap(40, 44, icon_mist, 16, 16, SSD1306_WHITE);
  oled.setCursor(58, 48);
  if (humidOn) {
    oled.print("ON");
  } else {
    oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    oled.print("OF");
    oled.setTextColor(SSD1306_WHITE);
  }

  oled.drawBitmap(80, 44, icon_lamp, 16, 16, SSD1306_WHITE);
  oled.setCursor(98, 48);
  if (lampOn) {
    oled.print("ON");
  } else {
    oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    oled.print("OF");
    oled.setTextColor(SSD1306_WHITE);
  }
}

static void screenSetpoints() {
  oled.setTextSize(1);
  oled.setCursor(0,  8); oled.print("Tmin: "); oled.print(tempMin, 1);  oled.print("C");
  oled.setCursor(0, 18); oled.print("Tmax: "); oled.print(tempMax, 1);  oled.print("C");
  oled.setCursor(0, 28); oled.print("Tcrit:"); oled.print(tempCrit, 1); oled.print("C");
  oled.setCursor(0, 38); oled.print("Hmin: "); oled.print(humMin, 0);   oled.print("%");
  oled.setCursor(0, 48); oled.print("Hmax: "); oled.print(humMax, 0);   oled.print("% +/-"); oled.print(humHyst, 0);
  oled.setCursor(70,  8); oled.print("Lamp:");
  oled.setCursor(70, 18); oled.print(lampShouldBeOn ? "desej:ON" : "desej:OF");
  oled.setCursor(70, 28); oled.print(lampOn         ? "real :ON" : "real :OF");
}

static void screenNetwork() {
  oled.setTextSize(1);
  if (WiFi.status() == WL_CONNECTED) {
    oled.drawBitmap(0, 0, icon_wifi, 16, 16, SSD1306_WHITE);
    oled.setCursor(20, 4);  oled.print("WiFi");
    oled.setCursor(0, 20);  oled.print("SSID:");
    oled.setCursor(0, 30);
    String ssid = WiFi.SSID();
    if (ssid.length() > 21) ssid = ssid.substring(0, 21);
    oled.print(ssid);
    oled.setCursor(0, 42); oled.print("IP: ");   oled.print(WiFi.localIP().toString());
    oled.setCursor(0, 54); oled.print("SA01:"); oled.print(tasmotaIP);
  } else {
    oled.drawBitmap(0, 0, icon_ap, 16, 16, SSD1306_WHITE);
    oled.setCursor(20, 4);  oled.print("Modo AP");
    oled.setCursor(0, 20);  oled.print("SSID: "); oled.print(apSSID);
    oled.setCursor(0, 34);  oled.print("Pass: "); oled.print(apPass);
    oled.setCursor(0, 48);  oled.print("IP: 192.168.4.1");
  }
}

void updateDisplay() {
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  drawScreenIndicator();
  switch (currentScreen) {
    case 0: screenStatus();    break;
    case 1: screenSetpoints(); break;
    case 2: screenNetwork();   break;
  }
  oled.display();
}
