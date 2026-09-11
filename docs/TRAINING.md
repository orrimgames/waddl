# Training "hey Pip" (Tier A wake word)

The only stub left in `firmware/pip_main_v01.ino` is the model bytes. This
is the recipe that produces them, start to finish, on Google Colab's free
tier. Zero cost. Plan an afternoon; most of it is the GPU churning.

We use [microWakeWord](https://github.com/kahrendt/microWakeWord) (Open Home
Foundation, Apache-2.0). It trains a small streaming keyword spotter that
exports to TensorFlow Lite Micro - exactly what the sketch's audio task
loads. Detection pipeline, per upstream: 16 kHz mono audio -> micro_speech
preprocessor (40 features every 10 ms, with noise suppression and AGC) ->
quantized streaming MixConv model -> probability per 30 ms window.

## Honest warning (from upstream)

microWakeWord's own README is blunt: training a *usable* model is hard and
takes experimentation with hyperparameters and sample counts. Expect two or
three training runs before the false-accept rate is livable. The recipe
below is the shortest path to a first model, not a guarantee.

## Steps

1. Open a Colab notebook (Runtime -> change runtime type -> GPU).
2. `!git clone https://github.com/kahrendt/microWakeWord` and open
   `notebooks/basic_training_notebook.ipynb` from the clone. That notebook
   is the upstream starting point; everything below is the Pip-specific
   delta.
3. Set the target phrase to `hey pip`. Two syllables plus a plosive is
   friendly territory for keyword spotters - short but distinct.
4. Generate samples with
   [piper-sample-generator](https://github.com/rhasspy/piper-sample-generator)
   (the notebook wires this in). More voices and more augmentations beat
   more epochs: aim for 25k+ generated positive samples across many Piper
   voices, speeds, and pitches. A kid-toy wake word gets said by kids -
   include higher-pitch voices.
5. Negatives: the pregenerated spectrogram negative sets on Hugging Face
   (`kahrendt/microwakeword` dataset) - dinner-party chatter, ambient
   noise, music. These drive the false-accepts-per-hour metric, which is
   the number that decides whether Pip's ears are annoying or magic.
6. Train. The notebook picks best weights in two passes: first minimize
   false accepts per hour on ambient noise, then maximize accuracy. Keep
   that order - a wake word that cries wolf is worse than one that
   occasionally needs a second "hey Pip".
7. Export: the notebook emits a quantized streaming `.tflite`.
8. Convert to a C array for the firmware:
   `xxd -i hey_pip.tflite > g_model.h` (or the repo's conversion script if
   it ships one). The array is typically 40-80 KB quantized.
9. Drop `g_model.h` next to `pip_main_v01.ino`, wire it into the audio
   task's TFLM arena (~60 KB budget, see `docs/VOICE.md`), and flash.

## Bench acceptance test

Before the model earns a spot in a printed shell:

- Say "hey Pip" 20 times, varied distance/voice: want >= 18 detections.
- Run one hour of podcast audio at normal room volume: want <= 1 false
  accept. If it fails, raise `negative_class_weight` or add ambient
  negatives and retrain (step 6).
- Servo-noise check: waddle running, say the wake word from 1 m. If the
  gait drowns it, the servo-noise gate in `pip_main_v01.ino` (skip
  inference mid-swing) is the fallback - also in `docs/VOICE.md`.

## Command words

"dance", "spin", "sleep" and friends follow this same recipe, one model
per word or a small multi-class head, inside the 3-second window the wake
word opens. Train them only after "hey Pip" passes the acceptance test -
the wake word is the hard one.

## Cost

$0. Colab free tier, open-source models, permissive licenses throughout
(Apache-2.0 for microWakeWord and the model repo). The resulting weights
are ours to ship in firmware.
