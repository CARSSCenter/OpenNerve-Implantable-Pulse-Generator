# Magnetic Switch Guide
OpenNerve Gen2
April 2026

---

## Overview

The Gen2 IPG includes a Hall-effect magnetic switch (`MAG_DET`, PA3) that allows non-contact user interaction through the implant body. It is the primary way to wake the device, put it to sleep, and reset BLE pairing credentials without any physical connection.

All interactions mediated through the MCU are **gesture-based**: the firmware measures how long the magnet is held and **only acts when the magnet is removed**, based on that duration. No action is taken on approach alone.

In addition to state changes mediated through the MCU there is an analog circuit that physically connects or disconnects the battery from the rest of the board using the magnetic switch, allowing power cycling in the event of a firmware issue or shutdown for storage. Turnon is mediated by capacitor C800, which triggers a connection between batteries and the rest of the circuit after holding the magnet for 10-15 seconds. Shutdown is mediated by capacitor C802, which disconnects power after 20-30 seconds of holding the magnet. Startup / shutdown occurs as soon as the capacitor charges to the necessary voltage level, and is **not** gesture-based (does not require removing the magnet).

**Important note on magnetic shutdown:** currently, there is an issue in the magnetic switch circuit where, once the device enters shutdown state, capacitor C802 must discharge fully before the IPG can be turned back on. Due to the high resistance between RC2_VC and GND this can take up to five minutes. Furthermore, attempting to turn on the IPG early by applying the magnet will re-charge C802 and further lengthen the discharge time. To avoid issues, 1) remove the magnet as soon as the device enter shutdown, and 2) always wait 5 minutes after shutdown before attempting to turn the IPG back on.

---
## Magnetic Switch Circuit

<img width="805" height="661" alt="Screenshot 2026-04-13 at 11 30 36 AM" src="https://github.com/user-attachments/assets/2b44b4a8-a1de-4c3b-b16c-e930819c53d2" />

---
## Digital State Changes (sleep-wake, BLE reset)

### How Duration Is Measured (Digital)

When a magnet approaches the device, a 1-second LPTIM timer starts counting. Each whole second that passes increments an internal counter. When the magnet is removed, the counter value (in whole seconds elapsed) is passed to the gesture logic.

**Consequence:** duration has 1-second resolution. A hold that ends at 2.9 seconds counts as 2, not 3. To reliably register a 2-second hold, remove the magnet after hearing/feeling 2 full seconds have passed — not before.

The maximum measurable duration is 255 seconds (hardware limit of the 8-bit counter). Any hold longer than ~4 minutes is treated the same as 255 seconds.

---

### Gesture Windows

There are two independent gesture windows, both configurable over BLE by an Admin-authenticated session:

| Parameter | ID | Default | Range | Purpose |
|---|---|---|---|---|
| Wakeup min time | `HP22` | 2 s | 1–60 s | Start of wake/sleep window |
| Wakeup max time | `HP23` | 6 s | 1–60 s | End of wake/sleep window |
| Reset min time | `HP24` | 9 s | 1–60 s | Start of BLE reset window |
| Reset max time | `HP25` | 13 s | 1–60 s | End of BLE reset window |

A hold duration that falls within the **wakeup window** triggers a sleep/wake toggle. A hold that falls within the **reset window** counts toward a BLE credential reset. Durations outside both windows are silently ignored.

The two windows do not overlap by default (wakeup: 2–6 s, reset: 9–13 s). They can be configured to overlap, but overlapping windows mean a single gesture would trigger both actions simultaneously — generally undesirable.

---

### Gesture 1: Sleep / Wake Toggle

**Behavior**

| Device state when magnet removed | Result |
|---|---|
| **Sleeping** (`STATE_SLEEP`) | Wakes → enters BLE advertising mode |
| **Any active state** | Stops all active operations and goes to sleep |
| DVT mode | **Ignored** — magnet has no effect |
| Shutdown state | **Ignored** — magnet has no effect |

**Active states that the magnet will put to sleep**

