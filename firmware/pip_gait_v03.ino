// Pip firmware v0.3 - mood layer: blinking, idle personality, 4 eye moods
// Same pins as v0.1/v0.2 (see docs/WIRING.md). Gait unchanged from v0.2.

#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define PIN_FOOT_L 25
#define PIN_FOOT_R 26
#define PIN_WING_L 32
#define PIN_WING_R 33
#define PIN_BUZZ   27
#define PIN_TOUCH  T0   // GPIO4

Adafruit_SSD1306 display(128, 64, &Wire, -1);
Servo footL, footR, wingL, wingR;

enum Mood { NEUTRAL, HAPPY, SLEEPY, CURIOUS };
Mood mood = NEUTRAL;

const int8_t GAIT[][4] = {
  { 18, -18, -12,  12}, { 0, 0, 0, 0}, {-18, 18, 12, -12}, { 0, 0, 0, 0}
};
const uint8_t GAIT_LEN = 4;
const uint16_t STEP_MS = 420;

float cur[4] = {0,0,0,0};
uint8_t step_i = 0;
unsigned long step_t = 0, last_activity = 0, last_blink = 0, next_blink = 2600;
bool asleep = false, blinking = false;
unsigned long blink_start = 0;

float ease(float t) { return (1 - cos(t * PI)) / 2; }

void setServos() {
  footL.write(90 + cur[0]); footR.write(90 + cur[1]);
  wingL.write(90 + cur[2]); wingR.write(90 + cur[3]);
}

void peep(int f, int d) { tone(PIN_BUZZ, f, d); last_activity = millis(); }

// Eye renderer: 4 moods + blink frame. 128x64, eyes at x=32/96, cy=30.
void eyes() {
  display.clearDisplay();
  bool closed = blinking;
  for (int cx : {32, 96}) {
    if (closed || mood == SLEEPY) {
      display.fillRect(cx - 14, 30, 28, 3, 1);                    // closed line
    } else {
      int r = (mood == HAPPY) ? 10 : 8;
      display.fillCircle(cx, 30, r, 1);
      if (mood == HAPPY)   display.fillRect(cx - 10, 20, 20, 8, 0);   // squint top
      if (mood == CURIOUS) display.fillCircle(cx + 3, 27, 3, 0);      // pupil offset
    }
  }
  display.display();
}

void setMood(Mood m) { mood = m; eyes(); }

void setup() {
  ESP32PWM::allocateTimer(0); ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2); ESP32PWM::allocateTimer(3);
  footL.attach(PIN_FOOT_L); footR.attach(PIN_FOOT_R);
  wingL.attach(PIN_WING_L); wingR.attach(PIN_WING_R);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  touchAttachInterrupt(PIN_TOUCH, [](){}, 40);
  eyes(); peep(1200, 80);
  last_activity = millis();
}

void loop() {
  unsigned long now = millis();

  // touch: wake + happy chirp + curious look
  if (touchRead(PIN_TOUCH) < 40) {
    if (asleep) { asleep = false; setMood(CURIOUS); }
    else setMood(HAPPY);
    peep(1600, 60); peep(2100, 60);
    last_activity = now; delay(300);
  }

  // idle nap after 5 min
  if (!asleep && now - last_activity > 300000UL) {
    asleep = true; setMood(SLEEPY); setServos();
  }
  if (asleep) { delay(200); return; }

  // blink: 120ms lid close every ~2-4s (a little random = alive)
  if (!blinking && now - last_blink > next_blink) {
    blinking = true; blink_start = now; eyes();
  }
  if (blinking && now - blink_start > 120) {
    blinking = false; last_blink = now; next_blink = 2000 + random(2200); eyes();
  }

  // eased gait (unchanged from v0.2)
  float t = min(1.0f, (now - step_t) / (float)STEP_MS);
  int8_t prev = (step_i + GAIT_LEN - 1) % GAIT_LEN;
  for (int j = 0; j < 4; j++)
    cur[j] = GAIT[prev][j] + (GAIT[step_i][j] - GAIT[prev][j]) * ease(t);
  setServos();
  if (t >= 1.0f) { step_i = (step_i + 1) % GAIT_LEN; step_t = now; }
  delay(15);
}
