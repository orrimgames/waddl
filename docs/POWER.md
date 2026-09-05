# Power budget (Tier 0, engineering estimate)

Goal: honest runtime numbers for the 1000mAh LiPo claim on the BOM.

## Draw estimates

| Consumer | Typical | Peak | Notes |
| --- | --- | --- | --- |
| 4x SG90 servos | ~250 mA avg while waddling | ~2.5 A stall (all 4) | waddle duty ~50%, two servos active at once |
| ESP32 (WROOM-32) | ~80 mA | ~240 mA (Wi-Fi burst) | radios idle in Tier 0; ~40 mA with modem sleep |
| OLED 128x64 | ~15 mA | ~25 mA | eyes on |
| Piezo + touch | <5 mA | - | negligible |
| **Total, waddling** | **~350 mA** | | |
| **Total, idle/nap** | **~20-40 mA** | | ESP32 light sleep + OLED eyes dimmed/closed |

## Runtime math (1000 mAh cell, ~80% usable)

- Continuous waddling: 800 mAh / 350 mA = **~2.3 hours**
- Mixed pet use (waddle 20%, idle 80%): 800 / (0.2*350 + 0.8*30) = **~8.5 hours**
- Pure nap: 800 / 30 = **~26 hours**

The BOM line "about 2-3 hours of waddling per charge" holds. As a desk pet
that mostly sits, blinks, and reacts, a day per charge is realistic.

## Tier B add-on (Pi Zero 2 W)

- Pi Zero 2 W idle ~100 mA, STT/TTS burst ~350-450 mA.
- Wake-word-gated pipeline (Pi sleeps until the ESP32's mic gate fires)
  keeps Tier B mixed-use around ~3-4 hours on the same cell. The +$4 bigger
  battery (2000mAh) doubles that and is why it is in the Tier B BOM.

## Rules that protect the battery

1. Servos on their own rail with a 470uF cap (see WIRING.md).
2. Firmware naps after 5 min idle (v0.2+).
3. Never let the TP4056 see over 5.2V in; charge via USB-C only.
