// Pip gait firmware v0.2 - eased waddle + sleep, ESP32 + 4x SG90 + SSD1306 + piezo + touch
// v0.2 changes: cosine easing between gait keyframes (no more servo slam),
// deep-ish light sleep after 5 min idle, wake on touch. Same pins as v0.1.

#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define PIN_FOOT_L 25
#define PIN_FOOT_R 26
#define PIN_WING_L 32
#define PIN_WING_R 33
#define PIN_BUZZ   27
#define PIN_TOUCH  T0   // GPIO4 touch pad

Adafruit_SSD1306 display(128, 64, &Wire, -1);
Servo footL, footR, wingL, wingR;

// Gait keyframes: [footL, footR, wingL, wingR] degrees from neutral.
// Feet rock the body onto each rocker sole; wings counter-flap.
const int8_t GAIT[][4] = {
  { 18, -18, -12,  12},   // roll left
  {  0,   0,   0,   0},   // settle
  {-18,  18,  12, -12},   // roll right
  {  0,   0,   0,   0}
};
const uint8_t GAIT_LEN = 4;
const uint16_t STEP_MS = 420;      // per keyframe, eased

float cur[4] = {0, 0, 0, 0};       // current eased positions
uint8_t step_i = 0;
unsigned long step_t = 0, last_activity = 0;
bool asleep = false;

float ease(float t) { return (1 - cos(t * PI)) / 2; }   // cosine ease in-out

void setServos() {
  footL.write(90 + cur[0]); footR.write(90 + cur[1]);
  wingL.write(90 + cur[2]); wingR.write(90 + cur[3]);
}

void peep(int f, int d) { tone(PIN_BUZZ, f, d); last_activity = millis(); }

void eyes(uint8_t mood) {   // 0 neutral, 1 happy, 2 sleepy
  display.clearDisplay();
  if (mood == 2) { display.fillRect(18, 30, 28, 3, 1); display.fillRect(82, 30, 28, 3, 1); }
  else {
    display.fillCircle(32, 30, mood ? 10 : 8, 1);
    display.fillCircle(96, 30, mood ? 10 : 8, 1);
    if (mood) { display.fillRect(22, 24, 20, 8, 0); display.fillRect(86, 24, 20, 8, 0); }
  }
  display.display();
}

void setup() {
  ESP32PWM::allocateTimer(0); ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2); ESP32PWM::allocateTimer(3);
  footL.attach(PIN_FOOT_L); footR.attach(PIN_FOOT_R);
  wingL.attach(PIN_WING_L); wingR.attach(PIN_WING_R);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  touchAttachInterrupt(PIN_TOUCH, [](){}, 40);   // wake source
  eyes(0); peep(1200, 80);
  last_activity = millis();
}

void loop() {
  if (touchRead(PIN_TOUCH) < 40) {          // touched: wake + react
    if (asleep) { asleep = false; eyes(1); }
    peep(1600, 60); peep(2100, 60);
    last_activity = millis(); delay(300);
  }
  if (!asleep && millis() - last_activity > 300000UL) {   // 5 min idle: nap
    asleep = true; eyes(2); setServos();
  }
  if (asleep) { delay(200); return; }

  unsigned long now = millis();
  float t = min(1.0f, (now - step_t) / (float)STEP_MS);
  int8_t prev = (step_i + GAIT_LEN - 1) % GAIT_LEN;
  for (int j = 0; j < 4; j++)
    cur[j] = GAIT[prev][j] + (GAIT[step_i][j] - GAIT[prev][j]) * ease(t);
  setServos();
  if (t >= 1.0f) { step_i = (step_i + 1) % GAIT_LEN; step_t = now; }
  delay(15);
}
