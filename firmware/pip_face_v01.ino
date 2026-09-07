/*
 * pip_face_v01.ino - OLED face driver for Pip (Waddl project)
 * Target: ESP32 DevKit + SSD1306 128x64 I2C OLED (addr 0x3C)
 * Libs: Adafruit SSD1306 + Adafruit GFX (Library Manager)
 *
 * UNTESTED ON HARDWARE - first bring-up sketch.
 * Renders Pip's two-eye face with the same four moods as
 * firmware/pip_gait_v03.ino and the face demo on robot.html:
 * HAPPY (arched), IDLE (round), CURIOUS (one raised), SLEEPY (half-lidded).
 * Blink layer is independent: random 3-6 s interval, 120 ms lid closure.
 *
 * Wiring: OLED SDA->GPIO21, SCL->GPIO22, VCC->3V3, GND->GND.
 * On the final board these pins share the I2C bus with nothing else,
 * so the face task owns the bus outright.
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_W 128
#define SCREEN_H 64
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, -1);

// ---- moods (keep in sync with pip_gait_v03.ino Mood enum) ----
enum Mood { IDLE, HAPPY, CURIOUS, SLEEPY };
Mood mood = IDLE;

// Eye geometry: Pip's eyes sit wide and low, big for the body size.
// Tuned to read well at arm's length on a 0.96" panel.
static const int EYE_W   = 22;
static const int EYE_H   = 26;
static const int EYE_Y   = 24;   // top of eye box
static const int EYE_L_X = 26;   // left eye left edge
static const int EYE_R_X = 80;   // right eye left edge

unsigned long nextBlinkAt = 0;
unsigned long blinkStart  = 0;
bool blinking = false;
const unsigned long BLINK_MS = 120;

// Serial command hook for bench testing: send 'i','h','c','s' to set mood.
// In the integrated build the gait stack calls setMood() directly.
void setMood(Mood m) { mood = m; }

void drawEye(int x, int lid, bool raised) {
  // lid: 0 = open .. EYE_H = fully closed
  int y = EYE_Y - (raised ? 4 : 0);
  int h = EYE_H - lid;
  if (h < 2) h = 2;
  display.fillRoundRect(x, y + (EYE_H - h) / 2, EYE_W, h, h / 2, SSD1306_WHITE);
}

void drawFace() {
  display.clearDisplay();
  int lid = 0;
  bool  raisedR = false;

  switch (mood) {
    case HAPPY:   lid = EYE_H / 3; break;            // arched look
    case CURIOUS: raisedR = true; break;             // right eye raised
    case SLEEPY:  lid = EYE_H / 2; break;            // half-lidded
    case IDLE:    default: break;
  }
  if (blinking) lid = EYE_H;                          // blink wins over mood

  drawEye(EYE_L_X, lid, false);
  drawEye(EYE_R_X, lid, raisedR);
  display.display();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("SSD1306 not found - check wiring/address"));
    while (true) delay(100);
  }
  display.clearDisplay();
  display.display();
  nextBlinkAt = millis() + 3000 + random(3000);
  Serial.println(F("pip face v0.1 up. moods: i=idle h=happy c=curious s=sleepy"));
}

void loop() {
  unsigned long now = millis();

  if (!blinking && now >= nextBlinkAt) { blinking = true; blinkStart = now; }
  if (blinking && now - blinkStart >= BLINK_MS) {
    blinking = false;
    nextBlinkAt = now + 3000 + random(3000);          // sleepy blinks more often
    if (mood == SLEEPY) nextBlinkAt = now + 1500 + random(2000);
  }

  if (Serial.available()) {
    char ch = Serial.read();
    if (ch == 'i') setMood(IDLE);
    if (ch == 'h') setMood(HAPPY);
    if (ch == 'c') setMood(CURIOUS);
    if (ch == 's') setMood(SLEEPY);
  }

  drawFace();
  delay(33);   // ~30 fps cap; cheap, and display RAM write dominates anyway
}
