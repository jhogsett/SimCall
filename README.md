# SimCall

A keypad-driven telephone call simulator built on an Arduino Nano, capable of producing authentic DTMF dialing tones and realistic call-progress sounds including ringback, busy, reorder, and error tones — for both North American and UK telephone standards.

---

## Background

SimCall is the spiritual successor to a Basic Stamp-2 telephone simulator built in 2001. Where the original made automatic random calls, this version puts a 4×4 DTMF keypad in your hands so you can dial any number yourself and hear what happens.

---

## Hardware

| Component | Details |
|---|---|
| Processor | Arduino Nano (clone) |
| Keypad | 4×4 DTMF keypad with I²C interface (PCF8574) |
| Tone generators | (2×) AD9833 DDS modules via SPI |
| Amplifier | PAM8403 stereo class-D amp |
| Speaker | Small 4–8 Ω speaker |
| Off-hook indicator | LED pilot light |
| Power | Li-Ion 18500 cell with micro-USB charging board |
| Enclosure | 3D printed |

### Pin Assignments

| Signal | Pin |
|---|---|
| SPI Data | D11 |
| SPI Clock | D13 |
| AD9833 #1 FSYNC | A2 |
| AD9833 #2 FSYNC | A3 |
| Off-hook LED | A1 |
| RNG seed (floating) | A0 |

---

## How It Works

Two AD9833 DDS ICs generate simultaneous sine waves whose frequencies are mixed in hardware to produce any two-tone telephone signal. One IC carries the row frequency and the other carries the column frequency for DTMF, or one carries each component of a call-progress tone.

The I²C keypad is debounced in firmware with a configurable minimum keypress time and a six-state state machine (idle → maybe-press → legit-press → held → maybe-release → legit-release).

Outcome sequences (ringback, busy, etc.) are driven by a non-blocking cooperative scheduler that advances one step per `loop()` call, so the main loop remains responsive to key input at all times.

---

## DTMF Tone Map

Standard 4×4 keypad layout:

| Key | Row (Hz) | Column (Hz) |
|---|---|---|
| 1 | 697 | 1209 |
| 2 | 697 | 1336 |
| 3 | 697 | 1477 |
| 4 | 770 | 1209 |
| 5 | 770 | 1336 |
| 6 | 770 | 1477 |
| 7 | 852 | 1209 |
| 8 | 852 | 1336 |
| 9 | 852 | 1477 |
| 0 | 941 | 1336 |
| * | 941 | 1209 |
| # | 941 | 1477 |
| A–D | 697–941 | 1633 |

---

## Usage

### Making a Call

1. Press **A** — dial tone sounds and the off-hook LED lights.
2. Dial digits:
   - **7 digits** → local call
   - **1 + 10 digits** → long-distance call
   - **011 + 10 digits** → international call (UK tones)
3. Once the expected digit count is reached, routing is simulated automatically.
4. Press **A** at any time to cancel — a cancellation tone plays and the line resets.

### Call Outcomes

| Outcome | Sound | US Cadence | UK Cadence |
|---|---|---|---|
| Ringback | 480 + 440 Hz | 2 s on / 4 s off | 400 + 450 Hz, double-ring |
| Busy | 480 + 620 Hz | 500 ms on/off | 400 Hz, 375 ms on/off |
| Reorder | 480 + 620 Hz | 250 ms on/off | 400 Hz, irregular |
| Error | 913.8 / 1428.5 / 1776.7 Hz | Three-tone sweep | 400 Hz steady |

### Outcome Probabilities

| Result | Local | Long Distance | International |
|---|---|---|---|
| Ringback | 73% | 68% | 58% |
| Busy | 25% | 23% | 25% |
| Reorder | 0.5% | 6% | 12% |
| Error | 1.5% | 3% | 5% |

> **Tip:** The last digit dialed also forces a specific outcome — ending in `1` forces busy, `2` forces reorder, `3` forces error, any other digit uses the probability table above.

---

## Planned Features

### B — Operator / Blue Box Mode
Plays the 2600 Hz SF disconnect tone for one second (simulating seizing an open trunk), then switches the numeric keys to MF operator signaling tones. `*` maps to KP1, `#` to ST.

### C — International Operator Mode
Similar to B but for international trunks. When `#` is pressed, the previously cached digits are automatically sequenced with KP1 at the start and KP2 at the end — recreating the auto-dial sequence heard at the end of *Young Lust* (Pink Floyd, *The Wall*).

### D — Free Play Mode
Each key plays a different telephone sound while held: dial tone, ringback, busy, reorder, error tones (US and UK variants), coin-deposit signals, 2600 Hz disconnect, and others.

---

## Project Structure

```
SimCall/
├── src/
│   └── main.cpp          # Application logic
├── include/
│   └── types.h           # KeypadHandler, NonBlockingSequence, and support classes
├── lib/
│   └── Randomizer/
│       └── random_seed.h # ADC-noise RNG seeder
├── doc/
│   └── hardware_description.md
└── platformio.ini
```

## Dependencies

Managed via PlatformIO:

| Library | Author |
|---|---|
| I2CKeyPad | Rob Tillaart |
| MD_AD9833 | MajicDesigns |

---

## Building & Uploading

This project uses [PlatformIO](https://platformio.org/). Open the folder in VS Code with the PlatformIO extension installed, then use the Build and Upload tasks, or run:

```sh
pio run              # build
pio run -t upload    # upload to Nano
```

