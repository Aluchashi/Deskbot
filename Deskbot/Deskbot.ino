#include <ESPmDNS.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <time.h>
#include <math.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

#include "src/config.h"  // pins, defaults, zones, colors, timing, counts

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// These headers use the global `display`, so they must come after it
#include "src/helpers.h"    // text/url/html helpers, setBrightness()
#include "src/icons.h"      // weather icons
#include "src/ui_widgets.h" // wifi, bell, heart, drop, header, printCentered...
#include "src/dashboard.h"  // DASH_HTML (phone dashboard page)

Preferences prefs;
WebServer server(80);
bool inConfigMode = false;

// ---- Pages ----
#define PAGE_EYES 0
#define PAGE_CLOCK 1
#define PAGE_WEATHER 2
#define PAGE_FORECAST 3
#define PAGE_WORLD 4
#define PAGE_POMO 5
int currentPage = PAGE_EYES;
bool highBrightness = true;

// ---- Moods ----
#define MOOD_NORMAL 0
#define MOOD_HAPPY 1
#define MOOD_SURPRISED 2
#define MOOD_SLEEPY 3
#define MOOD_ANGRY 4
#define MOOD_SAD 5
#define MOOD_EXCITED 6
#define MOOD_LOVE 7
#define MOOD_SUSPICIOUS 8
const char* moodNames[] = { "NORMAL", "HAPPY", "SURPRISED", "SLEEPY", "ANGRY", "SAD", "EXCITED", "LOVE", "SUSPICIOUS" };
int currentMood = MOOD_NORMAL;

// ---- Settings (loaded from flash) ----
String wifiSsid, wifiPass, apiKey, city, countryCode, tzString;

// ---- Weather data ----
bool weatherLoaded = false;
bool forecastLoaded = false;
unsigned long lastWeatherUpdate = 0;
float temperature = 0, feelsLike = 0;
int humidity = 0;
String weatherMain = "Clouds";
String weatherDesc = "Loading";

struct ForecastDay {
  String dayName;
  int temp;
  String main;
};
ForecastDay fcast[3];

// ---- Alarms ----
struct AlarmSlot {
  uint8_t h, m, days;  // days: bit0 = Sunday ... bit6 = Saturday, 0 = ring once
  bool en;
  long lastKey;
};
AlarmSlot alarms[ALARM_COUNT];
bool alarmRinging = false;
int ringIdx = 0;
unsigned long ringStart = 0;
unsigned long nextRingBeep = 0;
time_t snoozeAt = 0;
int snoozeIdx = 0;

// ---- Pomodoro ----
#define POMO_WORK 0
#define POMO_BREAK 1
#define POMO_LONG 2
int pomoWorkMin = 25;
int pomoBreakMin = 5;
int pomoLongMin = 15;
int pomoRounds = 4;
int pomoPhase = POMO_WORK;
int pomoDone = 0;
bool pomoStarted = false;
bool pomoRunning = false;
unsigned long pomoEnd = 0;
unsigned long pomoLeftMs = 0;

// Eye must be declared before any function
struct Eye {
  float x, y, w, h;
  float tx, ty, tw, th;
  float px, py, tpx, tpy;
  float vx, vy, vw, vh, pvx, pvy;
  bool blinking;
  unsigned long lastBlink, nextBlink;

  void init(float _x, float _y, float _w, float _h) {
    x = tx = _x;
    y = ty = _y;
    w = tw = _w;
    h = th = _h;
    px = py = tpx = tpy = 0;
    vx = vy = vw = vh = pvx = pvy = 0;
    blinking = false;
    lastBlink = 0;
    nextBlink = millis() + random(1000, 4000);
  }

  void update() {
    const float k = 0.12f, d = 0.60f, pk = 0.08f, pd = 0.50f;
    vx = (vx + (tx - x) * k) * d;
    vy = (vy + (ty - y) * k) * d;
    vw = (vw + (tw - w) * k) * d;
    vh = (vh + (th - h) * k) * d;
    x += vx;
    y += vy;
    w += vw;
    h += vh;
    pvx = (pvx + (tpx - px) * pk) * pd;
    pvy = (pvy + (tpy - py) * pk) * pd;
    px += pvx;
    py += pvy;
  }
};

