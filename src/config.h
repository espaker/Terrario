#pragma once
#include <Arduino.h>

#define PIN_DHT        D5
#define PIN_FAN        D6
#define PIN_HUMIDIFIER D7
#define PIN_BUTTON     D3
#define SDA_PIN        D2
#define SCL_PIN        D1

#define DHT_TYPE DHT22

#define EEPROM_SIZE      112
#define READ_INTERVAL    2000UL
#define TASMOTA_CMD_COOL 10000UL
#define TASMOTA_POLL_INT 15000UL
#define DEBOUNCE_MS      200UL

extern float temperature, humidity;
extern bool  fanOn, humidOn, lampOn, lampShouldBeOn;
extern float humMin, humMax, humHyst, tempMin, tempMax, tempCrit;
extern char  tasmotaIP[16];
extern char  savedSSID[32];
extern char  savedPass[64];
extern unsigned long lastRead, lastTasmotaCmd, lastTasmotaPoll;
extern uint8_t       currentScreen;
extern unsigned long lastButtonPress;
extern String        apSSID, apPass;
