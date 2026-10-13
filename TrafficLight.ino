// Arduino IDE: keep this sketch, config.h and web_ui.h in TrafficLight/.
#include <WiFi.h>
#include <WebServer.h>
#include "config.h"
#include "web_ui.h"
#if ENABLE_OLED
#include <Wire.h>
#include <U8g2lib.h>
#if OLED_SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE, SCL_PIN, SDA_PIN);
#else
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE, SCL_PIN, SDA_PIN);
#endif
#endif
WebServer server(80);
enum Light { RED, GREEN, YELLOW };
Light light = RED;
bool automatic = true, running = false;
uint32_t durations[3] = {10, 10, 3};
uint32_t phaseStart = 0, lastDisplay = 0;
const char* lightName() { return light == RED ? "red" : light == GREEN ? "green" : "yellow"; }
void output() {
  const int pins[] = {RED_PIN, GREEN_PIN, YELLOW_PIN};
  // Turn everything off before enabling the selected lamp.
  for (int pin : pins) digitalWrite(pin, LED_ACTIVE_HIGH ? LOW : HIGH);
  digitalWrite(pins[light], LED_ACTIVE_HIGH ? HIGH : LOW);
}
void selectLight(Light value) { light = value; phaseStart = millis(); output(); }
void advance() {
  if (!automatic || !running) return;
  uint32_t now = millis();
  // Unsigned subtraction handles millis() rollover. Preserve phase boundaries.
  while (uint32_t(now - phaseStart) >= durations[light] * 1000UL) {
    phaseStart += durations[light] * 1000UL;
    light = light == RED ? GREEN : light == GREEN ? YELLOW : RED;
    output();
  }
}
uint32_t remaining() {
  if (!automatic || !running) return 0;
  uint32_t elapsed = uint32_t(millis() - phaseStart);
  uint32_t total = durations[light] * 1000UL;
  return elapsed >= total ? 0 : (total - elapsed + 999) / 1000;
}
void state() {
  advance();
  String json = "{\"mode\":\""; json += automatic ? "auto" : "manual";
  json += "\",\"running\":"; json += running ? "true" : "false";
  json += ",\"light\":\""; json += lightName();
  json += "\",\"remaining\":"; json += remaining();
  json += ",\"durations\":{\"red\":"; json += durations[RED];
  json += ",\"green\":"; json += durations[GREEN];
  json += ",\"yellow\":"; json += durations[YELLOW]; json += "}}";
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", json);
}
void error(const char* message, int code = 400) { server.send(code, "application/json", String("{\"error\":\"") + message + "\"}"); }
bool secondsArg(const char* name, uint32_t& value) {
  String s = server.arg(name);
  if (s.length() == 0 || s.length() > 3) return false;
  for (unsigned int i = 0; i < s.length(); i++) if (s[i] < '0' || s[i] > '9') return false;
  value = s.toInt(); return value >= 1 && value <= 120;
}
void action() {
  advance(); String command = server.arg("command"), value = server.arg("value");
  if (command == "stop") { running = false; selectLight(RED); }
  else if (command == "start") { running = true; selectLight(RED); }
  else if (command == "mode") {
    if (value != "auto" && value != "manual") { error("Invalid mode"); return; }
    automatic = value == "auto"; selectLight(RED);
  } else if (command == "light") {
    if (automatic || !running) { error("Start manual mode first", 409); return; }
    if (value != "red" && value != "green" && value != "yellow") { error("Invalid light"); return; }
    selectLight(value == "red" ? RED : value == "green" ? GREEN : YELLOW);
  } else if (command == "config") {
    uint32_t r, g, y;
    if (!secondsArg("red", r) || !secondsArg("green", g) || !secondsArg("yellow", y)) { error("Use integer seconds from 1 to 120"); return; }
    durations[RED] = r; durations[GREEN] = g; durations[YELLOW] = y;
    selectLight(RED);
  } else { error("Unknown command"); return; }
  state();
}
void display() {
#if ENABLE_OLED
  oled.clearBuffer(); oled.setFont(u8g2_font_6x12_tf);
  oled.drawStr(0, 12, "TRAFFIC LAB");
  oled.drawStr(0, 26, !running ? "STOP / RED" : automatic ? "AUTO" : "MANUAL / HOLD");
  oled.drawStr(0, 40, lightName());
  if (automatic && running) { char count[8]; snprintf(count, sizeof(count), "%lu", (unsigned long)remaining()); oled.setFont(u8g2_font_logisoso24_tf); oled.drawStr(78, 44, count); }
  oled.setFont(u8g2_font_6x12_tf); oled.drawStr(0, 62, "IP: 192.168.4.1"); oled.sendBuffer();
#endif
}
void setup() {
  Serial.begin(115200);
  for (int pin : {RED_PIN, GREEN_PIN, YELLOW_PIN}) { pinMode(pin, OUTPUT); digitalWrite(pin, LED_ACTIVE_HIGH ? LOW : HIGH); }
  selectLight(RED);
#if ENABLE_OLED
  Wire.begin(SDA_PIN, SCL_PIN); oled.setI2CAddress(OLED_ADDRESS << 1); oled.begin();
#endif
  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP(AP_NAME, AP_PASSWORD)) { Serial.println("AP failed; red lamp held"); display(); return; }
  Serial.print("Wi-Fi: "); Serial.println(AP_NAME); Serial.print("URL: http://"); Serial.println(WiFi.softAPIP());
  server.on("/", HTTP_GET, []() { server.sendHeader("Cache-Control", "no-store"); server.send_P(200, "text/html; charset=utf-8", WEB_UI); });
  server.on("/api/state", HTTP_GET, state);
  server.on("/api/action", HTTP_POST, action);
  server.onNotFound([]() { error("Not found", 404); });
  server.begin(); display();
}
void loop() {
  advance(); server.handleClient(); advance();
  if (uint32_t(millis() - lastDisplay) >= 200) { lastDisplay = millis(); display(); }
}
