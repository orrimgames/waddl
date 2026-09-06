# Tier A voice: "keyword ears" design (target firmware: pip_voice_v01)

Goal: Pip hears "hey Pip" and a dozen commands, on-device, on the ESP32 -
no cloud, no account, ~$3 of added parts. This is the design; the code
scaffold is firmware/pip_voice_v01.ino. Untested until hardware lands.

## Pipeline

INMP441 I2S mic (16 kHz, mono) -> ring buffer -> microWakeWord streaming
inference (TensorFlow Lite Micro, int8) -> on detect: command window (3 s)
-> keyword match against the command set -> action + canned reply (piezo
jingle or OLED text).

microWakeWord (ESPHome's engine) runs a ~40 KB model on the ESP32 at
~20% CPU with 16 kHz audio. Alternative: ESP-SR's WakeNet (heavier,
needs the -S3 for the good models; we stay on plain ESP32, so
microWakeWord is the pick).

## Command set v1 (12)

hey Pip (wake) - dance - spin - sleep - wake up - happy - sad - sing -
look at me - good night - hello - stop

Each maps to a behavior already in v0.3 (gait toggle, spin, mood change,
nap, peep melody). "sing" plays the 4-note peep arpeggio. Replies are
canned: chirp patterns + OLED text/mood. No STT - keyword spotting only.
That is what keeps it $3 and instant.

## Pins (matches WIRING.md Tier A)

INMP441: BCK=14, WS=15, SD=13, VCC=3.3V, L/R=GND.

## Models

Wake model: train "hey Pip" with the microWakeWord notebook (free Colab),
export int8 TFLM, embed as a C array. Command models: one multi-label
model for the 11 commands is smaller than 11 separate ones.

## Risks

- False wakes from TV/music: threshold tune + require wake before commands.
- ESP32 RAM: TFLM arena ~60 KB is fine alongside the gait/face stack.
- Noise while waddling: servos are loud; gate the mic during gait steps if
  false triggers show up in testing.
