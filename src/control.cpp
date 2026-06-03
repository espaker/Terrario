#include "control.h"
#include "config.h"
#include "tasmota.h"

void controlLoop() {
  if (isnan(temperature) || isnan(humidity)) return;

  if      (humidity > humMax)           fanOn = true;
  else if (humidity < humMax - humHyst) fanOn = false;
  digitalWrite(PIN_FAN, fanOn ? HIGH : LOW);

  if      (humidity < humMin)           humidOn = true;
  else if (humidity > humMin + humHyst) humidOn = false;
  digitalWrite(PIN_HUMIDIFIER, humidOn ? HIGH : LOW);

  if (temperature > tempCrit)
    lampShouldBeOn = false;
  else if (temperature < tempMin)
    lampShouldBeOn = true;
  else if (temperature < tempCrit - 2.0)
    lampShouldBeOn = true;

  if (lampShouldBeOn != lampOn) {
    if (tasmotaCommand(lampShouldBeOn))
      Serial.printf("Lamp corrigida para %s (T=%.1f)\n",
        lampShouldBeOn ? "ON" : "OFF", temperature);
  }
}
