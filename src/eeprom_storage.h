#pragma once
#include <Arduino.h>

void saveCredentials(const String& ssid, const String& pass, const String& tip = "");
void loadCredentials();
void saveSetpoints();
void loadSetpoints();
