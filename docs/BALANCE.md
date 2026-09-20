# Mass budget and balance (v0.5)

Why this exists: the whole gait is "roll onto one rocker sole, swing the
other foot." That only works if the center of gravity stays low and
centered. This doc adds up the mass, estimates the CG, and checks the
stability envelope. All numbers are estimates from datasheet masses and
PLA density - they get re-measured on the real build.

## Mass budget (Tier A)

| Part | Mass | Height above sole (mm) |
| --- | --- | --- |
| PLA shell + parts (~150 g filament) | 150 g | ~45 (shell walls center mid-body) |
| 4x SG90 servo | 36 g | ~20 (low pockets) |
| ESP32 devkit + tray | 12 g | ~25 |
| LiPo 1000 mAh | 22 g | ~12 (bottom bay) |
| OLED + bezel | 10 g | ~60 (face) |
| TP4056 + switch + wiring + buzzer | 10 g | ~15 |
| INMP441 mic (Tier A) | 2 g | ~55 |
| **Total** | **~242 g** | |

Weighted CG estimate: about **40 mm** above the sole, i.e. at mid-body,
slightly below the egg's geometric center. The heavy stuff (battery,
servos, board) all sits in the bottom third; the shell's 150 g is what
pulls the CG up to mid-body. Fine - what matters is the roll margin below.

## Roll stability envelope

- Feet are 30 mm wide, mounted at hip_x = 14 mm off center. In a full
  roll onto one foot, the support polygon is one sole: the CG's ground
  projection must stay within ~15 mm of that foot's center line.
- Servo swing is +/-18 deg at the hip. With sole_r = 55 mm rocker radius,
  18 deg of hip swing rolls the body by roughly 18 * (some geometry) -
  in practice the CG shifts laterally about 10-12 mm per full swing.
- That leaves ~3-5 mm of margin against tipping past the sole edge.
  Comfortable, not generous. This is why the rules below exist.

## Design rules this doc justifies

1. **Battery low and centered** (PRINTING.md step 5). Moving the 22 g
   LiPo up 30 mm raises CG ~3 mm and eats most of the roll margin.
2. **Nothing heavy in the head.** The OLED is 10 g at 60 mm - already
   the highest component. No speakers up there at Tier B; speaker goes
   in the belly.
3. **Tier B mass audit before printing.** Pi Zero 2 W (12 g) + speaker
   (~10 g) + bigger battery: keep all of it below 25 mm and the margin
   holds. Recheck this doc then.
4. **Waddle amplitude is firmware-limited, not hardware-limited.** The
   18 deg swing in GAIT already uses the safe envelope; do not raise it
   without re-checking the margin.

## Known soft spots

- The CG estimate treats the shell as a uniform shell of PLA; infill
  pattern (20 % gyroid) shifts it a little. Re-weigh printed parts.
- The margin (3-5 mm) assumes a level desk. A sloped or soft surface
  narrows it. The firmware's idle-nap already parks Pip when untouched,
  which covers the "left alone on a wobbly shelf" case.
