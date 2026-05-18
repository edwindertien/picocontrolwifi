#pragma once

// ── Active vehicle ──────────────────────────────────────────────────────────
// Only one target in this project: the WiFi-controlled vacuum robot.
#define STOFZUIGER (1)

// ── Feature flags ───────────────────────────────────────────────────────────
#define WIFICONTROL (1)
#define BOARD_V3    (1)
#define USE_MOTOR   (1)
#define USE_OLED    (1)

// Motor speed limit (sign-magnitude PWM via Motor class)
#define LOW_SPEED     255
#define BRAKE_TIMEOUT  30   // loops at 20 Hz → ~1.5 s

// Channel layout (not used by WiFi path, kept for saveValues array size)
#define NUM_CHANNELS 16
const int saveValues[NUM_CHANNELS] = { 127, 127, 127, 127,
                                         0,   0,   0,   0,
                                         0,   0,   0,   0,
                                         0,   0,   0,   0 };

// Motor objects – defined in config.cpp
#include "Motor.h"
void configureMotors();
extern Motor motorLeft;
extern Motor motorRight;
