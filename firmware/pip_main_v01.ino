/*
 * pip_main_v01.ino - integrated Tier A sketch: gait + moods + eyes + touch
 * + keyword ears, all in one ESP32 program.
 *
 * Supersedes pip_voice_v01.ino (which was v0.3 + comments). This file is
 * the real merge: v0.3 body verbatim, I2S init from voice v0.1, and the
 * command behaviors wired to actual gait/mood hooks instead of comments.
 * pip_face_v01.ino stays as a standalone OLED bring-up; the eye renderer
 * here is the v0.3 one (circle eyes) so gait, moods, and face agree.
 *
 * UNTESTED ON HARDWARE. TFLM/microWakeWord model bytes are the only stub:
 * g_model ships after Colab training (see docs/VOICE.md). Everything else
 * compiles against stock ESP32 Arduino + ESP32Servo + Adafruit SSD1306.
 *
 * Pins (docs/WIRING.md, Tier A column): servos 25/26/32/33, buzzer 27,
 * touch T0 (GPIO4), OLED I2C 21/22, INMP441 I2S BCK 14 / WS 15 / SD 13.
 * No overlaps: I2S0 owns 13/14/15, I2C owns 21/22, LEDC owns the rest.
 */

#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <driver/i2s.h>

#define PIN_FOOT_L 25
#define PIN_FOOT_R 26
#define PIN_WING_L 32
#define PIN_WING_R 33
#define PIN_BUZZ   27
#define PIN_TOUCH  T0   // GPIO4
#define I2S_BCK 14
#define I2S_WS  15
#define I2S_SD  13

Adafruit_SSD1306 display(128, 64, &Wire, -1);
Servo footL, footR, wingL, wingR;

enum Mood { NEUTRAL, HAPPY, SLEEPY, CURIOUS };
Mood mood = NEUTRAL;

// ---- gait state (v0.3, unchanged) ----
const int8_t GAIT[][4] = {
  { 18, -18, -12,  12}, { 0, 0, 0, 0}, {-18, 18, 12, -12}, { 0, 0, 0, 0}
};
const uint8_t GAIT_LEN = 4;
const uint16_t STEP_MS = 420;
float cur[4] = {0,0,0,0};
uint8_t step_i = 0;
unsigned long step_t = 0, last_activity = 0, last_blink = 0, next_blink = 2600;
bool asleep = false, blinking = false, walking = true;
unsigned long blink_start = 0;
int8_t spin_dir = 0;          // 0 = normal gait, +/-1 = spin in place
uint16_t step_ms = STEP_MS;   // dance command shortens this

float ease(float t) { return (1 - cos(t * PI)) / 2; }

void setServos() {
  footL.write(90 + cur[0]); footR.write(90 + cur[1]);
  wingL.write(90 + cur[2]); wingR.write(90 + cur[3]);
}

void peep(int f, int d) { tone(PIN_BUZZ, f, d); last_activity = millis(); }

// ---- eye renderer (v0.3, unchanged: circle eyes, 4 moods + blink) ----
void eyes() {
  display.clearDisplay();
  bool closed = blinking;
  for (int cx : {32, 96}) {
    if (closed || mood == SLEEPY) {
      display.fillRect(cx - 14, 30, 28, 3, 1);
    } else {
      int r = (mood == HAPPY) ? 10 : 8;
      display.fillCircle(cx, 30, r, 1);
      if (mood == HAPPY)   display.fillRect(cx - 10, 20, 20, 8, 0);
      if (mood == CURIOUS) display.fillCircle(cx + 3, 27, 3, 0);
    }
  }
  display.display();
}

void setMood(Mood m) { mood = m; eyes(); }

// ---- voice: command table + behaviors wired to the gait stack ----
enum Cmd { CMD_NONE, CMD_DANCE, CMD_SPIN, CMD_SLEEP, CMD_WAKEUP, CMD_HAPPY,
           CMD_SAD, CMD_SING, CMD_LOOK, CMD_NIGHT, CMD_HELLO, CMD_STOP };

volatile Cmd pending_cmd = CMD_NONE;   // audio task (core 0) -> loop (core 1)

void on_command(Cmd c) {
  last_activity = millis();
  switch (c) {
    case CMD_DANCE:  walking = true; spin_dir = 0; step_ms = 210;
                     setMood(HAPPY); break;
    case CMD_SPIN:   walking = true; spin_dir = 1; step_ms = STEP_MS;
                     setMood(CURIOUS); break;
    case CMD_SLEEP:
    case CMD_NIGHT:  asleep = true; walking = false; setMood(SLEEPY);
                     setServos(); break;
    case CMD_WAKEUP: asleep = false; walking = true; step_ms = STEP_MS;
                     spin_dir = 0; setMood(CURIOUS); break;
    case CMD_HAPPY:  setMood(HAPPY);
                     wingL.write(60); wingR.write(120); break;   // happy flap
    case CMD_SAD:    setMood(SLEEPY); peep(600, 200); break;
    case CMD_SING:   peep(1200,90); peep(1500,90);
                     peep(1800,90); peep(2400,140); break;
    case CMD_LOOK:
    case CMD_HELLO:  setMood(CURIOUS); peep(1600,60); peep(2100,60); break;
    case CMD_STOP:   walking = false; spin_dir = 0; setMood(NEUTRAL);
                     setServos(); break;
    default: break;
  }
}