const char* dayShort[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
const char* dayFull[] = { "SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY" };
const char* monthShort[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };

// ---- Notification ----
bool notifActive = false;
unsigned long notifStart = 0;
String notifTitle = "";
String notifMsg = "";

// ---- Button ----
bool btnDown = false;
bool longDone = false;
unsigned long btnPressAt = 0;
unsigned long lastRelease = 0;
uint8_t tapCount = 0;

// ---- Overlay (page dots / toast) ----
unsigned long overlayUntil = 0;
char toastText[16] = "";

// ---- Buzzer sequencer (non-blocking) ----
int beepSteps = 0;
bool beepState = false;
unsigned long beepNext = 0;

Eye leftEye, rightEye;
unsigned long lastSaccade = 0;
unsigned long saccadeInterval = 2000;
float headX = 0, headY = 0;


void startBeep(int count) {
  beepSteps = count * 2;
  beepState = false;
  beepNext = millis();
}

void updateBuzzer() {
  if (beepSteps <= 0) return;
  unsigned long now = millis();
  if (now < beepNext) return;
  beepState = !beepState;
  digitalWrite(BUZZER_PIN, beepState ? HIGH : LOW);
  beepNext = now + (beepState ? 90 : 110);
  beepSteps--;
  if (beepSteps == 0) digitalWrite(BUZZER_PIN, LOW);
}

void showToast(const char* s) {
  strncpy(toastText, s, 15);
  toastText[15] = 0;
  overlayUntil = millis() + 1200;
}

void showDots() {
  toastText[0] = 0;
  overlayUntil = millis() + 1800;
}


bool isNightNow() {
  struct tm t;
  if (!getLocalTime(&t, 0)) return false;
  return (t.tm_hour < 6 || t.tm_hour >= 18);
}

//Flash Configure
void loadConfig() {
  prefs.begin("deskbot", true);
  wifiSsid = prefs.getString("ssid", DEF_WIFI_SSID);
  wifiPass = prefs.getString("pass", DEF_WIFI_PASS);
  apiKey = prefs.getString("apikey", DEF_API_KEY);
  city = prefs.getString("city", DEF_CITY);
  countryCode = prefs.getString("country", DEF_COUNTRY);
  tzString = prefs.getString("tz", DEF_TZ);
  for (int i = 0; i < ALARM_COUNT; i++) {
    String k = "a" + String(i);
    alarms[i].h = prefs.getUChar((k + "h").c_str(), 7);
    alarms[i].m = prefs.getUChar((k + "m").c_str(), 0);
    alarms[i].days = prefs.getUChar((k + "d").c_str(), 0);
    alarms[i].en = prefs.getBool((k + "e").c_str(), false);
    alarms[i].lastKey = -1;
  }
  pomoWorkMin = prefs.getUChar("pw", 25);
  pomoBreakMin = prefs.getUChar("pb", 5);
  pomoLongMin = prefs.getUChar("pl", 15);
  pomoRounds = prefs.getUChar("pr", 4);
  prefs.end();
}

void saveConfig(const String& s, const String& p, const String& ak, const String& c, const String& cc, const String& tz) {
  prefs.begin("deskbot", false);
  prefs.putString("ssid", s);
  prefs.putString("pass", p);
  prefs.putString("apikey", ak);
  prefs.putString("city", c);
  prefs.putString("country", cc);
  prefs.putString("tz", tz);
  prefs.end();
}

void saveAlarm(int i) {
  prefs.begin("deskbot", false);
  String k = "a" + String(i);
  prefs.putUChar((k + "h").c_str(), alarms[i].h);
  prefs.putUChar((k + "m").c_str(), alarms[i].m);
  prefs.putUChar((k + "d").c_str(), alarms[i].days);
  prefs.putBool((k + "e").c_str(), alarms[i].en);
  prefs.end();
}

void savePomo() {
  prefs.begin("deskbot", false);
  prefs.putUChar("pw", pomoWorkMin);
  prefs.putUChar("pb", pomoBreakMin);
  prefs.putUChar("pl", pomoLongMin);
  prefs.putUChar("pr", pomoRounds);
  prefs.end();
}

void handleConfigRoot() {
  String h = F(
    "<!DOCTYPE html><html><head><meta charset='utf-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'><title>Desk Bot Setup</title><style>"
    "body{font-family:system-ui,sans-serif;background:#0b1220;color:#e6f0ff;margin:0;padding:20px}"
    ".c{max-width:420px;margin:auto;background:#121c33;border:1px solid #23325a;border-radius:14px;padding:22px}"
    "h1{margin:0 0 4px;font-size:22px;color:#6cb6ff}p{margin:0;color:#8aa3c8;font-size:13px}"
    "h2{margin:22px 0 0;font-size:13px;color:#6cb6ff;border-top:1px solid #23325a;padding-top:14px}"
    "label{display:block;margin-top:12px;font-size:13px;color:#9db7e0}"
    "input{width:100%;box-sizing:border-box;padding:11px;margin-top:5px;border-radius:8px;"
    "border:1px solid #2b3d6b;background:#0b1220;color:#e6f0ff}"
    "button{width:100%;margin-top:22px;padding:13px;border:0;border-radius:8px;background:#2f81f7;color:#fff;font-size:16px}"
    "</style></head><body><div class='c'><h1>Desk Bot Setup</h1><p>WiFi &amp; weather settings</p>"
    "<form method='POST' action='/save'>");
  h += "<label>WiFi name</label><input name='ssid' value='" + esc(wifiSsid) + "'>";
  h += "<label>WiFi password</label><input name='pass' type='password' placeholder='(leave empty to keep current)'>";
  h += "<h2>WEATHER (OpenWeatherMap)</h2>";
  h += "<label>API key</label><input name='apikey' value='" + esc(apiKey) + "'>";
  h += "<label>City</label><input name='city' value='" + esc(city) + "'>";
  h += "<label>Country code</label><input name='country' value='" + esc(countryCode) + "'>";
  h += "<h2>TIME</h2>";
  h += "<label>Timezone (POSIX)</label><input name='tz' value='" + esc(tzString) + "'>";
  h += "<button type='submit'>Save &amp; Reboot</button></form></div></body></html>";
  server.send(200, "text/html", h);
}

void handleConfigSave() {
  String s = server.arg("ssid");
  if (s.length() == 0) {
    server.send(400, "text/plain", "SSID required");
    return;
  }
  String p = server.arg("pass");
  if (p.length() == 0) p = wifiPass;
  String ak = server.arg("apikey");
  if (ak.length() == 0) ak = apiKey;
  String c = server.arg("city");
  if (c.length() == 0) c = city;
  String cc = server.arg("country");
  if (cc.length() == 0) cc = countryCode;
  String tz = server.arg("tz");
  if (tz.length() == 0) tz = tzString;
  saveConfig(s, p, ak, c, cc, tz);
  server.send(200, "text/html",
              "<html><body style='font-family:sans-serif;background:#0b1220;color:#e6f0ff;padding:40px'>"
              "<h2 style='color:#6cb6ff'>Saved!</h2><p>Rebooting...</p></body></html>");
  delay(1500);
  ESP.restart();
}

void startConfigPortal() {
  inConfigMode = true;
  WiFi.mode(WIFI_AP);
  WiFi.softAP(CONFIG_AP_SSID, CONFIG_AP_PASS);
  server.on("/", handleConfigRoot);
  server.on("/save", HTTP_POST, handleConfigSave);
  server.begin();

  display.clearDisplay();
  display.setFont(NULL);
  display.setTextColor(BG);
  display.fillRect(0, 0, 128, 12, FG);
  display.setCursor(30, 2);
  display.print("SETUP MODE");
  display.setTextColor(FG);
  display.setCursor(0, 17);
  display.print("1. Join WiFi:");
  display.setCursor(12, 27);
  display.print(CONFIG_AP_SSID);
  display.setCursor(0, 38);
  display.print("2. Password:");
  display.setCursor(12, 48);
  display.print(CONFIG_AP_PASS);
  display.setCursor(0, 57);
  display.print("3. Open 192.168.4.1");
  display.display();
}

// Notification + Weather
void pushNotice(const String& title, const String& msg, int beeps) {
  notifTitle = cleanText(title, 40);
  notifMsg = cleanText(msg, 80);
  if (notifTitle.length() == 0) notifTitle = "Notification";
  if (notifMsg.length() == 0) notifMsg = "(text not supported)";
  notifActive = true;
  notifStart = millis();
  startBeep(beeps);
}

void handleNotify() {
  if (!server.hasArg("title") && !server.hasArg("msg")) {
    server.send(400, "text/plain", "Use /notify?title=..&msg=..");
    return;
  }
  pushNotice(server.arg("title"), server.arg("msg"), 2);
  server.send(200, "text/plain", "OK");
}

// ---------- phone dashboard ----------
// DASH_HTML lives in dashboard.h

void sendOk() {
  server.send(200, "application/json", "{\"ok\":1}");
}

void sendBad() {
  server.send(400, "application/json", "{\"ok\":0}");
}

void handleDashboard() {
  server.send_P(200, "text/html; charset=utf-8", DASH_HTML);
}

String stateJson() {
  struct tm t;
  char tb[12] = "--:--:--";
  if (getLocalTime(&t, 0)) sprintf(tb, "%02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
  String j = "{\"time\":\"" + String(tb) + "\"";
  j += ",\"mood\":" + String(currentMood);
  j += ",\"page\":" + String(currentPage);
  j += ",\"ringing\":" + String(alarmRinging ? 1 : 0);
  j += ",\"weather\":{\"ok\":" + String(weatherLoaded ? 1 : 0);
  j += ",\"temp\":" + String((int)lround(temperature));
  j += ",\"main\":\"" + weatherMain + "\"}";
  j += ",\"alarms\":[";
  for (int i = 0; i < ALARM_COUNT; i++) {
    if (i) j += ",";
    j += "{\"h\":" + String(alarms[i].h);
    j += ",\"m\":" + String(alarms[i].m);
    j += ",\"e\":" + String(alarms[i].en ? 1 : 0);
    j += ",\"d\":" + String(alarms[i].days) + "}";
  }
  j += "]";
  j += ",\"pomo\":{\"phase\":" + String(pomoPhase);
  j += ",\"started\":" + String(pomoStarted ? 1 : 0);
  j += ",\"running\":" + String(pomoRunning ? 1 : 0);
  j += ",\"left\":" + String((int)((pomoLeft() + 999) / 1000));
  j += ",\"work\":" + String(pomoWorkMin);
  j += ",\"brk\":" + String(pomoBreakMin);
  j += ",\"lng\":" + String(pomoLongMin);
  j += ",\"rounds\":" + String(pomoRounds);
  j += ",\"done\":" + String(pomoDone) + "}}";
  return j;
}

void handleApiState() {
  server.send(200, "application/json", stateJson());
}

void handleApiMood() {
  if (!server.hasArg("m")) {
    sendBad();
    return;
  }
  int m = server.arg("m").toInt();
  if (m < 0 || m > MOOD_SUSPICIOUS) {
    sendBad();
    return;
  }
  currentMood = m;
  currentPage = PAGE_EYES;
  lastSaccade = 0;
  showToast(moodNames[m]);
  sendOk();
}

void handleApiPage() {
  if (!server.hasArg("p")) {
    sendBad();
    return;
  }
  int pg = server.arg("p").toInt();
  if (pg < 0 || pg >= PAGE_COUNT) {
    sendBad();
    return;
  }
  currentPage = pg;
  notifActive = false;
  showDots();
  sendOk();
}

void handleApiAlarm() {
  if (!server.hasArg("i")) {
    sendBad();
    return;
  }
  int i = server.arg("i").toInt();
  if (i < 0 || i >= ALARM_COUNT) {
    sendBad();
    return;
  }
  alarms[i].h = constrain(server.arg("h").toInt(), 0, 23);
  alarms[i].m = constrain(server.arg("m").toInt(), 0, 59);
  alarms[i].en = (server.arg("e") == "1");
  alarms[i].days = server.arg("d").toInt() & 0x7F;
  alarms[i].lastKey = -1;
  saveAlarm(i);
  sendOk();
}

void handleApiRing() {
  int i = server.arg("i").toInt();
  if (i < 0 || i >= ALARM_COUNT) i = 0;
  startRinging(i);
  sendOk();
}

void handleApiStop() {
  bool snooze = (server.arg("snooze") == "1");
  if (alarmRinging) stopAlarm(snooze);
  else notifActive = false;
  sendOk();
}

void handleApiPomo() {
  String act = server.arg("a");
  if (act == "toggle") pomoToggle();
  else if (act == "reset") pomoReset();
  else if (act == "skip") {
    if (pomoStarted) pomoAdvance(false);
  } else {
    sendBad();
    return;
  }
  sendOk();
}

void handleApiPomoSet() {
  pomoWorkMin = constrain(server.arg("work").toInt(), 1, 120);
  pomoBreakMin = constrain(server.arg("brk").toInt(), 1, 60);
  pomoLongMin = constrain(server.arg("lng").toInt(), 1, 120);
  pomoRounds = constrain(server.arg("rounds").toInt(), 2, 6);
  if (pomoDone > pomoRounds) pomoDone = pomoRounds;
  savePomo();
  sendOk();
}
void handleApiBright() {
  if (!server.hasArg("v")) {
    sendBad();
    return;
  }
  highBrightness = (server.arg("v") == "1");
  setBrightness(display, highBrightness);
  showToast(highBrightness ? "BRIGHT" : "DIM");
  sendOk();
}

void updateMoodFromWeather() {
  int m = MOOD_NORMAL;
  if (weatherMain == "Clear") m = MOOD_HAPPY;
  else if (weatherMain == "Rain" || weatherMain == "Drizzle") m = MOOD_SAD;
  else if (weatherMain == "Thunderstorm") m = MOOD_SURPRISED;
  else if (weatherMain == "Clouds") m = MOOD_NORMAL;
  else if (temperature > 32) m = MOOD_EXCITED;
  else if (temperature < 8) m = MOOD_SLEEPY;
  currentMood = m;
}

void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  http.setTimeout(6000);
  String place = urlEncodeSpaces(city) + "," + countryCode;

  // ---- current weather ----
  String url = "http://api.openweathermap.org/data/2.5/weather?q=" + place + "&appid=" + apiKey + "&units=metric";
  http.begin(url);
  if (http.GET() == 200) {
    JSONVar o = JSON.parse(http.getString());
    if (JSON.typeof(o) != "undefined") {
      temperature = double(o["main"]["temp"]);
      feelsLike = double(o["main"]["feels_like"]);
      humidity = int(o["main"]["humidity"]);
      weatherMain = (const char*)o["weather"][0]["main"];
      weatherDesc = (const char*)o["weather"][0]["description"];
      if (weatherDesc.length()) weatherDesc.setCharAt(0, toupper(weatherDesc[0]));
      weatherLoaded = true;
      updateMoodFromWeather();
    }
  }
  http.end();

  // ---- forecast ----
  url = "http://api.openweathermap.org/data/2.5/forecast?q=" + place + "&appid=" + apiKey + "&units=metric";
  http.begin(url);
  if (http.GET() == 200) {
    JSONVar f = JSON.parse(http.getString());
    if (JSON.typeof(f) != "undefined") {
      int n = f["list"].length();
      long tzOff = (long)double(f["city"]["timezone"]);
      int idxs[3] = { 7, 15, 23 };
      bool ok = true;
      for (int i = 0; i < 3; i++) {
        int idx = idxs[i];
        if (idx >= n) {
          ok = false;
          break;
        }
        fcast[i].temp = (int)lround(double(f["list"][idx]["main"]["temp"]));
        fcast[i].main = (const char*)f["list"][idx]["weather"][0]["main"];
        time_t dt = (time_t)double(f["list"][idx]["dt"]) + tzOff;
        struct tm tmv;
        gmtime_r(&dt, &tmv);
        fcast[i].dayName = dayShort[tmv.tm_wday];
      }
      forecastLoaded = ok;
    }
  }
  http.end();
}

// ==================================================
// 6. EYES (spring physics)
// ==================================================



#define PARTICLE_COUNT 6
struct Particle {
  int x, y, speed;
};
Particle particles[PARTICLE_COUNT];

void drawEyelidMask(float x, float y, float w, float h, int mood, bool isLeft) {
  int ix = (int)x, iy = (int)y, iw = (int)w, ih = (int)h;
  if (mood == MOOD_ANGRY) {
    if (isLeft) for (int i = 0; i < 16; i++) display.drawLine(ix, iy + i, ix + iw, iy - 6 + i, BG);
    else for (int i = 0; i < 16; i++) display.drawLine(ix, iy - 6 + i, ix + iw, iy + i, BG);
  } else if (mood == MOOD_SAD) {
    if (isLeft) for (int i = 0; i < 16; i++) display.drawLine(ix, iy - 6 + i, ix + iw, iy + i, BG);
    else for (int i = 0; i < 16; i++) display.drawLine(ix, iy + i, ix + iw, iy - 6 + i, BG);
  } else if (mood == MOOD_HAPPY || mood == MOOD_LOVE || mood == MOOD_EXCITED) {
    display.fillRect(ix, iy + ih - 12, iw, 14, BG);
    display.fillCircle(ix + iw / 2, iy + ih + 6, iw / 1.3, BG);
  } else if (mood == MOOD_SLEEPY) {
    display.fillRect(ix, iy, iw, ih / 2 + 2, BG);
  } else if (mood == MOOD_SUSPICIOUS) {
    if (isLeft) display.fillRect(ix, iy, iw, ih / 2 - 2, BG);
    else display.fillRect(ix, iy + ih - 8, iw, 8, BG);
  }
}

void drawEye(Eye& e, bool isLeft) {
  int ix = (int)e.x, iy = (int)e.y, iw = (int)e.w, ih = (int)e.h;
  if (iw < 2 || ih < 1) return;
  int r = (iw < 20) ? 3 : 8;
  display.fillRoundRect(ix, iy, iw, ih, r, FG);

  int pw = iw / 2.2, ph = ih / 2.2;
  int px = ix + iw / 2 + (int)e.px - pw / 2;
  int py = iy + ih / 2 + (int)e.py - ph / 2;
  if (px < ix) px = ix;
  if (px + pw > ix + iw) px = ix + iw - pw;
  if (py < iy) py = iy;
  if (py + ph > iy + ih) py = iy + ih - ph;
  display.fillRoundRect(px, py, pw, ph, r / 2, BG);

  if (iw > 15 && ih > 15) display.fillCircle(px + pw - 4, py + 4, 2, FG);
  drawEyelidMask(e.x, e.y, e.w, e.h, currentMood, isLeft);
}

void updateEyes() {
  unsigned long now = millis();
  float breath = sinf(now / 800.0f) * 1.5f;

  // blink
  if (now > leftEye.nextBlink) {
    leftEye.blinking = rightEye.blinking = true;
    leftEye.lastBlink = now;
    leftEye.nextBlink = now + random(2000, 6000);
  }
  if (leftEye.blinking && now - leftEye.lastBlink > 120) leftEye.blinking = rightEye.blinking = false;

  // gaze
  if (!leftEye.blinking && now - lastSaccade > saccadeInterval) {
    lastSaccade = now;
    saccadeInterval = random(500, 3000);
    static const int8_t gaze[10][2] = { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { -6, -4 }, { 6, -4 }, { -6, 4 }, { 6, 4 }, { 8, 0 }, { -8, 0 } };
    int d = random(0, 10);
    float lx = gaze[d][0], ly = gaze[d][1];
    leftEye.tpx = rightEye.tpx = lx;
    leftEye.tpy = rightEye.tpy = ly;
    headX = lx * 0.3f;
    headY = ly * 0.3f;
  }

  // mood shape
  float w = 36, h = 36 + breath;
  switch (currentMood) {
    case MOOD_HAPPY:
    case MOOD_LOVE:
      w = 40;
      h = 32;
      break;
    case MOOD_EXCITED:
      w = 40;
      h = 34;
      break;
    case MOOD_SURPRISED:
      w = 30;
      h = 45;
      leftEye.tpx += random(-1, 2);
      break;
    case MOOD_SLEEPY:
      w = 38;
      h = 30;
      break;
    case MOOD_ANGRY:
      w = 34;
      h = 32;
      break;
    case MOOD_SAD:
      w = 34;
      h = 40;
      break;
  }
  leftEye.tw = rightEye.tw = w;
  leftEye.th = rightEye.th = h;
  if (currentMood == MOOD_SUSPICIOUS) {
    leftEye.th = 20;
    rightEye.th = 42;
  }
  if (leftEye.blinking) leftEye.th = rightEye.th = 2;

  // keep both eyes centered around their anchor
  leftEye.tx = 36 + headX - leftEye.tw / 2;
  rightEye.tx = 92 + headX - rightEye.tw / 2;
  leftEye.ty = 32 + headY - leftEye.th / 2;
  rightEye.ty = 32 + headY - rightEye.th / 2;

  if (currentMood == MOOD_SUSPICIOUS) {
    float scan = sinf(now * 0.005f) * 10;
    leftEye.tpx = rightEye.tpx = scan;
  }

  leftEye.update();
  rightEye.update();
}

void updateParticles() {
  if (currentMood != MOOD_SAD && currentMood != MOOD_LOVE && currentMood != MOOD_SLEEPY) return;
  for (int i = 0; i < PARTICLE_COUNT; i++) {
    Particle& p = particles[i];
    if (currentMood == MOOD_SAD) {
      p.y += p.speed;
      if (p.y > 64) {
        p.y = -6;
        p.x = random(0, 120);
      }
      drawDrop(p.x, p.y);
    } else {
      if (i % 2) p.y -= 1;  // some float up slower than others
      else p.y -= p.speed;
      if (p.y < -8) {
        p.y = 64;
        p.x = random(0, 120);
      }
      if (currentMood == MOOD_LOVE) drawHeart(p.x, p.y);
      else {
        display.setCursor(p.x, p.y);
        display.print("z");
      }
    }
  }
}

void drawEyesPage() {
  updateEyes();
  updateParticles();
  drawEye(leftEye, true);
  drawEye(rightEye, false);
}

// ==================================================
// 7. PAGES
// ==================================================
void drawClockPage() {
  struct tm t;
  display.setFont(NULL);
  display.setTextColor(FG);
  if (!getLocalTime(&t, 0)) {
    drawHeader("CLOCK");
    printCentered("Syncing time...", 64, 34);
    return;
  }
  // top row: weekday + wifi
  display.setCursor(0, 1);
  display.print(dayFull[t.tm_wday]);
  drawWifi(111, 1);
  if (anyAlarmEnabled()) drawBell(94, 0, FG);

  // big time
  int h12 = t.tm_hour % 12;
  if (h12 == 0) h12 = 12;
  char hh[4], mm[4];
  sprintf(hh, "%02d", h12);
  sprintf(mm, "%02d", t.tm_min);
  display.setFont(&FreeSansBold18pt7b);
  display.setCursor(1, 43);
  display.print(hh);
  display.setTextColor((t.tm_sec % 2 == 0) ? FG : BG);  // blinking colon
  display.print(":");
  display.setTextColor(FG);
  display.print(mm);
  int endX = display.getCursorX();

  // AM/PM + seconds
  display.setFont(NULL);
  display.setCursor(endX + 4, 20);
  display.print(t.tm_hour >= 12 ? "PM" : "AM");
  char ss[4];
  sprintf(ss, "%02d", t.tm_sec);
  display.setFont(&FreeSansBold9pt7b);
  display.setCursor(endX + 3, 43);
  display.print(ss);

  // date + mini weather
  display.setFont(NULL);
  display.setCursor(0, 50);
  display.print(t.tm_mday);
  display.print(" ");
  display.print(monthShort[t.tm_mon]);
  display.print(" ");
  display.print(t.tm_year + 1900);
  if (weatherLoaded) {
    drawWeatherIcon(84, 46, 14, weatherType(weatherMain), isNightNow());
    display.setCursor(101, 50);
    printTempSmall((int)lround(temperature));
  }

  // seconds progress bar
  display.drawFastHLine(0, 62, 128, FG);
  display.fillRect(0, 61, ((t.tm_sec + 1) * 128) / 60, 3, FG);
}

void drawWeatherPage() {
  String c = city;
  c.toUpperCase();
  if (c.length() > 14) c = c.substring(0, 13) + ".";
  drawHeader(c.c_str());

  if (WiFi.status() != WL_CONNECTED || !weatherLoaded) {
    display.setFont(NULL);
    printCentered(WiFi.status() != WL_CONNECTED ? "No WiFi" : "Loading weather...", 64, 32);
    return;
  }
  drawWeatherIcon(2, 13, 32, weatherType(weatherMain), isNightNow());

  display.setFont(&FreeSansBold18pt7b);
  display.setCursor(42, 41);
  int tInt = (int)lround(temperature);
  display.print(tInt);
  int cx = display.getCursorX();
  display.drawCircle(cx + 4, 21, 3, FG);
  display.setFont(&FreeSansBold9pt7b);
  display.setCursor(cx + 10, 41);
  display.print("C");

  display.setFont(NULL);
  String d = weatherDesc;
  if (d.length() > 21) d = d.substring(0, 21);
  display.setCursor(0, 47);
  display.print(d);

  display.setCursor(0, 56);
  display.print("Hum ");
  display.print(humidity);
  display.print("%");
  display.setCursor(66, 56);
  display.print("Feels ");
  printTempSmall((int)lround(feelsLike));
}

void drawForecastPage() {
  drawHeader("3-DAY FORECAST");
  for (int i = 0; i < 3; i++) {
    int x = 2 + i * 42;
    display.drawRoundRect(x, 12, 40, 51, 5, FG);
    display.setFont(NULL);
    if (!forecastLoaded) {
      printCentered("--", x + 20, 34);
      continue;
    }
    printCentered(fcast[i].dayName, x + 20, 16);
    drawWeatherIcon(x + 12, 27, 16, weatherType(fcast[i].main), false);

    String ts = String(fcast[i].temp);
    display.setFont(&FreeSansBold9pt7b);
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(ts, 0, 59, &x1, &y1, &w, &h);
    int startX = x + 18 - (int)(w / 2) - x1;
    display.setCursor(startX, 59);
    display.print(ts);
    display.drawCircle(startX + x1 + w + 3, 50, 2, FG);
  }
}

void drawZoneCard(int x, int w, const char* name, int offMin) {
  time_t now;
  time(&now);
  now += (time_t)offMin * 60;
  struct tm t;
  gmtime_r(&now, &t);
  int h12 = t.tm_hour % 12;
  if (h12 == 0) h12 = 12;
  char buf[8];
  sprintf(buf, "%d:%02d", h12, t.tm_min);

  display.drawRoundRect(x, 12, w, 51, 5, FG);
  display.setFont(NULL);
  printCentered(name, x + w / 2, 17);
  display.setFont(&FreeSans9pt7b);
  printCentered(buf, x + w / 2, 42);
  display.setFont(NULL);
  String sub = String(t.tm_hour >= 12 ? "PM " : "AM ") + dayShort[t.tm_wday];
  printCentered(sub, x + w / 2, 50);
}

void drawWorldPage() {
  drawHeader("WORLD CLOCK");
  drawZoneCard(1, 62, ZONE1_NAME, ZONE1_OFFSET);
  drawZoneCard(65, 62, ZONE2_NAME, ZONE2_OFFSET);
}

void drawNotification(unsigned long now) {
  unsigned long el = now - notifStart;

  // title bar
  display.fillRoundRect(0, 0, 128, 14, 3, FG);
  drawBell(4, 2, BG);
  display.setFont(NULL);
  display.setTextColor(BG);
  String ti = notifTitle;
  if (ti.length() > 18) ti = ti.substring(0, 16) + "..";
  display.setCursor(18, 3);
  display.print(ti);

  display.setTextColor(FG);
  display.drawRoundRect(0, 0, 128, 64, 3, FG);

  // message (word-wrapped)
  String lines[4];
  int n = wrapText(notifMsg, lines, 4, 20);
  for (int i = 0; i < n; i++) {
    display.setCursor(4, 19 + i * 10);
    display.print(lines[i]);
  }

  // countdown bar
  int bw = 124 - (int)((124UL * el) / NOTIF_MS);
  if (bw > 0) display.fillRect(2, 60, bw, 2, FG);

  // unfold-in animation
  if (el < 260) {
    int bh = (int)(64UL * el / 260);
    int top = (64 - bh) / 2;
    display.fillRect(0, 0, 128, top, BG);
    display.fillRect(0, 64 - top, 128, top, BG);
  }
}

void drawOverlay() {
  if (millis() > overlayUntil) return;
  display.setFont(NULL);
  if (toastText[0]) {
    int w = strlen(toastText) * 6 + 12;
    int x = (128 - w) / 2;
    display.fillRoundRect(x, 50, w, 13, 6, BG);
    display.drawRoundRect(x, 50, w, 13, 6, FG);
    display.setTextColor(FG);
    display.setCursor(x + 6, 53);
    display.print(toastText);
  } else {
    int w = PAGE_COUNT * 9 + 9;
    int x = (128 - w) / 2;
    display.fillRoundRect(x, 52, w, 11, 5, BG);
    display.drawRoundRect(x, 52, w, 11, 5, FG);
    for (int i = 0; i < PAGE_COUNT; i++) {
      int cx = x + 9 + i * 9;
      if (i == currentPage) display.fillCircle(cx, 57, 2, FG);
      else display.drawCircle(cx, 57, 1, FG);
    }
  }
}

// ==================================================
// 7b. ALARM + POMODORO
// ==================================================
bool anyAlarmEnabled() {
  for (int i = 0; i < ALARM_COUNT; i++)
    if (alarms[i].en) return true;
  return false;
}

void startRinging(int idx) {
  alarmRinging = true;
  ringIdx = idx;
  ringStart = millis();
  nextRingBeep = 0;
  notifActive = false;
}

void stopAlarm(bool snooze) {
  alarmRinging = false;
  beepSteps = 0;
  digitalWrite(BUZZER_PIN, LOW);
  if (snooze) {
    time_t n;
    time(&n);
    snoozeAt = n + SNOOZE_MIN * 60;
    snoozeIdx = ringIdx;
    showToast("SNOOZE 5 MIN");
  } else {
    snoozeAt = 0;
  }
}

void checkAlarms() {
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck < 500) return;
  lastCheck = millis();
  if (alarmRinging) return;
  struct tm t;
  if (!getLocalTime(&t, 0)) return;
  time_t nowT;
  time(&nowT);
  if (snoozeAt != 0 && nowT >= snoozeAt) {
    snoozeAt = 0;
    startRinging(snoozeIdx);
    return;
  }
  long key = (long)t.tm_yday * 1440L + t.tm_hour * 60 + t.tm_min;
  for (int i = 0; i < ALARM_COUNT; i++) {
    if (!alarms[i].en) continue;
    if (alarms[i].h != t.tm_hour || alarms[i].m != t.tm_min) continue;
    if (alarms[i].days != 0 && !(alarms[i].days & (1 << t.tm_wday))) continue;
    if (alarms[i].lastKey == key) continue;
    alarms[i].lastKey = key;
    if (alarms[i].days == 0) {  // one-shot alarm switches itself off
      alarms[i].en = false;
      saveAlarm(i);
    }
    startRinging(i);
    return;
  }
}

