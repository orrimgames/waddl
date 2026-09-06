// Pip Tier A voice scaffold v0.1 - "keyword ears" on top of gait v0.3.
// UNTESTED - no hardware yet. Structure and integration points are real;
// the microWakeWord model bytes (g_model) ship after training on Colab.
// Pins: see docs/WIRING.md Tier A. Design: docs/VOICE.md.

#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <driver/i2s.h>
// #include <tensorflow/lite/micro/...>  // TFLM + microWakeWord feature gen

#define PIN_FOOT_L 25
#define PIN_FOOT_R 26
#define PIN_WING_L 32
#define PIN_WING_R 33
#define PIN_BUZZ   27
#define PIN_TOUCH  T0
// INMP441
#define I2S_BCK 14
#define I2S_WS  15
#define I2S_SD  13

// ---- command table: id -> behavior hook ----
enum Cmd { CMD_NONE, CMD_DANCE, CMD_SPIN, CMD_SLEEP, CMD_WAKEUP, CMD_HAPPY,
           CMD_SAD, CMD_SING, CMD_LOOK, CMD_NIGHT, CMD_HELLO, CMD_STOP };
const char* CMD_NAME[] = {"", "dance", "spin", "sleep", "wake up", "happy",
                          "sad", "sing", "look at me", "good night", "hello", "stop"};

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

void on_wake() {
  // hey Pip detected: curious face, acknowledge chirp, open 3s command window
  tone(PIN_BUZZ, 1800, 60);
  // setMood(CURIOUS);  // v0.3 hook
}

void on_command(Cmd c) {
  switch (c) {
    case CMD_DANCE:  /* gait on, wings wide */ break;
    case CMD_SPIN:   /* spin in place */ break;
    case CMD_SLEEP:  /* nap: SLEEPY mood, servos neutral */ break;
    case CMD_HAPPY:  /* HAPPY mood + happy flap */ break;
    case CMD_SING:   tone(PIN_BUZZ,1200,90); tone(PIN_BUZZ,1500,90);
                     tone(PIN_BUZZ,1800,90); tone(PIN_BUZZ,2400,140); break;
    case CMD_STOP:   /* all motion off */ break;
    default: break;  // HELLO/LOOK/NIGHT/WAKEUP/SAD map to moods + chirps
  }
}

void setup() {
  // ... v0.3 servo/OLED/touch setup unchanged ...
  i2s_init();
  // TODO: TFLM setup: load g_model, allocate ~60KB arena, start audio task
  // on core 0 (gait/display stay on core 1).
}

void loop() {
  // ... v0.3 gait + mood loop unchanged ...
  // Audio task (core 0, xTaskCreatePinnedToCore) does:
  //   i2s_read -> sliding 1s window -> microWakeWord infer every 40ms
  //   wake hit -> on_wake() -> 3s window -> command model -> on_command(c)
  // Servo-noise gate: skip inference while a gait step is mid-swing if
  // false triggers show up in testing (see docs/VOICE.md).
}
