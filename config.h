#pragma once
// CIRCUS Pi EZ Start Kit+ / NodeMCU-32S: official pin map.
constexpr int RED_PIN = 16;
constexpr int YELLOW_PIN = 12;
constexpr int GREEN_PIN = 13;
constexpr int SDA_PIN = 21;
constexpr int SCL_PIN = 22;
constexpr bool LED_ACTIVE_HIGH = true;
// Change to false to compile without an OLED library.
#define ENABLE_OLED 1
// 1 = SH1106, 0 = SSD1306. Check your display variant if blank.
#define OLED_SH1106 1
constexpr unsigned char OLED_ADDRESS = 0x3C;
constexpr char AP_NAME[] = "ESP32-Traffic-Lab";
// Classroom default, change before use; 8 or more characters.
constexpr char AP_PASSWORD[] = "traffic123";