void updateAlarmRing(unsigned long now) {
  if (!alarmRinging) return;
  if (now - ringStart > 60000UL) {  // nobody answered -> give up after 1 minute
    stopAlarm(false);
    return;
  }
  if (beepSteps <= 0 && now >= nextRingBeep) {
    startBeep(3);
    nextRingBeep = now + 1600;
  }
}

unsigned long pomoPhaseMs(int phase) {
  if (phase == POMO_WORK) return pomoWorkMin * 60000UL;
  if (phase == POMO_BREAK) return pomoBreakMin * 60000UL;
  return pomoLongMin * 60000UL;
}

unsigned long pomoLeft() {
  if (!pomoStarted) return pomoPhaseMs(POMO_WORK);
  if (pomoRunning) {
    long d = (long)(pomoEnd - millis());
    return d > 0 ? (unsigned long)d : 0;
  }
  return pomoLeftMs;
}

void pomoAdvance(bool notify) {
  if (pomoPhase == POMO_WORK) {
    pomoDone++;
    if (pomoDone >= pomoRounds) {
      pomoPhase = POMO_LONG;
      currentMood = MOOD_EXCITED;
      if (notify) pushNotice("Pomodoro", "Great work! Long break " + String(pomoLongMin) + " min", 3);
    } else {
      pomoPhase = POMO_BREAK;
      currentMood = MOOD_HAPPY;
      if (notify) pushNotice("Pomodoro", "Focus done! Break " + String(pomoBreakMin) + " min", 3);
    }
  } else {
    if (pomoPhase == POMO_LONG) pomoDone = 0;
    pomoPhase = POMO_WORK;
    currentMood = MOOD_NORMAL;
    if (notify) pushNotice("Pomodoro", "Break over! Focus " + String(pomoWorkMin) + " min", 2);
  }
  pomoLeftMs = pomoPhaseMs(pomoPhase);
  if (pomoRunning) pomoEnd = millis() + pomoLeftMs;
}

