#pragma once


inline void setBrightness(Adafruit_SSD1306& d, bool high) {
  d.ssd1306_command(SSD1306_SETCONTRAST);
  d.ssd1306_command(high ? 0xFF : 0x01);
}


inline String urlEncodeSpaces(String s) {
  s.replace(" ", "%20");
  return s;
}

inline String esc(const String& s) {
  String o;
  o.reserve(s.length() + 8);
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '"') o += "&quot;";
    else if (c == '\'') o += "&#39;";
    else if (c == '<') o += "&lt;";
    else if (c == '&') o += "&amp;";
    else o += c;
  }
  return o;
}

// ---- টেক্সট হেল্পার ----
inline String cleanText(const String& s, size_t maxLen) {
  String out;
  out.reserve(maxLen);
  for (size_t i = 0; i < s.length() && out.length() < maxLen; i++) {
    uint8_t c = (uint8_t)s[i];
    if (c >= 32 && c < 127) out += (char)c;
    else if (c == '\n' || c == '\r' || c == '\t') out += ' ';
  }
  out.trim();
  return out;
}

inline int wrapText(const String& s, String* lines, int maxLines, int maxChars) {
  int n = 0, i = 0, len = s.length();
  String cur = "";
  while (i < len && n < maxLines) {
    int j = i;
    while (j < len && s[j] != ' ') j++;
    String word = s.substring(i, j);
    i = j;
    while (i < len && s[i] == ' ') i++;
    while ((int)word.length() > maxChars) {
      if (cur.length()) {
        lines[n++] = cur;
        cur = "";
        if (n >= maxLines) return n;
      }
      lines[n++] = word.substring(0, maxChars);
      word = word.substring(maxChars);
      if (n >= maxLines) return n;
    }
    if (cur.length() == 0) cur = word;
    else if ((int)(cur.length() + 1 + word.length()) <= maxChars) cur += " " + word;
    else {
      lines[n++] = cur;
      cur = word;
    }
  }
  if (cur.length() && n < maxLines) lines[n++] = cur;
  return n;
}