#pragma once

inline void drawWifi(int x, int y) {
  if (WiFi.status() != WL_CONNECTED) {
    display.drawLine(x + 2, y, x + 9, y + 7, FG);
    display.drawLine(x + 9, y, x + 2, y + 7, FG);
    return;
  }
  int rssi = WiFi.RSSI();
  int bars = rssi > -55 ? 4 : rssi > -65 ? 3 : rssi > -75 ? 2 : 1;
  for (int i = 0; i < 4; i++) {
    int h = 2 + i * 2;
    if (i < bars) display.fillRect(x + i * 4, y + 8 - h, 3, h, FG);
    else display.drawFastHLine(x + i * 4, y + 7, 3, FG);
  }
}

inline void drawBell(int x, int y, uint16_t c) {
  display.fillRoundRect(x + 2, y, 7, 7, 3, c);
  display.fillRect(x, y + 6, 11, 2, c);
  display.fillCircle(x + 5, y + 9, 1, c);
}

inline void drawHeart(int x, int y) {
  display.fillCircle(x + 2, y + 2, 2, FG);
  display.fillCircle(x + 5, y + 2, 2, FG);
  display.fillTriangle(x, y + 3, x + 7, y + 3, x + 3, y + 8, FG);
}

inline void drawDrop(int x, int y) {
  display.fillCircle(x + 3, y + 5, 2, FG);
  display.fillTriangle(x + 3, y, x + 1, y + 4, x + 5, y + 4, FG);
}

inline void printCentered(const String& s, int cx, int y) {
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(s, 0, y, &x1, &y1, &w, &h);
  display.setCursor(cx - (int)(w / 2) - x1, y);
  display.print(s);
}

inline void drawHeader(const char* title) {
  display.setFont(NULL);
  display.setTextSize(1);
  display.setTextColor(FG);
  display.setCursor(0, 1);
  display.print(title);
  drawWifi(111, 1);
  display.drawFastHLine(0, 10, 128, FG);
}

// ছোট ডিগ্রি সাইন + C
inline void printTempSmall(int t) {
  display.print(t);
  int cx = display.getCursorX();
  int cy = display.getCursorY();
  display.drawCircle(cx + 2, cy + 1, 1, FG);
  display.setCursor(cx + 5, cy);
  display.print("C");
}