void pomoTick() {
  if (pomoRunning && (long)(millis() - pomoEnd) >= 0) pomoAdvance(true);
}

void pomoToggle() {
  if (!pomoStarted) {
    pomoStarted = true;
    pomoPhase = POMO_WORK;
    pomoDone = 0;
    pomoLeftMs = pomoPhaseMs(POMO_WORK);
    pomoRunning = true;
    pomoEnd = millis() + pomoLeftMs;
    currentMood = MOOD_NORMAL;
  } else if (pomoRunning) {
    pomoLeftMs = pomoLeft();
    pomoRunning = false;
  } else {
    pomoRunning = true;
    pomoEnd = millis() + pomoLeftMs;
  }
}

void pomoReset() {
  pomoStarted = false;
  pomoRunning = false;
  pomoPhase = POMO_WORK;
  pomoDone = 0;
  pomoLeftMs = 0;
}

void drawPomodoroPage() {
  display.setFont(NULL);
  display.setTextColor(FG);
  String head;
  if (!pomoStarted) {
    head = "READY";
  } else {
    head = (pomoPhase == POMO_WORK) ? "FOCUS" : (pomoPhase == POMO_BREAK) ? "BREAK" : "LONG BREAK";
    if (!pomoRunning) head += " ||";
  }
  display.setCursor(0, 1);
  display.print(head);
  for (int i = 0; i < pomoRounds; i++) {
    int cx = 128 - pomoRounds * 7 + i * 7 + 3;
    if (i < pomoDone) display.fillCircle(cx, 4, 2, FG);
    else display.drawCircle(cx, 4, 2, FG);
  }
  display.drawFastHLine(0, 10, 128, FG);

  unsigned long leftMs = pomoLeft();
  int secs = (int)((leftMs + 999) / 1000);
  char buf[8];
  sprintf(buf, "%02d:%02d", secs / 60, secs % 60);
  display.setFont(&FreeSansBold18pt7b);
  printCentered(buf, 64, 41);

  display.setFont(NULL);
  unsigned long total = pomoPhaseMs(pomoPhase);
  if (leftMs > total) leftMs = total;
  int w = total ? (int)(116UL * (total - leftMs) / total) : 0;
  display.drawRoundRect(4, 47, 120, 6, 3, FG);
  if (w > 0) display.fillRoundRect(6, 49, w, 2, 1, FG);

  const char* hint = !pomoStarted ? "hold: start" : pomoRunning ? "hold:pause 2x:reset" : "hold:resume 2x:reset";
  printCentered(hint, 64, 56);
}

