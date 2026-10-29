#pragma once

#define W_CLEAR 0
#define W_CLOUD 1
#define W_RAIN 2
#define W_STORM 3
#define W_SNOW 4
#define W_MIST 5

inline int weatherType(const String& w) {
  if (w == "Clear") return W_CLEAR;
  if (w == "Clouds") return W_CLOUD;
  if (w == "Rain" || w == "Drizzle") return W_RAIN;
  if (w == "Thunderstorm") return W_STORM;
  if (w == "Snow") return W_SNOW;
  return W_MIST;
}

static inline int sc(float v, float k) {
  return (int)(v * k + 0.5f);
}

inline void drawCloudShape(int x, int y, float k, int dy) {
  display.fillCircle(x + sc(11, k), y + sc(17 + dy, k), max(1, sc(6, k)), FG);
  display.fillCircle(x + sc(18, k), y + sc(13 + dy, k), max(1, sc(8, k)), FG);
  display.fillCircle(x + sc(25, k), y + sc(19 + dy, k), max(1, sc(5, k)), FG);
  display.fillRoundRect(x + sc(5, k), y + sc(19 + dy, k), max(2, sc(26, k)), max(2, sc(9, k)), max(1, sc(4, k)), FG);
}

// size: 32 (big) or 16 (small)
inline void drawWeatherIcon(int x, int y, int size, int type, bool night) {
  float k = size / 32.0f;
  switch (type) {
    case W_CLEAR:
      if (night) {
        display.fillCircle(x + sc(15, k), y + sc(16, k), sc(9, k), FG);
        display.fillCircle(x + sc(21, k), y + sc(12, k), sc(8, k), BG);
      } else {
        int cx = x + sc(16, k), cy = y + sc(16, k);
        display.fillCircle(cx, cy, max(2, sc(7, k)), FG);
        for (int a = 0; a < 8; a++) {
          float r = a * PI / 4.0f;
          display.drawLine(cx + (int)(cosf(r) * 10 * k), cy + (int)(sinf(r) * 10 * k),
                           cx + (int)(cosf(r) * 14 * k), cy + (int)(sinf(r) * 14 * k), FG);
        }
      }
      break;
    case W_CLOUD:
      drawCloudShape(x, y, k, 0);
      break;
    case W_RAIN:
      drawCloudShape(x, y, k, -5);
      for (int i = 0; i < 3; i++) {
        int x0 = 9 + i * 7;
        display.drawLine(x + sc(x0, k), y + sc(25, k), x + sc(x0 - 2, k), y + sc(30, k), FG);
      }
      break;
    case W_STORM:
      drawCloudShape(x, y, k, -5);
      display.fillTriangle(x + sc(18, k), y + sc(22, k), x + sc(12, k), y + sc(29, k), x + sc(17, k), y + sc(29, k), FG);
      display.fillTriangle(x + sc(15, k), y + sc(27, k), x + sc(21, k), y + sc(27, k), x + sc(14, k), y + sc(31, k), FG);
      break;
    case W_SNOW:
      drawCloudShape(x, y, k, -5);
      display.fillCircle(x + sc(10, k), y + sc(27, k), max(1, sc(1.5f, k)), FG);
      display.fillCircle(x + sc(17, k), y + sc(30, k), max(1, sc(1.5f, k)), FG);
      display.fillCircle(x + sc(24, k), y + sc(27, k), max(1, sc(1.5f, k)), FG);
      break;
    default:
      for (int i = 0; i < 4; i++) {
        int yy = y + sc(9 + i * 6, k);
        int xx = x + sc((i % 2) ? 7 : 3, k);
        display.drawFastHLine(xx, yy, sc(22, k), FG);
        if (size >= 24) display.drawFastHLine(xx, yy + 1, sc(22, k), FG);
      }
      break;
  }
}