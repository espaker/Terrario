#include "web.h"
#include "config.h"
#include "eeprom_storage.h"
#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>

static ESP8266WebServer server(80);

const char CONFIG_PAGE[] PROGMEM = R"rawhtml(
<!DOCTYPE html><html lang="pt-BR">
<head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Terrário Config</title>
<style>*{box-sizing:border-box;margin:0;padding:0}body{font-family:sans-serif;background:#1a1a2e;color:#eee;display:flex;justify-content:center;align-items:center;min-height:100vh}.card{background:#16213e;border-radius:12px;padding:24px;width:100%;max-width:360px}h1{text-align:center;color:#a0c4ff;margin-bottom:20px;font-size:1.2rem}label{font-size:.85rem;color:#888;display:block;margin-bottom:4px}input{width:100%;padding:10px;border-radius:8px;border:1px solid #333;background:#0f3460;color:#eee;margin-bottom:16px}button{width:100%;padding:12px;border:none;border-radius:8px;background:#a0c4ff;color:#1a1a2e;font-weight:bold;cursor:pointer}</style>
</head><body><div class="card"><h1>🦎 Terrário BCC</h1>
<form action="/save" method="POST">
<label>SSID</label><input type="text" name="ssid" placeholder="Nome da rede" required>
<label>Senha WiFi</label><input type="password" name="pass">
<label>Senha do portal</label><input type="password" name="appass" placeholder="lucifer">
<label>IP do Tasmota (SA-01)</label><input type="text" name="tip" placeholder="192.168.x.x">
<button type="submit">Salvar e conectar</button>
</form></div></body></html>
)rawhtml";