void drawAlarmScreen(unsigned long now) {
  bool blink = ((now / 500) % 2) == 1;
  int shake = blink ? 1 : -1;
  display.fillRoundRect(0, 0, 128, 14, 3, FG);
  drawBell(4 + shake, 2, BG);
  display.setFont(NULL);
  display.setTextColor(BG);
  display.setCursor(20, 3);
  display.print("ALARM");
  display.setTextColor(FG);

  int ah = alarms[ringIdx].h;
  int am = alarms[ringIdx].m;
  int h12 = ah % 12;
  if (h12 == 0) h12 = 12;
  char buf[8];
  sprintf(buf, "%02d:%02d", h12, am);
  display.setFont(&FreeSansBold18pt7b);
  int16_t x1, y1;
  uint16_t tw, th;
  display.getTextBounds(buf, 0, 44, &x1, &y1, &tw, &th);
  int startX = (128 - (int)tw - 22) / 2 - x1;
  display.setCursor(startX, 44);
  display.print(buf);
  display.setFont(NULL);
  display.setCursor(startX + x1 + (int)tw + 4, 28);
  display.print(ah >= 12 ? "PM" : "AM");
  if (blink) display.drawRoundRect(0, 0, 128, 64, 3, FG);
  printCentered("tap=stop hold=snooze", 64, 54);
}

