# Wiring Pip (firmware v0.2+, Tier 0)

One LiPo, one charge board, one switch. No custom power electronics.

## Power path

```
LiPo 1000mAh (JST) --> TP4056 B+/B- --> slide switch --> ESP32 VIN/5V + GND
                                                    \-> servo rail (4x SG90 VCC) + 470uF cap
TP4056 OUT+ / OUT- ................. everything downstream of the switch
USB-C into TP4056 charges the LiPo; Pip can run while charging.
```

- Servos NEVER power from the ESP32 3.3V or 5V pin: four SG90s stall-pull
  ~2.5A and will brown out the board. Servo red wires go straight to the
  switched battery rail; brown wires to GND; orange to signal pins below.
- 470uF (or bigger) electrolytic across the servo rail, close to the servos.
  This is the brown-out fix - do not skip it.
- All grounds common: ESP32 GND, servo GND, TP4056 OUT-, OLED GND, buzzer -.

## Signal map (matches firmware/pip_gait_v02.ino)

| Part | Pin | ESP32 GPIO |
| --- | --- | --- |
| Foot servo L | orange | 25 |
| Foot servo R | orange | 26 |
| Wing servo L | orange | 32 |
| Wing servo R | orange | 33 |
| Piezo buzzer | + | 27 (other leg to GND) |
| Touch pad (head) | bare wire | GPIO4 (T0) |
| OLED 128x64 I2C | SDA / SCL | 21 / 22 (addr 0x3C, VCC 3.3V) |

## Notes

- The touch pad is just a bare wire end or a small copper tape square inside
  the head, wired to GPIO4. No component needed.
- OLED runs on 3.3V from the ESP32 - it is the one peripheral that does.
- Route servo wires through the shell seam gap, not over the LiPo.
- Tier A adds an INMP441 mic: BCK 14, WS 15, SD 13, VCC 3.3V, L/R to GND.
- Tier B adds a Pi Zero 2 W on the same battery rail through its own 5V
  buck (or a second TP4056+cell); the Pi talks to the ESP32 over UART
  (TX2/RX2 = GPIO 17/16, 115200 baud, logic is 3.3V on both sides).