The sleep transition fires from any active state including:
- BLE advertising (`STATE_ACT_MODE_BLE_ACT`)
- BLE connected (`STATE_ACT_MODE_BLE_CONN`)
- Therapy session running (`STATE_ACT_MODE_THERAPY_SESSION`)
- Impedance test (`STATE_ACT_MODE_IMPED_TEST`)
- Battery test (`STATE_ACT_MODE_BATT_TEST`)
- Wireless charging — WPT High or WPT Paused (`STATE_ACT_MODE_WPT_HIGH/PAUSED`)

In all of these cases, active operations (stimulation, sensing, charging) stop cleanly before sleep entry. An `EVENT_MAGNET_DETECTION` log entry is written whenever the gesture is recognized.

**Timing with default settings**

| Hold duration | Result |
|---|---|
| < 2 s | Ignored |
| 2–6 s | Sleep ↔ wake toggle |
| 7–8 s | Ignored (gap between windows) |
| 9–13 s | Counts toward BLE reset gesture (see below) |
| > 13 s | Ignored |

---

### Gesture 2: BLE Credential Reset

This gesture allows the device to temporarily accept connections from any BLE programmer, regardless of the stored passkey or whitelist. It is used to re-pair with a new or replacement programmer if the original is unavailable.

**How it works**

1. Hold the magnet for the **reset window duration** (default 9–13 s), then remove.
2. An internal counter increments. On the **first qualifying hold**, a 10-minute countdown window opens.
3. Repeat the gesture until the counter reaches **5** within that 10-minute window.
4. On the 5th qualifying hold: the device enters **BLE default mode** for 10 minutes.

**BLE default mode**

While active, `app_func_ble_is_default()` returns true. This causes:
- The stored passkey to be replaced by the **factory default passkey** for the duration
- The **whitelist to be disabled** — any programmer can attempt authentication

BLE default mode expires after 10 minutes, or immediately if the counter window expires before 5 holds are completed.

**Reset gesture state machine**

| Event | Effect |
|---|---|
| 1st qualifying hold | Counter = 1; 10-minute window opens |
| 2nd–4th qualifying holds (within window) | Counter increments |
| 5th qualifying hold (within window) | BLE default mode active for 10 minutes |
| Window expires before 5th hold | Counter resets to 0; window closes |
| BLE default mode timer expires | Returns to normal BLE credentials |

The window and count are reset any time the 10-minute timer runs out without reaching 5.

---

### EOS / Post-Shutdown Behavior

If the device has reached End of Service (EOS) and entered sleep with `BATT_SW_EN` pulled low (batteries in the process of disconnecting), applying the magnet with a valid wakeup gesture will still wake the device. `BATT_SW_EN` is restored HIGH on wakeup, which halts the battery-disconnect process. Normal BLE advertising begins. The EOS counter remains set — the next battery test (or the next return to sleep) will re-evaluate whether EOS still applies.

---

### Summary of All Hold Durations (Default Settings)

| Duration | Action |
|---|---|
| < 2 s | Nothing |
| 2–6 s | Wake from sleep **or** go to sleep from any active state |
| 7–8 s | Nothing |
| 9–13 s | Increment BLE reset counter (must repeat 5× within 10 min) |
| > 13 s | Nothing |

---

### Relevant Source Files

| File | Role |
|---|---|
| `Core/Src/gpio.c` | Configures `MAG_DET_Pin` (PA3) as rising+falling edge EXTI |
| `Core/Src/stm32u5xx_it.c` | `EXTI3_IRQHandler` dispatches to HAL GPIO callbacks |
| `App/Bsp/Src/bsp_magnet.c` | Rising/falling edge callbacks; LPTIM-based duration measurement |
| `App/Functions/Src/app_func_state_machine.c` | `app_func_sm_magnet_lost_cb()` — sleep/wake state transitions |
| `App/Functions/Src/app_func_ble.c` | `app_func_ble_magnet_lost_cb()` — BLE reset gesture counter |
| `App/Functions/Src/app_func_parameter.c` | Default values for all four time window parameters |
