/*
 * pip_gait_v01.ino - Pip Tier 0 firmware (ESP32)
 * Waddle gait + OLED eye face + piezo voice. No cloud, no radio needed.
 *
 * Wiring:
 *   Feet servos:  GPIO 18 (left), 19 (right)
 *   Wing servos:  GPIO 21 (left), 22 (right)
 *   OLED 0.96" I2C: SDA 23, SCL 25 (SSD1306, 128x64)
 *   Piezo buzzer: GPIO 26
 *   Touch sensor: T0 (GPIO 4) - head pat
 *
 * Libs (Arduino Library Manager): ESP32Servo, Adafruit SSD1306, Adafruit GFX
 */

#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define PIN_FOOT_L 18
#define PIN_FOOT_R 19
#define PIN_WING_L 21
#define PIN_WING_R 22
#define PIN_BUZZER 26
#define PIN_TOUCH  T0

Adafruit_SSD1306 display(128, 64, &Wire, -1);
Servo footL, footR, wingL, wingR;

// Servo centers and travel. Tune on the desk - every SG90 is a little different.
const int FOOT_CENTER = 90, FOOT_LIFT = 28;
const int WING_REST = 90, WING_FLAP = 45;

enum Mood { IDLE, HAPPY, SLEEPY, WIDE };
Mood mood = IDLE;

unsigned long lastBlink = 0, nextBlinkIn = 2500;
unsigned long lastStep = 0;
bool stepping = false;
int stepSide = 0;

void setup() {
  footL.attach(PIN_FOOT_L); footR.attach(PIN_FOOT_R);
  wingL.attach(PIN_WING_L); wingR.attach(PIN_WING_R);
  pinMode(PIN_BUZZER, OUTPUT);
  Wire.begin(23, 25);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  centerAll();
  peep(2);  // hello
}

void centerAll() {
  footL.write(FOOT_CENTER); footR.write(FOOT_CENTER);
  wingL.write(WING_REST);   wingR.write(WING_REST);
}

// ---- voice ----
void peep(int n) {
  for (int i = 0; i < n; i++) {
    tone(PIN_BUZZER, 2400 + random(-300, 400), 90);
    delay(140);
  }
}
void quack() {  // tiny proud quack
  tone(PIN_BUZZER, 900, 120); delay(150);
  tone(PIN_BUZZER, 700, 160); delay(200);
  noTone(PIN_BUZZER);
}

// ---- gait: the waddle ----
// Rock onto one foot, swing the other forward. Adorable beats fast.
void waddleStep() {
  if (stepSide == 0) {
    footL.write(FOOT_CENTER + FOOT_LIFT);   // plant left, lift right
    footR.write(FOOT_CENTER - FOOT_LIFT/2);
  } else {
    footR.write(FOOT_CENTER - FOOT_LIFT);   // plant right, lift left
    footL.write(FOOT_CENTER + FOOT_LIFT/2);
  }
  stepSide = 1 - stepSide;
  lastStep = millis();
}

void happyFlap() {
  for (int i = 0; i < 6; i++) {
    wingL.write(WING_REST + WING_FLAP); wingR.write(WING_REST - WING_FLAP);
    delay(120);
    wingL.write(WING_REST); wingR.write(WING_REST);
    delay(120);
  }
}

// ---- face ----
void drawEyes(bool blink, int lookX, Mood m) {
  display.clearDisplay();
  int h = blink ? 4 : (m == WIDE ? 26 : (m == SLEEPY ? 10 : 20));
  int w = (m == WIDE) ? 20 : 16;
  int y = 32 - h/2;
  if (m == HAPPY) {  // happy eyes: inverted-U arcs
    display.fillCircle(40 + lookX, 36, 12, SSD1306_WHITE);
    display.fillCircle(40 + lookX, 40, 12, SSD1306_BLACK);
    display.fillCircle(88 + lookX, 36, 12, SSD1306_WHITE);
    display.fillCircle(88 + lookX, 40, 12, SSD1306_BLACK);
  } else {
    display.fillRoundRect(40 - w/2 + lookX, y, w, h, 6, SSD1306_WHITE);
    display.fillRoundRect(88 - w/2 + lookX, y, w, h, 6, SSD1306_WHITE);
  }
  display.display();
}

void loop() {
  unsigned long now = millis();

  // idle waddle: a step every 900ms
  if (stepping && now - lastStep > 900) waddleStep();

  // blinking
  if (now - lastBlink > nextBlinkIn) {
    drawEyes(true, 0, mood);
    delay(140);
    lastBlink = now;
    nextBlinkIn = 2000 + random(0, 3500);
  }
  drawEyes(false, (int)(8 * sin(now / 1400.0)), mood);  // slow look-around

  // head pat = happy
  if (touchRead(PIN_TOUCH) < 40) {
    mood = HAPPY;
    happyFlap();
    peep(3);
    mood = IDLE;
  }
}
