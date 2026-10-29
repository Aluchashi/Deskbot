#pragma once
#include "secrets.h"


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C

// ---- Pins ----
#define SDA_PIN 17
#define SCL_PIN 16
#define BUTTON_PIN 22
#define BUZZER_PIN 21

// ---- Defaults ----
#define DEF_CITY "Rajshahi"
#define DEF_COUNTRY "BD"
#define DEF_TZ "BST-6"  // Bangladesh = UTC+6

// ---- World clock ----
#define ZONE1_NAME "GERMANY"
#define ZONE1_OFFSET 240
#define ZONE2_NAME "SINGAPORE"
#define ZONE2_OFFSET 480

// ---- Setup portal ----
#define CONFIG_AP_SSID "DeskBot"
#define CONFIG_AP_PASS "12345678"

// ---- Colors ----
#define FG SSD1306_WHITE
#define BG SSD1306_BLACK

// ---- Timing ----
#define LONG_PRESS_MS 700
#define DOUBLE_TAP_MS 280
#define NOTIF_MS 8000
#define SNOOZE_MIN 5

// ---- Counts ----
#define PAGE_COUNT 6
#define ALARM_COUNT 3