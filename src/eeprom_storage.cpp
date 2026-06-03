#include "eeprom_storage.h"
#include "config.h"
#include <EEPROM.h>

void saveCredentials(const String& ssid, const String& pass, const String& tip) {
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < 32; i++) EEPROM.write(i,    i < (int)ssid.length() ? ssid[i] : 0);
  for (int i = 0; i < 64; i++) EEPROM.write(32+i, i < (int)pass.length() ? pass[i] : 0);
  if (tip.length() > 0)
    for (int i = 0; i < 16; i++) EEPROM.write(96+i, i < (int)tip.length() ? tip[i] : 0);
  EEPROM.commit();
  EEPROM.end();
}

void loadCredentials() {
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < 32; i++) savedSSID[i] = EEPROM.read(i);
  for (int i = 0; i < 64; i++) savedPass[i]  = EEPROM.read(32+i);
  for (int i = 0; i < 16; i++) tasmotaIP[i]  = EEPROM.read(96+i);
  savedSSID[31] = '\0';
  savedPass[63] = '\0';
  tasmotaIP[15] = '\0';
  if (tasmotaIP[0] == 0 || tasmotaIP[0] == 0xFF)
    strncpy(tasmotaIP, "192.168.100.87", 15);
  EEPROM.end();
}