// ==================================================
// 8. BUTTON
// ==================================================
void onSingleTap() {
  if (alarmRinging) {
    stopAlarm(false);
    return;
  }
  if (notifActive) {
    notifActive = false;
    return;
  }
  currentPage = (currentPage + 1) % PAGE_COUNT;
  showDots();
}

void onDoubleTap() {
  if (alarmRinging) {
    stopAlarm(false);
    return;
  }
  if (currentPage == PAGE_POMO && !notifActive) {
    pomoReset();
    showToast("RESET");
    return;
  }
  highBrightness = !highBrightness;
  setBrightness(display, highBrightness);
  showToast(highBrightness ? "BRIGHT" : "DIM");
}

void onLongPress() {
  if (alarmRinging) {
    stopAlarm(true);
    return;
  }
  if (notifActive) {
    notifActive = false;
    return;
  }
  if (currentPage == PAGE_POMO) {
    pomoToggle();
    showToast(pomoRunning ? "RUNNING" : "PAUSED");
    return;
  }
  if (currentPage == PAGE_EYES) {
    currentMood = (currentMood + 1) % (MOOD_SUSPICIOUS + 1);
    lastSaccade = 0;
    showToast(moodNames[currentMood]);
  } else {
    currentPage = PAGE_EYES;
    showDots();
  }
}

