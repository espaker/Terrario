#include "config.h"

float temperature    = 0;
float humidity       = 0;
bool  fanOn          = false;
bool  humidOn        = false;
bool  lampOn         = false;
bool  lampShouldBeOn = true;
float humMin   = 60.0;
float humMax   = 80.0;
float humHyst  =  5.0;
float tempMin  = 24.0;
float tempMax  = 32.0;
float tempCrit = 36.0;
char  tasmotaIP[16] = "192.168.100.87";
char  savedSSID[32] = {};
char  savedPass[64] = {};
unsigned long lastRead        = 0;
unsigned long lastTasmotaCmd  = 0;
unsigned long lastTasmotaPoll = 0;
uint8_t       currentScreen   = 0;
unsigned long lastButtonPress  = 0;
String        apSSID           = "";
String        apPass           = "";
