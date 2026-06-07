#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include "config.h"
#include "eeprom_storage.h"
#include "display.h"
#include "tasmota.h"
#include "control.h"
#include "web.h"

DHT dht(PIN_DHT, DHT_TYPE);

void setup() {
  Serial.begin(115200);
  pinMode(PIN_FAN,        OUTPUT); digitalWrite(PIN_FAN,        LOW);
  pinMode(PIN_HUMIDIFIER, OUTPUT); digitalWrite(PIN_HUMIDIFIER, LOW);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  dht.begin();

  Wire.begin(SDA_PIN, SCL_PIN);
  if (!initDisplay()) {
    Serial.println("OLED nao encontrado!"); while (true);
  }

  loadCredentials();

  String mac = WiFi.macAddress();
  mac.replace(":", "");
  mac.toUpperCase();
  apSSID = "AP_Terrario_" + mac.substring(0, 4);
  apPass  = "Lucifer_"     + mac.substring(8, 12);

  if (strlen(savedSSID) > 0) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(savedSSID, savedPass);
    unsigned long t = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t < 10000) {
      delay(500); Serial.print(".");
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSSID.c_str(), apPass.c_str());
    displayApMode();
  } else {
    Serial.printf("\nIP: %s\n", WiFi.localIP().toString().c_str());
    delay(500);
    tasmotaPollStatus();
  }

  initWebServer();
}

void loop() {
  webHandleClient();

  if (digitalRead(PIN_BUTTON) == LOW) {
    if (millis() - lastButtonPress > DEBOUNCE_MS) {
      lastButtonPress = millis();
      currentScreen = (currentScreen + 1) % 4;
      updateDisplay();
    }
  }

  if (WiFi.status() == WL_CONNECTED &&
      millis() - lastTasmotaPoll >= TASMOTA_POLL_INT) {
    lastTasmotaPoll = millis();
    tasmotaPollStatus();
  }

  if (currentScreen == 3 && millis() - lastSnakeStep >= SNAKE_INTERVAL) {
    lastSnakeStep = millis();
    snakeStep();
    updateDisplay();
  }

  if (millis() - lastRead >= READ_INTERVAL) {
    lastRead = millis();
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t)) temperature = t;
    if (!isnan(h)) humidity    = h;
    controlLoop();
    updateDisplay();
    Serial.printf("T:%.1f H:%.1f Fan:%s Umid:%s Lamp:%s(desej:%s)\n",
      temperature, humidity,
      fanOn ? "ON" : "OFF", humidOn ? "ON" : "OFF",
      lampOn ? "ON" : "OFF", lampShouldBeOn ? "ON" : "OFF");
  }
}