// ---- I2S mic (from voice v0.1) ----
void i2s_init() {
  i2s_config_t cfg = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4, .dma_buf_len = 256, .use_apll = false
  };
  i2s_driver_install(I2S_NUM_0, &cfg, 0, NULL);
  i2s_pin_config_t pins = {.bck_io_num=I2S_BCK, .ws_io_num=I2S_WS,
                           .data_out_num=I2S_PIN_NO_CHANGE, .data_in_num=I2S_SD};
  i2s_set_pin(I2S_NUM_0, &pins);
}

// Audio task on core 0: i2s_read -> 1s sliding window -> microWakeWord
// every 40ms -> on wake, 3s command window -> command model -> pending_cmd.
// Only the queue crossing to core 1 is real today; the inference is the stub.
void audio_task(void*) {
  int32_t buf[256]; size_t got;
  bool in_window = false; unsigned long window_end = 0;
  for (;;) {
    i2s_read(I2S_NUM_0, buf, sizeof(buf), &got, portMAX_DELAY);
    // TODO(TFLM): feature gen + inference here. Until the model ships:
    // - wake word and command classification are absent
    // - pending_cmd is only ever set by the Serial bench hook below
    if (in_window && millis() > window_end) in_window = false;
    vTaskDelay(1);
  }
}

void setup() {
  Serial.begin(115200);
  ESP32PWM::allocateTimer(0); ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2); ESP32PWM::allocateTimer(3);
  footL.attach(PIN_FOOT_L); footR.attach(PIN_FOOT_R);
  wingL.attach(PIN_WING_L); wingR.attach(PIN_WING_R);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  touchAttachInterrupt(PIN_TOUCH, [](){}, 40);
  i2s_init();
  xTaskCreatePinnedToCore(audio_task, "ears", 8192, NULL, 1, NULL, 0);
  eyes(); peep(1200, 80);
  last_activity = millis();
  Serial.println(F("pip main v0.1 up. bench: type command word (dance, spin...)"));
}

void loop() {
  unsigned long now = millis();

  // Serial bench hook: stand-in for the command model until TFLM ships.
  if (Serial.available()) {
    String w = Serial.readStringUntil('\n'); w.trim();
    if      (w == "dance")  pending_cmd = CMD_DANCE;
    else if (w == "spin")   pending_cmd = CMD_SPIN;
    else if (w == "sleep")  pending_cmd = CMD_SLEEP;
    else if (w == "wake up") pending_cmd = CMD_WAKEUP;
    else if (w == "happy")  pending_cmd = CMD_HAPPY;
    else if (w == "sing")   pending_cmd = CMD_SING;
    else if (w == "stop")   pending_cmd = CMD_STOP;
  }
  if (pending_cmd != CMD_NONE) { Cmd c = pending_cmd; pending_cmd = CMD_NONE;
                                 on_command(c); }

  if (touchRead(PIN_TOUCH) < 40) {
    if (asleep) { asleep = false; walking = true; setMood(CURIOUS); }
    else setMood(HAPPY);
    peep(1600, 60); peep(2100, 60);
    last_activity = now; delay(300);
  }

  if (!asleep && now - last_activity > 300000UL) {
    asleep = true; walking = false; setMood(SLEEPY); setServos();
  }
  if (asleep) { delay(200); return; }

  if (!blinking && now - last_blink > next_blink) {
    blinking = true; blink_start = now; eyes();
  }
  if (blinking && now - blink_start > 120) {
    blinking = false; last_blink = now; next_blink = 2000 + random(2200); eyes();
  }

  if (walking) {
    float t = min(1.0f, (now - step_t) / (float)step_ms);
    int8_t prev = (step_i + GAIT_LEN - 1) % GAIT_LEN;
    for (int j = 0; j < 4; j++)
      cur[j] = GAIT[prev][j] + (GAIT[step_i][j] - GAIT[prev][j]) * ease(t);
    if (spin_dir != 0) { cur[0] = fabs(cur[0]); cur[1] = fabs(cur[1]); }  // same-side = turn
    setServos();
    if (t >= 1.0f) { step_i = (step_i + 1) % GAIT_LEN; step_t = now; }
  }
  delay(15);
}
