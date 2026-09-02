# Printing and assembling Pip (v0.3)

Everything here prints on an entry-level FDM printer (Ender-3 class or
smaller) without supports. Total filament ~150 g, total print time roughly
one day. Source: `cad/pip_v03.scad` - set `PART`, F6, export STL.

## Print settings (all parts)

| Setting | Value |
| --- | --- |
| Layer height | 0.2 mm |
| Walls | 3 perimeters |
| Infill | 20 % gyroid (shell), 40 % (feet, wings) |
| Material | PLA (cream body, orange feet/beak) |
| Supports | none |
| Bed adhesion | brim on the feet only |

## Parts

| PART name | Qty | Color | Orientation | Notes |
| --- | --- | --- | --- | --- |
| `shell_front` | 1 | cream | seam face down | contains face window + beak seat |
| `shell_back` | 1 | cream | seam face down | |
| `face_bezel` | 1 | black/dark | flat side down | snaps over the OLED window |
| `board_tray` | 1 | any | flat | ESP32 mounts with M2 self-tap |
| `foot_left`, `foot_right` | 1 each | orange | sole down | rocker sole; horn pocket on top |
| `wing_left`, `wing_right` | 1 each | cream | flat | horn pocket at root |
| `beak` | 1 | orange | as oriented | prints as one; hinge pin = 25 mm of 1.75 mm filament |

## Hardware beyond the BOM

- 8x M2x8 self-tapping screws (shell bosses + board tray)
- The servo horn screws that come in the SG90 bag
- 25 mm of spare 1.75 mm filament (beak hinge pin)

## Assembly order

1. Test-fit the shell halves. The seam faces should sit flush; a light pass
   with a file on the seam edge is normal.
2. Press the four servo horns into the feet and wings (spline press fit,
   then the horn screw).
3. Mount the foot servos into the lower shell pockets, horns pointing down.
   Seat the feet on the horns. Neutral (90 deg) = sole flat on the table.
4. Mount the wing servos, horns out the sides; press the wings on.
5. Screw the ESP32 tray in, low in the shell. Battery bay below it -
   keep the mass low, it is what makes the waddle stable.
6. Wire per `firmware/pip_gait_v02.ino` pin defines (25/26 feet, 32/33
   wings, 27 buzzer, GPIO4 touch, OLED on I2C 21/22).
7. Slide the OLED into the face window from inside, bezel over the front.
8. Beak: upper half screws/glue to the beak seat, lower half flaps on the
   filament hinge pin.
9. Flash firmware (Arduino IDE, ESP32 board package, SSD1306 + ESP32Servo
   libs), close the shell with the 4 seam screws.

## First power-up

Pip peeps, opens its eyes, and starts waddling in place until you pick it
up. Touch the head (GPIO4 pad) for a happy chirp. Five minutes of stillness
and it naps; touch wakes it.

## Fit troubleshooting

- Horn pocket too tight: drill 4.8 mm -> 5.0 mm, or scale feet 101 % in the
  slicer. PLA shrink varies by brand.
- Shell seam gappy: check elephant foot on the seam face first.
- Waddle tips forward: battery not low enough, or feet swapped L/R.