void handleButton() {
  bool pressed = (digitalRead(BUTTON_PIN) == LOW);
  unsigned long now = millis();
  if (pressed && !btnDown) {
    btnDown = true;
    btnPressAt = now;
    longDone = false;
  } else if (pressed && btnDown) {
    if (!longDone && now - btnPressAt >= LONG_PRESS_MS) {
      longDone = true;
      onLongPress();
    }
  } else if (!pressed && btnDown) {
    btnDown = false;
    if (!longDone && now - btnPressAt > 30) {
      tapCount++;
      lastRelease = now;
    }
  }
  if (tapCount > 0 && !btnDown && now - lastRelease > DOUBLE_TAP_MS) {
    if (tapCount == 1) onSingleTap();
    else onDoubleTap();
    tapCount = 0;
  }
}

// ==================================================
// 9. BOOT
// ==================================================
// returns true if the button was held -> open setup portal
bool bootScreen() {
  unsigned long start = millis();
  unsigned long heldSince = 0;
  const unsigned long TOTAL = 2200;
  while (millis() - start < TOTAL) {
    unsigned long el = millis() - start;
    if (digitalRead(BUTTON_PIN) == LOW) {
      if (heldSince == 0) heldSince = millis();
      if (millis() - heldSince >= 1500) return true;
    } else heldSince = 0;

    display.clearDisplay();
    display.setTextColor(FG);
    display.setFont(&FreeSansBold9pt7b);
    printCentered("DESK BOT", 64, 26);
    // little eyes under the title
    int open = (el / 150 % 7 == 6) ? 1 : 6;  // quick blink
    display.fillRoundRect(48, 33 + (6 - open) / 2, 12, open, 3, FG);
    display.fillRoundRect(68, 33 + (6 - open) / 2, 12, open, 3, FG);
    display.drawRoundRect(24, 46, 80, 6, 3, FG);
    display.fillRoundRect(26, 48, (int)(76UL * el / TOTAL), 2, 1, FG);
    display.setFont(NULL);
    printCentered("hold button for setup", 64, 56);
    display.display();
    delay(30);
  }
  return false;
}