const char DASHBOARD[] PROGMEM = R"rawhtml(
<!DOCTYPE html><html lang="pt-BR">
<head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Terrário BCC</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:sans-serif;background:#1a1a2e;color:#eee;padding:16px}
h1{text-align:center;color:#a0c4ff;margin-bottom:16px;font-size:1.3rem}
.grid{display:grid;grid-template-columns:1fr 1fr;gap:10px;max-width:420px;margin:0 auto 16px}
.card{background:#16213e;border-radius:12px;padding:14px;text-align:center}
.label{font-size:.72rem;color:#888;margin-bottom:4px}
.value{font-size:1.8rem;font-weight:bold}
.temp{color:#ff9a3c}.hum{color:#4fc3f7}.on{color:#69f0ae}.off{color:#555}.warn{color:#ff5252}
.status{font-size:.82rem}
.alert{background:#3a1a1a;border:1px solid #ff5252;border-radius:10px;padding:10px;text-align:center;margin:0 auto 16px;max-width:420px;color:#ff5252;font-weight:bold}
form{max-width:420px;margin:0 auto;background:#16213e;border-radius:12px;padding:16px}
form h2{font-size:.95rem;margin-bottom:12px;color:#a0c4ff}
.field{display:flex;justify-content:space-between;align-items:center;margin-bottom:10px;font-size:.82rem}
input[type=number]{width:80px;padding:4px 8px;border-radius:6px;border:1px solid #333;background:#0f3460;color:#eee}
button{width:100%;margin-top:10px;padding:10px;border:none;border-radius:8px;background:#a0c4ff;color:#1a1a2e;font-weight:bold;cursor:pointer}
.info{max-width:420px;margin:12px auto 0;font-size:.72rem;color:#555;text-align:center}
</style></head><body>
<h1>🦎 Terrário BCC</h1>
ALERT_BLOCK
<div class="grid">
<div class="card"><div class="label">🌡️ Temperatura</div><div class="value temp">TEMP_VAL°C</div></div>
<div class="card"><div class="label">💧 Umidade</div><div class="value hum">HUM_VAL%</div></div>
<div class="card"><div class="label">🌀 Exaustora</div><div class="value status FAN_CLASS">FAN_VAL</div></div>
<div class="card"><div class="label">💨 Umidificador</div><div class="value status HUMID_CLASS">HUMID_VAL</div></div>
<div class="card"><div class="label">🔥 Lâmpada</div><div class="value status LAMP_CLASS">LAMP_VAL</div></div>
<div class="card"><div class="label">📋 Faixa BCC</div><div class="value status" style="font-size:.7rem;color:#888">TMIN-TMAX°C<br>HMIN-HMAX%</div></div>
</div>
<form action="/setpoints" method="GET">
<h2>⚙️ Setpoints</h2>
<div class="field"><label>🌡️ Temp mín °C (religa lamp.)</label><input type="number" name="tempMin" step="0.5" value="TMIN_VAL"></div>
<div class="field"><label>🌡️ Temp máx °C (referência)</label><input type="number" name="tempMax" step="0.5" value="TMAX_VAL"></div>
<div class="field"><label>⚠️ Temp crítica °C (corta lamp.)</label><input type="number" name="tempCrit" step="0.5" value="TCRIT_VAL"></div>
<div class="field"><label>💧 Umid mín % (liga umidif.)</label><input type="number" name="humMin" step="1" value="HMIN_VAL"></div>
<div class="field"><label>💧 Umid máx % (liga exaust.)</label><input type="number" name="humMax" step="1" value="HMAX_VAL"></div>
<div class="field"><label>↕️ Histerese umid % (desliga)</label><input type="number" name="humHyst" step="1" value="HHYST_VAL"></div>
<div class="field"><label>🔌 IP Tasmota (SA-01)</label><input type="text" name="tip" style="width:120px" value="TIP_VAL"></div>
<button type="submit">💾 Salvar</button>
</form>
<div class="info">Atualiza a cada 5s · poll Tasmota 15s · /api JSON</div>
<script>
function updateValues() {
  fetch('/api')
    .then(r => r.json())
    .then(d => {
      document.querySelectorAll('.value.temp')[0].textContent = d.temp + '°C';
      document.querySelectorAll('.value.hum')[0].textContent = d.hum + '%';

      var fan = document.querySelectorAll('.value.status')[0];
      fan.textContent = d.fan ? 'LIGADA' : 'DESLIG.';
      fan.className = 'value status ' + (d.fan ? 'on' : 'off');

      var umid = document.querySelectorAll('.value.status')[1];
      umid.textContent = d.humidifier ? 'LIGADO' : 'DESLIG.';
      umid.className = 'value status ' + (d.humidifier ? 'on' : 'off');

      var lamp = document.querySelectorAll('.value.status')[2];
      lamp.textContent = d.lampOn ? 'LIGADA' : 'DESLIG.';
      lamp.className = 'value status ' + (d.lampOn ? 'on' : (!d.lampShould ? 'warn' : 'off'));

      var alert = document.querySelector('.alert');
      if (!d.lampShould && !alert) {
        var a = document.createElement('div');
        a.className = 'alert';
        a.textContent = '⚠️ TEMPERATURA CRÍTICA — LÂMPADA CORTADA';
        document.querySelector('h1').after(a);
      } else if (d.lampShould && alert) {
        alert.remove();
      }
    })
    .catch(function(){});
}
setInterval(updateValues, 5000);
</script>
</body></html>
)rawhtml";

static void handleRoot() {
  if (WiFi.status() != WL_CONNECTED) { server.send_P(200, "text/html", CONFIG_PAGE); return; }
  String page = String(DASHBOARD);
  bool lampCut = !lampShouldBeOn;
  String alert = lampCut ? "<div class=\"alert\">⚠️ TEMPERATURA CRÍTICA — LÂMPADA CORTADA</div>" : "";
  page.replace("ALERT_BLOCK",  alert);
  page.replace("TEMP_VAL",     isnan(temperature) ? "--" : String(temperature, 1));
  page.replace("HUM_VAL",      isnan(humidity)    ? "--" : String(humidity, 1));
  page.replace("FAN_VAL",      fanOn   ? "LIGADA" : "DESLIG.");
  page.replace("FAN_CLASS",    fanOn   ? "on"     : "off");
  page.replace("HUMID_VAL",    humidOn ? "LIGADO" : "DESLIG.");
  page.replace("HUMID_CLASS",  humidOn ? "on"     : "off");
  page.replace("LAMP_VAL",     lampOn  ? "LIGADA" : "DESLIG.");
  page.replace("LAMP_CLASS",   lampOn  ? "on"     : (lampCut ? "warn" : "off"));
  page.replace("TMIN_VAL",     String(tempMin,  1));
  page.replace("TMAX_VAL",     String(tempMax,  1));
  page.replace("TCRIT_VAL",    String(tempCrit, 1));
  page.replace("HMIN_VAL",     String(humMin,   0));
  page.replace("HMAX_VAL",     String(humMax,   0));
  page.replace("HHYST_VAL",    String(humHyst,  0));
  page.replace("TIP_VAL",      String(tasmotaIP));
  page.replace("TMIN-TMAX",    String(tempMin, 0) + "-" + String(tempMax, 0));
  page.replace("HMIN-HMAX",    String(humMin, 0)  + "-" + String(humMax, 0));
  server.send(200, "text/html", page);
}

static void handleSetpoints() {
  auto argf = [&](const char* k, float& v) {
    if (server.hasArg(k) && server.arg(k).length() > 0) v = server.arg(k).toFloat();
  };
  argf("tempMin",  tempMin);
  argf("tempMax",  tempMax);
  argf("tempCrit", tempCrit);
  argf("humMin",   humMin);
  argf("humMax",   humMax);
  argf("humHyst",  humHyst);
  if (server.hasArg("tip") && server.arg("tip").length() > 0) {
    server.arg("tip").toCharArray(tasmotaIP, 16);
    saveCredentials(savedSSID, savedPass, server.arg("tip"));
  }
  saveSetpoints();
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

static void handleSave() {
  if (server.hasArg("appass") && server.arg("appass") != "lucifer" && server.arg("appass") != "") {
    server.send(403, "text/plain", "Senha incorreta"); return;
  }
  saveCredentials(server.arg("ssid"), server.arg("pass"), server.arg("tip"));
  if (server.hasArg("tip") && server.arg("tip").length() > 0)
    server.arg("tip").toCharArray(tasmotaIP, 16);
  server.send(200, "text/html",
    "<html><body style='background:#1a1a2e;color:#eee;font-family:sans-serif;"
    "text-align:center;padding-top:40px'>"
    "<h2 style='color:#69f0ae'>Salvo!</h2><p>Reiniciando...</p></body></html>");
  delay(2000);
  ESP.restart();
}

static void handleApi() {
  String json = "{";
  json += "\"temp\":"       + String(temperature, 1) + ",";
  json += "\"hum\":"        + String(humidity, 1)    + ",";
  json += "\"fan\":"        + String(fanOn    ? "true" : "false") + ",";
  json += "\"humidifier\":" + String(humidOn  ? "true" : "false") + ",";
  json += "\"lampOn\":"     + String(lampOn   ? "true" : "false") + ",";
  json += "\"lampShould\":" + String(lampShouldBeOn ? "true" : "false") + ",";
  json += "\"tempMin\":"    + String(tempMin,  1) + ",";
  json += "\"tempMax\":"    + String(tempMax,  1) + ",";
  json += "\"tempCrit\":"   + String(tempCrit, 1) + ",";
  json += "\"humMin\":"     + String(humMin,   0) + ",";
  json += "\"humMax\":"     + String(humMax,   0) + ",";
  json += "\"humHyst\":"    + String(humHyst,  0);
  json += "}";
  server.send(200, "application/json", json);
}

void initWebServer() {
  server.on("/",          handleRoot);
  server.on("/save",      HTTP_POST, handleSave);
  server.on("/setpoints", handleSetpoints);
  server.on("/api",       handleApi);
  server.begin();
}

void webHandleClient() {
  server.handleClient();
}
