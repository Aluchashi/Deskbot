# Desk Bot

A tiny ESP32 desk companion with an animated pair of eyes, a clock, live weather, a world clock, alarms and a Pomodoro timer, all on a 128x64 OLED. You control it with one button or from a web dashboard on your phone, including voice commands.

<!-- Add a photo or GIF of your build here: -->
<!-- ![Desk Bot](docs/images/deskbot.jpg) -->

## Features

- **Animated eyes** with 9 moods (normal, happy, surprised, sleepy, angry, sad, excited, love, suspicious), blinking, gaze movement and mood particles (rain drops, hearts, "z").
- **6 pages**: Eyes, Clock, Weather, 3-day Forecast, World Clock, Pomodoro.
- **3 alarms** with weekday selection, snooze and a non-blocking buzzer.
- **Pomodoro timer** with configurable focus / break / long break / rounds.
- **Notifications**: push a message to the display with one HTTP request (works with MacroDroid, Tasker, curl, etc.).
- **Phone dashboard** served by the ESP32 itself, with mood and page control, alarm editor, Pomodoro ring and **voice commands** (Chrome, English).
- **Setup portal**: change WiFi, API key, city and timezone from your phone, no re-flashing needed.
- Settings (WiFi, alarms, Pomodoro) are saved in flash and survive reboots.

## Hardware

| Part | Notes |
|---|---|
| ESP32 dev board | Any ESP32 with WiFi |
| 0.96" OLED, SSD1306, 128x64, I2C | Address `0x3C` |
| Push button | Momentary |
| Active buzzer | 3.3 V compatible |

### Wiring

| Component | ESP32 pin |
|---|---|
| OLED SDA | GPIO 17 |
| OLED SCL | GPIO 16 |
| OLED VCC / GND | 3V3 / GND |
| Button | GPIO 22 and GND (internal pull-up is used) |
| Buzzer (+) | GPIO 21 |
| Buzzer (-) | GND |

Pins can be changed in [`Deskbot/src/config.h`](Deskbot/src/config.h).

## Getting started

### 1. Install the software

- [Arduino IDE](https://www.arduino.cc/en/software) 2.x
- ESP32 board package (Boards Manager: **esp32 by Espressif Systems**)
- Libraries (Library Manager):
  - `Adafruit BusIO`
  - `Adafruit GFX Library`
  - `Adafruit SSD1306`
  - `Arduino_JSON` (by Arduino, with the underscore)

### 2. Get a weather API key

Create a free account at [OpenWeatherMap](https://openweathermap.org/api) and copy your API key.

### 3. Add your credentials

```
cd Deskbot/src
cp secrets.example.h secrets.h      # on Windows: copy the file and rename it
```

Edit `secrets.h` with your WiFi name, password and API key. This file is git-ignored, so your secrets stay on your machine.

### 4. Check the network settings

By default Desk Bot uses a **static IP** (`192.168.137.200`), which matches the Windows Mobile Hotspot subnet. If your router uses a different subnet (for example `192.168.0.x`), edit or remove the `WiFi.config(...)` line in `connectWifi()` inside `Deskbot.ino`.

### 5. Upload

Open `Deskbot/Deskbot.ino`, select your ESP32 board and port, then click **Upload**.

> The sketch folder name must match the `.ino` name (`Deskbot`). The headers live in the `src/` subfolder next to it.

## Using Desk Bot

### Button

| Gesture | Normal | Alarm ringing |
|---|---|---|
| Single tap | Next page | Stop |
| Double tap | Toggle brightness (on the Pomodoro page: reset timer) | Stop |
| Long press | Eyes page: next mood. Other pages: back to Eyes. Pomodoro page: start / pause | Snooze (5 min) |

Hold the button during the boot screen (about 1.5 s) to open the **setup portal**.

### Setup portal

If WiFi fails, or you hold the button at boot, Desk Bot starts its own access point:

1. Join the WiFi `DeskBot` (password in `config.h`, default `12345678`).
2. Open `http://192.168.4.1`.
3. Enter WiFi, API key, city, country code and timezone (POSIX format, e.g. `BST-6`), then save. The device reboots.

### Phone dashboard

Once connected, the display shows its IP address. Open `http://<ip>` or `http://deskbot.local` in a browser.

**Voice commands** use the browser's speech recognition. Over plain `http://` Chrome may block the microphone; on Android, add the device IP to `chrome://flags/#unsafely-treat-insecure-origin-as-secure`. Examples: `happy`, `show weather`, `start`, `alarm in 10 minutes`, `alarm at 7:30 am`, `snooze`, `dim`, `say hello`.

### HTTP API

All endpoints use `GET`.

| Endpoint | Description |
|---|---|
| `/notify?title=Hi&msg=Hello` | Show a notification (ASCII text only) |
| `/api/state` | Full device state as JSON |
| `/api/mood?m=0..8` | Set mood |
| `/api/page?p=0..5` | Switch page |
| `/api/alarm?i=0..2&h=7&m=30&e=1&d=62` | Set alarm (`d` = weekday bitmask, bit0 = Sunday, 0 = once) |
| `/api/ring?i=0` | Test-ring an alarm |
| `/api/stop?snooze=0\|1` | Stop or snooze |
| `/api/pomo?a=toggle\|reset\|skip` | Pomodoro control |
| `/api/pomo/set?work=25&brk=5&lng=15&rounds=4` | Pomodoro settings |
| `/api/bright?v=0\|1` | Dim or bright display |

## Project structure

```
DeskBot/
├── README.md
├── LICENSE
├── .gitignore
├── Deskbot/                  Arduino sketch (open this folder in the IDE)
│   ├── Deskbot.ino           Main program: state, eyes, pages, alarms, Pomodoro, web handlers, setup(), loop()
│   └── src/                  Project headers
│       ├── config.h              Pins, defaults, world-clock zones, timing, page/alarm counts
│       ├── secrets.example.h     Template for WiFi and API credentials
│       ├── secrets.h             Your real credentials (git-ignored, create it yourself)
│       ├── helpers.h             Text cleaning, word wrap, URL/HTML escaping, brightness
│       ├── icons.h               Weather icons drawn with GFX primitives
│       ├── ui_widgets.h          WiFi bars, bell, heart, drop, header, centered text
│       └── dashboard.h           Phone dashboard (HTML/CSS/JS stored in flash)
└── docs/
    └── images/               Photos, screenshots, wiring diagrams
```

## Troubleshooting

| Problem | Fix |
|---|---|
| `Arduino_JSON.h` / `Adafruit_I2CDevice.h` not found | Install the libraries listed above (BusIO is required by GFX and SSD1306) |
| `secrets.h: No such file` | Copy `Deskbot/src/secrets.example.h` to `Deskbot/src/secrets.h` |
| OLED stays blank, Serial prints "OLED not found" | Check SDA/SCL wiring and the `0x3C` address |
| Stuck on "Connecting WiFi" then setup mode | Use a 2.4 GHz network and check the static IP / subnet (step 4) |
| Not shown in the Windows hotspot device list | Expected with a static IP; check with `ping 192.168.137.200` |
| Weather says "Loading..." | Check the API key (new keys can take a while to activate) and internet access |
| Settings from `secrets.h` ignored | Values saved earlier in flash win; re-save via the setup portal or erase flash on upload |

## Credits

Built with [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library), [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306), [Arduino_JSON](https://github.com/arduino-libraries/Arduino_JSON) and weather data from [OpenWeatherMap](https://openweathermap.org/).

## License

[MIT](LICENSE)
