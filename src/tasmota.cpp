#include "tasmota.h"
#include "config.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

bool tasmotaCommand(bool on) {
  if (WiFi.status() != WL_CONNECTED) return false;
  if (millis() - lastTasmotaCmd < TASMOTA_CMD_COOL) return false;
  lastTasmotaCmd = millis();
  WiFiClient client;
  HTTPClient http;
  String url = "http://" + String(tasmotaIP) + "/cm?cmnd=Power%20" + (on ? "ON" : "OFF");
  http.begin(client, url);
  int code = http.GET();
  String body = http.getString();
  http.end();
  Serial.printf("Tasmota cmd %s -> %d: %s\n", on ? "ON" : "OFF", code, body.c_str());
  if (code == 200) { lampOn = on; return true; }
  return false;
}

void tasmotaPollStatus() {
  if (WiFi.status() != WL_CONNECTED) return;
  WiFiClient client;
  HTTPClient http;
  String url = "http://" + String(tasmotaIP) + "/cm?cmnd=Power";
  http.begin(client, url);
  int code = http.GET();
  if (code == 200) {
    String body = http.getString();
    lampOn = body.indexOf("\"ON\"") >= 0;
    Serial.printf("Tasmota poll -> %s\n", lampOn ? "ON" : "OFF");
  }
  http.end();
}
