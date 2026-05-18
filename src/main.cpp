// picoControl – WiFi vacuum robot (STOFZUIGER)
// Raspberry Pi Pico W, earlephilhower Arduino core
//
// Resources:
//   https://arduino-pico.readthedocs.io/en/latest/index.html

#include <Arduino.h>
#include "config.h"
#include "WIFIremote.h"
#include "PicoRelay.h"

// ── OLED (optional, enabled by USE_OLED in config.h) ─────────────────────────
#ifdef USE_OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(128, 32, &Wire);
void processScreen();
#endif

// ── Globals ───────────────────────────────────────────────────────────────────
PicoRelay  relay;
WIFIremote remote;

// Running modes
#define IDLE   0
#define ACTIVE 1

// ── setup ─────────────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);

#ifdef USE_OLED
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
#endif

    // Motors
    configureMotors();

    // Brush / blower output pins (board v3.5)
    // Wide brush   – PWM on pin 14, enable-low on pin 15
    // Corner brush – enable-low on pin 15 (shared enable)
    // Blower       – active-high on pin 10, enable-low on pin 11
    pinMode(14, OUTPUT);  analogWrite(14, 0);
    pinMode(15, OUTPUT);  digitalWrite(15, HIGH);  // disable
    pinMode(10, OUTPUT);  digitalWrite(10, LOW);   // off
    pinMode(11, OUTPUT);  digitalWrite(11, HIGH);  // disable

    // WiFi AP
    WIFIremote::Config cfg;
    cfg.ssid             = "VacuumBot";
    cfg.pass             = "12345678";
    cfg.apIP             = IPAddress(192, 168, 42, 1);
    cfg.apGateway        = IPAddress(192, 168, 42, 1);
    cfg.apMask           = IPAddress(255, 255, 255, 0);
    cfg.lockControlPage  = true;
    cfg.ownerLeaseMs     = 10000;
    cfg.motorIdleMs      = 30000;
    cfg.disconnectIdleMs = 120000;

    remote.begin(cfg);

    Serial.print("AP IP: ");   Serial.println(remote.ip());
    Serial.println("WiFi QR: WIFI:T:WPA;S:VacuumBot;P:12345678;H:false;;");
}

// ── loop ──────────────────────────────────────────────────────────────────────
void loop() {
    static bool brakeState = true;
    static unsigned long brakeTimer = 0;

    // WiFi server – must be called as often as possible
    remote.loop();

    // ── 20 Hz control loop ────────────────────────────────────────────────────
    static unsigned long looptime = 0;
    if (millis() < looptime + 49) return;
    looptime = millis();

    if (remote.ownerId() != -1) {
        // Owner connected: apply commands
        brakeTimer = BRAKE_TIMEOUT;

        motorLeft.setSpeed (remote.left(),  brakeState);
        motorRight.setSpeed(remote.right(), brakeState);

        // Wide brush (PWM on pin 14)
        analogWrite(14, remote.wideBrush() ? 127 : 0);

        // Corner brush (active-low enable on pin 15)
        digitalWrite(15, remote.cornerBrush() ? LOW : HIGH);

        // Blower (active-high on pin 10, enable-low on 11)
        digitalWrite(10, remote.blower() ? HIGH : LOW);
        digitalWrite(11, remote.blower() ? LOW  : HIGH);

    } else {
        // No owner: safe stop
        brakeTimer = BRAKE_TIMEOUT;
        motorLeft.setSpeed (0, brakeState);
        motorRight.setSpeed(0, brakeState);
        analogWrite(14, 0);
        digitalWrite(15, HIGH);
        digitalWrite(10, LOW);
        digitalWrite(11, HIGH);
    }

    // Brake timer
    if (brakeTimer > 0) { brakeTimer--; brakeState = false; }
    if (brakeTimer == 0) brakeState = true;

    // LED heartbeat
    digitalWrite(LED_BUILTIN, (millis() / 500) & 1);

// ── OLED display ──────────────────────────────────────────────────────────────
#ifdef USE_OLED
    static unsigned long screenTimer = 0;
    if (millis() > screenTimer + 99) {
        screenTimer = millis();
        processScreen();
    }
#endif
}

// ── OLED rendering ────────────────────────────────────────────────────────────
#ifdef USE_OLED
void processScreen() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Line 0: AP IP
    display.setCursor(0, 0);
    display.print(remote.ip());

    // Line 1: owner IP or "no connection"
    display.setCursor(0, 8);
    if (remote.ownerId() == -1) {
        display.print("no connection");
    } else {
        IPAddress ip = remote.ownerIP();
        display.print(ip[0]); display.print('.');
        display.print(ip[1]); display.print('.');
        display.print(ip[2]); display.print('.');
        display.print(ip[3]);
    }

    // Line 2: in-use indicator
    display.setCursor(0, 16);
    if (remote.inUse()) display.print("getting data");

    // Line 3: motor bars
    int16_t l = remote.left(),  r = remote.right();
    // map -255..255 to 0..32 bar height (centre = 16)
    int barL = map(constrain(l, -255, 255), -255, 255, 0, 32);
    int barR = map(constrain(r, -255, 255), -255, 255, 0, 32);
    display.fillRect(110, 32 - barL, 5, barL, SSD1306_WHITE);
    display.fillRect(116, 32 - barR, 5, barR, SSD1306_WHITE);

    // Actuator dots
    if (remote.wideBrush())   display.fillRect(0, 24, 4, 4, SSD1306_WHITE);
    if (remote.cornerBrush()) display.fillRect(6, 24, 4, 4, SSD1306_WHITE);
    if (remote.blower())      display.fillRect(12, 24, 4, 4, SSD1306_WHITE);

    display.display();
}
#endif