bool connectWifi() {
  WiFi.mode(WIFI_STA);
  IPAddress ip(192, 168, 137, 200), gw(192, 168, 137, 1), mask(255, 255, 255, 0);
  WiFi.config(ip, gw, mask, gw);
  WiFi.begin(wifiSsid.c_str(), wifiPass.c_str());
  unsigned long st = millis();
  int f = 0;
  String shown = wifiSsid;
  if (shown.length() > 20) shown = shown.substring(0, 20);
  while (WiFi.status() != WL_CONNECTED && millis() - st < 15000) {
    display.clearDisplay();
    display.setFont(NULL);
    display.setTextColor(FG);
    printCentered("Connecting WiFi", 64, 12);
    printCentered(shown, 64, 26);
    for (int i = 0; i < 8; i++) {
      int x = 36 + i *8;
      if (i == f % 8) display.fillCircle(x, 46, 3, FG);
      else display.fillCircle(x, 46, 1, FG);
    }
    display.display();
    f++;
    delay(120);
  }
  return WiFi.status() == WL_CONNECTED;
}

void showConnected() {
  display.clearDisplay();
  display.setFont(NULL);
  display.setTextColor(FG);
  // check mark
  for (int i = 0; i < 2; i++) {
    display.drawLine(52, 14 + i, 58, 20 + i, FG);
    display.drawLine(58, 20 + i, 72, 6 + i, FG);
  }
  printCentered("CONNECTED", 64, 28);
  printCentered(WiFi.localIP().toString(), 64, 42);
  printCentered("use this IP in MacroDroid", 64, 55);
  display.display();
  delay(2500);
}

// ==================================================
// 10. MAIN
// ==================================================
void setup() {
  //Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED not found - check SDA(17)/SCL(16) wiring");
    while (true) delay(1000);
  }
  display.setTextColor(FG);

  loadConfig();

  if (bootScreen()) {
    startBeep(1);
    startConfigPortal();
    return;
  }

  leftEye.init(18, 14, 36, 36);
  rightEye.init(74, 14, 36, 36);
  for (int i = 0; i < PARTICLE_COUNT; i++) {
    particles[i].x = random(0, 120);
    particles[i].y = random(0, 64);
    particles[i].speed = random(1, 3);
  }

  if (!connectWifi()) {
    startConfigPortal();
    return;
  }
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());
  showConnected();

  if (MDNS.begin("deskbot")) MDNS.addService("http", "tcp", 80);
  server.on("/", handleDashboard);
  server.on("/notify", handleNotify);
  server.on("/api/state", handleApiState);
  server.on("/api/mood", handleApiMood);
  server.on("/api/page", handleApiPage);
  server.on("/api/alarm", handleApiAlarm);
  server.on("/api/ring", handleApiRing);
  server.on("/api/stop", handleApiStop);
  server.on("/api/pomo", handleApiPomo);
  server.on("/api/pomo/set", handleApiPomoSet);
  server.on("/api/bright", handleApiBright);
  server.begin();

  configTzTime(tzString.c_str(), "pool.ntp.org", "time.google.com");
  fetchWeather();
  lastWeatherUpdate = millis();
  startBeep(1);
  showDots();
}

void loop() {
  server.handleClient();
  if (inConfigMode) return;

  unsigned long now = millis();
  handleButton();
  updateBuzzer();
  checkAlarms();
  updateAlarmRing(now);
  pomoTick();

  unsigned long every = weatherLoaded ? 600000UL : 30000UL;
  if (now - lastWeatherUpdate > every) {
    fetchWeather();
    lastWeatherUpdate = millis();
  }

  static unsigned long lastReconnect = 0;
  if (WiFi.status() != WL_CONNECTED && now - lastReconnect > 30000) {
    lastReconnect = now;
    WiFi.reconnect();
  }

  display.clearDisplay();
  display.setFont(NULL);
  display.setTextSize(1);
  display.setTextColor(FG);

  if (alarmRinging) {
    drawAlarmScreen(now);
  } else if (notifActive) {
    drawNotification(now);
    if (now - notifStart > NOTIF_MS) notifActive = false;
  } else {
    switch (currentPage) {
      case PAGE_EYES: drawEyesPage(); break;
      case PAGE_CLOCK: drawClockPage(); break;
      case PAGE_WEATHER: drawWeatherPage(); break;
      case PAGE_FORECAST: drawForecastPage(); break;
      case PAGE_WORLD: drawWorldPage(); break;
      case PAGE_POMO: drawPomodoroPage(); break;
    }
    drawOverlay();
  }
  display.display();
  delay(10);
}
