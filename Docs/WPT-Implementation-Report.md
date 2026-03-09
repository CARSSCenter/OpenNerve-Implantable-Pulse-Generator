# WPT Wireless Charging — Firmware Implementation Report

**Target hardware:** Gen2 IPG PCBA
**MCU:** STM32U585
**Branch:** `feature/wireless-charging`
**Firmware path:** `Gen2 PCBA/FW-MCU-H2/FW-NIH-MCU-H2/`

---

## 1. Overview

The Gen2 IPG hardware is fully wired for wireless power transfer (WPT) charging. Prior to this implementation, all charging-related GPIO outputs were initialized low (chargers disabled), and the digital status signals were passively read into BLE advertisements without any firmware control. The device could not autonomously charge.

This implementation adds a complete autonomous charging state machine. When a WPT charging coil is brought near the device, the firmware detects the rectified voltage presence, transitions into a dedicated charging mode, controls the two battery charger ICs, monitors temperature and battery voltage, and advertises all status information over BLE — continuously, for the duration of the charging session.

---

## 2. Hardware Overview

Understanding the charging hardware is essential context for the firmware.

### 2.1 Signal Path

```
Wireless coil → rectifier → VRECT rail
                                │
                         VRECT_DETn (PC7) ──── "Coil present" indicator (active low)
                         VRECT_OVPn (PC6) ──── Rectifier over-voltage flag (active low)
                         VRECT_MON_EN (PC10) ── Enable VRECT analog monitor
                         VRECT_MON (PC4) ──── Analog monitor output (ADC)
                                │
                         LTC3130 buck-boost converter
                         VCHG_DISABLE (PC8) ── Disable converter when HIGH
                         VCHG_PGOOD (PC9) ──── Converter output good flag
                                │
                    ┌──────────┴──────────┐
               Charger IC 1          Charger IC 2
               CHG1_EN (PE10)        CHG2_EN (PE11)    ← Enable
               CHG1_STATUS (PE14)    CHG2_STATUS (PE15) ← nCHRG open-drain (LOW=charging, HIGH-Z=done)
               CHG1_OVP_ERRn (PE12)  CHG2_OVP_ERRn (PE13) ← OVP error (active low)
                    │                        │
               Battery A               Battery B
               BATT_MON1 (PA0)         BATT_MON2 (PA1) ← Voltage monitor (÷2 divider)
```

### 2.2 Charge Rate Control

`CHG_RATE1` (PE8) and `CHG_RATE2` (PE9) control the charge current. The firmware sets:
- `CHG_RATE1 = LOW`
- `CHG_RATE2 = HIGH`

This combination selects **50 mA** charge rate. Other combinations are available but not used in production WPT mode.

### 2.2a Charger IC Autonomous Operation (LTC4065)

The charger IC is the **LTC4065**, a standalone linear Li-Ion charger. An important design principle governs how the firmware interacts with it: **`CHGx_EN` is held HIGH for the entire coil-present session and the IC manages its own charge cycle autonomously.** The firmware never toggles `CHGx_EN` in response to charge status.

The LTC4065's `nCHRG` pin (connected directly to `CHGx_STATUS` with no inverting circuit) is an open-drain output with three states:

| nCHRG state | CHGx_STATUS GPIO reads | Meaning |
|---|---|---|
| Pulled LOW by internal MOSFET | `GPIO_PIN_RESET` (0) | Battery is actively charging |
| HIGH-Z (floating, pulled to VCC by STM32 pull-up) | `GPIO_PIN_SET` (1) | Charge complete — current dropped to C/10 termination |
| Pulsing at 2 Hz | Alternating 0/1 at 1 s sample rate | **Defective battery** — voltage remained below 2.9 V for ¼ of charge time |

After reaching C/10 termination, the IC enters standby and monitors the battery voltage internally. When the battery self-discharges sufficiently, the IC restarts the charge cycle automatically with no firmware interaction. `nCHRG` will go LOW again at that point.

The firmware reads `CHGx_STATUS` every second purely for **telemetry** — it is broadcast in the BLE advertisement. It does not gate any state transition.

The only conditions that cause the firmware to intervene (pull `CHGx_EN` LOW and assert `VCHG_DISABLE`) are external protection faults that the IC cannot self-manage: **over-temperature** and **VRECT overvoltage**.

### 2.3 Thermistor Circuit

Three ADC channels work together to measure temperature:

| Signal | GPIO | Role |
|--------|------|------|
| `THERM_REF` | PC2 | Reference voltage injected into the thermistor divider |
| `THERM_OUT` | PC1 | Voltage at the thermistor/sense-resistor junction |
| `THERM_OFST` | PC0 | Offset correction voltage |

The thermistor is a **104AP-2 NTC (10 kΩ at 25°C)**. A **49.9 kΩ** sense resistor in series allows current to be inferred from the voltage drop.

### 2.4 Battery Voltage Monitoring

`BATT_MON_EN` (PA2) enables the battery monitor circuit. Each battery voltage is measured through a 2× voltage divider:

- `BATT_MON1` (PA0) → Battery A ÷ 2
- `BATT_MON2` (PA1) → Battery B ÷ 2

`app_func_meas_batt_mon_meas()` applies the `BSP_BATT_FACTOR = 2` scale and returns voltages in millivolts.

---

## 3. Files Changed

### 3.1 New Files

| File | Purpose |
|------|---------|
| `App/Inc/app_mode_wpt.h` | Public interface: constants, function declarations |
| `App/Src/app_mode_wpt.c` | Full charging state machine implementation |

### 3.2 Modified Files

| File | Summary of Changes |
|------|--------------------|
| `Core/Src/gpio.c` | VCHG_DISABLE defaults HIGH at boot; VRECT_DETn reconfigured as EXTI interrupt |
| `Core/Src/stm32u5xx_it.c` | Added `EXTI7_IRQHandler` for VRECT_DETn (PC7) |
| `App/Functions/Inc/app_func_state_machine.h` | Two new state constants; `app_func_sm_vrect_coil_cb()` declaration |
| `App/Functions/Src/app_func_state_machine.c` | New `app_func_sm_vrect_coil_cb()`; WPT guard in `app_func_sm_confirmation_timer_cb()` |
| `App/Src/app.c` | WPT cases in main switch; `app_mode_wpt_timer_cb()` in `HAL_IncTick` |
| `App/Src/app_mode_ble_active.c` | VRECT_DETn poll in main loop; WPT status bits 14–15 in MSD |
| `App/Bsp/Src/bsp_magnet.c` | VRECT_DETn coil detection in existing EXTI callbacks |
| `App/Config/app_config.h` | `#include "app_mode_wpt.h"` added |

---

## 4. Application State Machine

### 4.1 New States

```c
// App/Functions/Inc/app_func_state_machine.h
#define STATE_ACT_MODE_WPT_HIGH    0x0209U   // Coil present, charging active
#define STATE_ACT_MODE_WPT_PAUSED  0x020AU   // Coil present, charging paused
```

These slot into the existing state hierarchy alongside the other `STATE_ACT_MODE_*` values (0x0201–0x0208).

### 4.2 Complete State Map

```
STATE_SLEEP (0x0100)
    │  ▲
    │  │ magnet
    ▼  │
STATE_ACT (0x0200)
    │
    ▼
STATE_ACT_MODE_BLE_ACT (0x0201) ◄────────────────────┐
    │  ▲                                               │
    │  │ VRECT_DETn rising (coil removed)             │
    │  │                                               │
    ▼  │                                               │
STATE_ACT_MODE_WPT_HIGH (0x0209)                      │
    │  ▲                                               │
    │  │ batteries < 4.1V AND temp < 42°C             │
    │  │ AND VRECT_OVPn high                           │
    ▼  │                                               │
STATE_ACT_MODE_WPT_PAUSED (0x020A) ──────────────────►┘
                                     VRECT_DETn rising
```

### 4.3 WPT Entry: Coil Detected

Coil detection is **dual-path** for reliability:

**Path 1 — EXTI interrupt (primary):**
VRECT_DETn (PC7) is configured as a rising+falling EXTI interrupt (EXTI7). When the rectifier activates, VRECT_DETn goes low. This triggers `EXTI7_IRQHandler` → `HAL_GPIO_EXTI_IRQHandler()` → `HAL_GPIO_EXTI_Falling_Callback()` in `bsp_magnet.c`:

```c
// App/Bsp/Src/bsp_magnet.c
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == MAG_DET_Pin) {
        bsp_magnet_detect_cb(false);
    }
    else if (GPIO_Pin == VRECT_DETn_Pin) {
        app_func_sm_vrect_coil_cb(true);   // coil present
    }
}
```

**Path 2 — Polling (belt-and-suspenders):**
In the BLE active mode main loop, VRECT_DETn is polled every ~50 ms:

```c
// App/Src/app_mode_ble_active.c
if (HAL_GPIO_ReadPin(VRECT_DETn_GPIO_Port, VRECT_DETn_Pin) == GPIO_PIN_RESET) {
    app_func_sm_current_state_set(STATE_ACT_MODE_WPT_HIGH);
}
```

This ensures that if an EXTI edge is missed (e.g., device was sleeping), the transition still occurs on the next polling pass.

**State machine callback:**

```c
// App/Functions/Src/app_func_state_machine.c
void app_func_sm_vrect_coil_cb(bool coil_present) {
    if (coil_present) {
        if (curr_state >= STATE_ACT && curr_state != STATE_SHUTDOWN
                                    && curr_state != STATE_ACT_MODE_DVT) {
            curr_state = STATE_ACT_MODE_WPT_HIGH;
        }
    } else {
        if (curr_state == STATE_ACT_MODE_WPT_HIGH
         || curr_state == STATE_ACT_MODE_WPT_PAUSED) {
            curr_state = STATE_ACT_MODE_BLE_ACT;
        }
    }
}
```

The coil-detect path will not interrupt DVT mode or the SHUTDOWN state. All other active states (BLE_ACT, BLE_CONN, THERAPY, etc.) will yield to the charger.

### 4.4 WPT Exit: Coil Removed

When the coil is removed, VRECT_DETn rises. The rising EXTI fires `HAL_GPIO_EXTI_Rising_Callback()`, which calls `app_func_sm_vrect_coil_cb(false)`, setting state back to `STATE_ACT_MODE_BLE_ACT`. The `while()` condition in `app_mode_wpt_handler()` then fails on the next iteration, the handler runs its exit block (disables chargers), and `app_handler()` dispatches to `app_mode_ble_act_handler()` on the following loop.

### 4.5 Battery/Impedance Test Suppression

The RTC-based confirmation timer normally triggers scheduled `STATE_ACT_MODE_BATT_TEST` and `STATE_ACT_MODE_IMPED_TEST` transitions. During WPT charging, battery voltage is elevated by the charging current, which would produce false ER/EOS readings. A guard was added:

```c
// App/Functions/Src/app_func_state_machine.c
void app_func_sm_confirmation_timer_cb(void) {
    if (curr_state == STATE_ACT_MODE_WPT_HIGH
     || curr_state == STATE_ACT_MODE_WPT_PAUSED) {
        return;   // suppress test transitions during charging
    }
    // ... rest of timer logic
}
```

---

## 5. Boot-Time Safety Fix: VCHG_DISABLE

Prior to this change, `VCHG_DISABLE` (PC8) was initialized `LOW` at boot, which enabled the LTC3130 buck-boost converter immediately on power-up — even when no coil was present and no charger was intended to run. This was corrected in `gpio.c`:

```c
// Core/Src/gpio.c — before
HAL_GPIO_WritePin(GPIOC,
    ECG_RR_SDNn_Pin | VRECT_MON_EN_Pin | VCHG_DISABLE_Pin | TEMP_EN_Pin,
    GPIO_PIN_RESET);

// Core/Src/gpio.c — after
HAL_GPIO_WritePin(GPIOC,
    ECG_RR_SDNn_Pin | VRECT_MON_EN_Pin | TEMP_EN_Pin,
    GPIO_PIN_RESET);
HAL_GPIO_WritePin(VCHG_DISABLE_GPIO_Port, VCHG_DISABLE_Pin, GPIO_PIN_SET);
```

`VCHG_DISABLE = HIGH` means the converter is **disabled**. The WPT handler drives it LOW only when a coil is confirmed present and charging should proceed.

---

## 6. WPT Handler Deep Dive

`app_mode_wpt_handler()` in `App/Src/app_mode_wpt.c` owns the entire charging session. It is called from `app_handler()` when the state is `STATE_ACT_MODE_WPT_HIGH` or `STATE_ACT_MODE_WPT_PAUSED`, and it does not return until the coil is removed.

### 6.1 Entry Sequence

```
1. CHG_RATE1 = LOW, CHG_RATE2 = HIGH        → select 50 mA charge rate
2. VRECT_MON_EN = HIGH                       → enable VRECT voltage monitor
3. VCHG_DISABLE = LOW                        → enable boost converter
4. app_func_meas_therm_enable(true)          → power up thermistor circuit
5. app_func_meas_batt_mon_enable(true)       → power up battery monitor
6. Sample BATT_MON1 and BATT_MON2
   - If ≥ 1000 mV → battery_x_present = true
   - If  < 1000 mV → battery_x_present = false (no charger enabled)
7. CHG1_EN = HIGH if battery A present
   CHG2_EN = HIGH if battery B present
8. Start BLE advertising (see §6.4)
```

### 6.2 Main Loop (every 50 ms iteration, sample every 1 s)

The `wpt_ms_timer` static variable is decremented in `app_mode_wpt_timer_cb()`, which is called from `HAL_IncTick()` at 1 ms intervals. When it reaches zero, the 1-second measurement block runs and the timer resets to 1000.

**Per-second measurement block:**

```
1. app_func_meas_batt_mon_meas()    → vbat[0], vbat[1] in mV  (for broadcast only)
2. app_func_meas_therm_meas() ×3   → THERM_REF, THERM_OUT, THERM_OFST in mV
3. app_mode_wpt_calc_temperature()  → temp_c (float, °C)
4. Battery absence detection         → per-battery 2-strike rule (1 V threshold)
5. Read CHG1/2_OVP_ERRn, VRECT_OVPn, CHG1/2_STATUS via HAL_GPIO_ReadPin
6. Evaluate state transition (§6.3) → driven by CHGx_STATUS, temp, OVP only
7. app_mode_ble_act_adv_msd_update() → refresh BLE advertisement
```

Battery voltage is measured every second and appears in the BLE advertisement (MSD bytes 1–2), but it does not drive any charging state decision. The 1 V threshold used in battery absence detection is only for determining whether a physical battery is connected at all — it is not used to infer charge state or to trigger HIGH↔PAUSED transitions.

**Between sample ticks:**

```
app_func_ble_new_state_get()
bsp_sp_cmd_handler()
HAL_Delay(50)
```
If the BLE advertisement burst ends (`BLE_STATE_ADV_STOP`), a new burst is immediately started with refreshed MSD.

### 6.3 Charging State Transitions

#### WPT_HIGH → WPT_PAUSED

Only external protection faults trigger a pause. Charge completion is **not** a firmware pause trigger — the LTC4065 handles termination and re-charge autonomously with `CHGx_EN` held HIGH.

| Condition | Signal / Logic |
|-----------|----------------|
| Battery A overvoltage error | `CHG1_OVP_ERRn = LOW` and `battery_a_present` |
| Battery B overvoltage error | `CHG2_OVP_ERRn = LOW` and `battery_b_present` |
| Temperature too high | `temp_c > 42.0°C` |
| Rectifier overvoltage | `VRECT_OVPn = LOW` |

On entering PAUSED:
```
VCHG_DISABLE = HIGH    (disable converter)
CHG1_EN = LOW
CHG2_EN = LOW
curr_state = STATE_ACT_MODE_WPT_PAUSED
```

#### WPT_PAUSED → WPT_HIGH

Resume requires only that the protection faults have cleared and at least one battery is present. The firmware does not consult `CHGx_STATUS` for resume decisions — the LTC4065 autonomously manages its own charge cycle (including re-charge after self-discharge) once `CHGx_EN` is asserted and `VCHG_DISABLE` is de-asserted.

Battery voltage is measured and broadcast in the BLE advertisement every second but does not gate any state transition.

| Condition | Logic |
|-----------|-------|
| At least one battery present | `battery_a_present OR battery_b_present` |
| Temperature below threshold | `temp_c < 42.0°C` |
| No rectifier overvoltage | `VRECT_OVPn = HIGH` |

On resuming:
```
VCHG_DISABLE = LOW     (enable converter)
CHG1_EN = HIGH         (if battery A present)
CHG2_EN = HIGH         (if battery B present)
curr_state = STATE_ACT_MODE_WPT_HIGH
```

Once `CHGx_EN` is HIGH and the converter is enabled, the LTC4065 takes over: it charges at the programmed rate, terminates at C/10, and autonomously re-initiates a charge cycle if the battery self-discharges below its internal re-charge threshold.

### 6.4 BLE Advertising During WPT

WPT mode uses the same BLE infrastructure as BLE active mode. `app_mode_ble_act_adv_msd_update()` is called to populate the MSD, which means every BLE scan of a charging device returns live measurements: battery voltages, thermistor readings, impedance, DVDD, and all GPIO status flags.

The advertisement timeout is set to 1 second (`wpt_adv_interval_s = 1`), matching the measurement cadence. The BLE module advertises a 1-second burst, stops, the firmware calls `app_mode_ble_act_adv_msd_update()` with the freshest data, then restarts the burst. This gives any observing host roughly one fresh advertisement per second.

#### MSD Byte Layout

The MSD payload is 24 bytes total. Bytes 0–7 carry analog measurements; bytes 8–9 are the 16-bit GPIO/state status word.

**Bytes 0–7 — Analog measurements:**

| Byte | Signal | Units | Notes |
|------|--------|-------|-------|
| 0 | DVDD voltage | × 100 mV | e.g. `0x0C` = 1.2 V |
| 1 | Battery A voltage | × 100 mV | e.g. `0x28` = 4.0 V |
| 2 | Battery B voltage | × 100 mV | |
| 3 | Impedance A | × 10 mV | |
| 4 | Impedance B | × 10 mV | |
| 5 | Thermistor REF | × 10 mV | |
| 6 | Thermistor OUT | × 10 mV | |
| 7 | Thermistor OFST | × 10 mV | |

**Bytes 8–9 — Status bits (16-bit little-endian):**

| Bit | Signal | `1` means | `0` means |
|-----|--------|-----------|-----------|
| 0 | `VRECT_DETn` | No coil present | Coil present (active low) |
| 1 | `VRECT_OVPn` | Rectifier voltage OK | Rectifier OVP fault |
| 2 | `VCHG_PGOOD` | Converter output good | Converter off / fault |
| 3 | `CHG1_STATUS` (nCHRG) | Battery A idle / charge complete | Battery A actively charging |
| 4 | `CHG1_OVP_ERRn` | Battery A OVP OK | Battery A OVP error |
| 5 | `CHG2_STATUS` (nCHRG) | Battery B idle / charge complete | Battery B actively charging |
| 6 | `CHG2_OVP_ERRn` | Battery B OVP OK | Battery B OVP error |
| 7 | `ENG2_SDNn` | ENG2 enabled | ENG2 shutdown |
| 8 | `ENG1_SDNn` | ENG1 enabled | ENG1 shutdown |
| 9 | `ECG_RLD` | ECG RLD enabled | ECG RLD off |
| 10 | `ECG_HR_SDNn` | ECG HR enabled | ECG HR shutdown |
| 11 | `ECG_RR_SDNn` | ECG RR enabled | ECG RR shutdown |
| 12 | `TEMP_EN` | Thermistor powered | Thermistor off |
| 13 | `IMP_EN` | Impedance measurement enabled | Off |
| 14 | `WPT_ACTIVE` | Device in WPT_HIGH or WPT_PAUSED | Not in WPT mode |
| 15 | `WPT_PAUSED` | Device in WPT_PAUSED specifically | Not paused |

**Example decode — status bytes `0x7B 0x10`:**

`0x7B` = `0111 1011` → bits 0,1,3,4,5,6 set: VRECT_DETn(no coil), VRECT_OVPn(OK), CHG1_STATUS(idle), CHG1_OVP_ERRn(OK), CHG2_STATUS(idle), CHG2_OVP_ERRn(OK)
`0x10` = `0001 0000` → bit 12 set: TEMP_EN(thermistor powered)

Interpretation: device idle in BLE active mode, no coil present, thermistor powered, both chargers idle, all protection signals healthy.

### 6.5 Exit Sequence

Triggered when `curr_state` is no longer `WPT_HIGH` or `WPT_PAUSED` (set by the VRECT rising-edge EXTI callback or a BLE authentication transitioning to BLE_CONN):

```
VCHG_DISABLE = HIGH    (disable converter)
CHG1_EN = LOW
CHG2_EN = LOW
VRECT_MON_EN = LOW     (disable VRECT monitor)
app_func_ble_enable(false)
```

---

## 7. Battery Absence Detection

The "2-strike" rule prevents spurious absence detection from a single noisy ADC reading:

```c
if (vbat[x] < 1000 mV) {
    battery_x_low_count++;
    if (battery_x_low_count >= 2 && battery_x_present) {
        battery_x_present = false;
        CHGx_EN = LOW;    // stop charging this slot
    }
} else {
    battery_x_low_count = 0;   // reset on any good reading
}
```

- Two **consecutive** samples below 1 V are required before a battery is declared absent.
- A single good reading resets the counter — there is no hysteresis on the other side.
- If both batteries become absent, the device stays in WPT state (coil still present) but does not attempt charging. The chargers are disabled and the device continues advertising.

---

## 8. Temperature Measurement

### 8.1 Circuit Model

```
THERM_REF ──── 49.9 kΩ sense ──── THERM_OUT ──── NTC thermistor ──── THERM_OFST (ground ref)
```

All three voltages are measured by the ADC and passed to `app_mode_wpt_calc_temperature()`.

### 8.2 Resistance Calculation

```c
float voltage      = therm_out_mv - therm_ofst_mv;   // voltage across thermistor
float voltage_drop = therm_ref_mv - therm_out_mv;    // voltage across 49.9 kΩ sense resistor
float current      = voltage_drop / 49900.0f;         // current through series circuit
float resistance   = voltage / current;               // thermistor resistance in Ω
```

If either `voltage` or `voltage_drop` is ≤ 0 (thermistor unpowered or shorted), the function returns 0.0°C and charging is not paused on temperature grounds.

### 8.3 Lookup Table and Interpolation

The 104AP-2 NTC characteristic is stored as a 5-point table (ported from the OpenNerve charger firmware):

| Temperature (°C) | Resistance (Ω) |
|-----------------|----------------|
| 20 | 126,400 |
| 25 | 100,000 |
| 30 | 79,590 |
| 40 | 51,320 |
| 50 | 33,790 |

Temperature is found by linear interpolation between the two bracketing entries:

```c
float frac = (r_hi - resistance) / (r_hi - r_lo);
return t_lo + frac * (t_hi - t_lo);
```

Values outside the table range (> 126,400 Ω → < 20°C, or < 33,790 Ω → > 50°C) are clamped to the table endpoints.

**Threshold check:** `42.0°C` corresponds to approximately **47,814 Ω** (interpolated between the 40°C and 50°C entries).

---

## 9. BLE Advertisement MSD Layout

`app_mode_ble_act_adv_msd_update()` populates the MSD buffer. This function is shared between BLE active mode and WPT mode; WPT bits are always written based on current state.

### Byte fields (MSD[0..7])

| Byte | Content | Scale |
|------|---------|-------|
| 0 | DVDD supply voltage | × 100 mV (e.g., 33 → 3.3 V) |
| 1 | Battery A voltage | × 100 mV |
| 2 | Battery B voltage | × 100 mV |
| 3 | Impedance channel A voltage | × 10 mV |
| 4 | Impedance channel B voltage | × 10 mV |
| 5 | THERM_REF voltage | × 10 mV |
| 6 | THERM_OUT voltage | × 10 mV |
| 7 | THERM_OFST voltage | × 10 mV |

### Bit field (MSD[8..9], starting at buff_offset)

The 16 bits packed into bytes 8–9 are:

| Bit | Signal | Active | Source |
|-----|--------|--------|--------|
| 0 | VRECT_DETn | 1 = coil absent (pin high) | GPIO |
| 1 | VRECT_OVPn | 1 = no OVP (pin high) | GPIO |
| 2 | VCHG_PGOOD | 1 = converter output good | GPIO |
| 3 | CHG1_STATUS | 1 = battery A charge complete | GPIO |
| 4 | CHG1_OVP_ERRn | 1 = no OVP error (pin high) | GPIO |
| 5 | CHG2_STATUS | 1 = battery B charge complete | GPIO |
| 6 | CHG2_OVP_ERRn | 1 = no OVP error (pin high) | GPIO |
| 7 | ENG2_SDNn | engine 2 shutdown state | GPIO |
| 8 | ENG1_SDNn | engine 1 shutdown state | GPIO |
| 9 | ECG_RLD | ECG right-leg drive state | GPIO |
| 10 | ECG_HR_SDNn | ECG HR shutdown state | GPIO |
| 11 | ECG_RR_SDNn | ECG RR shutdown state | GPIO |
| 12 | TEMP_EN | temperature circuit enable | GPIO |
| 13 | IMP_EN | impedance circuit enable | GPIO |
| **14** | **WPT_ACTIVE** | **1 = WPT_HIGH or WPT_PAUSED** | **State** |
| **15** | **WPT_PAUSED** | **1 = specifically WPT_PAUSED** | **State** |

Bits 14–15 are written directly into `buff_offset[1]` (MSD byte 9) bits 6 and 7. They reflect the firmware state, not a GPIO pin.

### MSD[LEN_BLE_MSD_MAX - 1]

The last byte is always the hardware version (`HW_VERSION = 21`).

---

## 10. GPIO Reference Table

All pins relevant to WPT, with their port, pin number, direction, and active level:

| Signal | Port | Pin | Direction | Active Level | Notes |
|--------|------|-----|-----------|--------------|-------|
| VRECT_DETn | PC | 7 | Input / EXTI | LOW = coil present | EXTI7, rising+falling |
| VRECT_OVPn | PC | 6 | Input | LOW = OVP fault | Pulled up |
| VRECT_MON_EN | PC | 10 | Output | HIGH = enabled | Enable VRECT ADC path |
| VRECT_MON | PC | 4 | Analog | — | ADC input |
| VCHG_DISABLE | PC | 8 | Output | HIGH = disabled | Starts HIGH at boot |
| VCHG_PGOOD | PC | 9 | Input | HIGH = good | No pull |
| CHG1_EN | PE | 10 | Output | HIGH = enabled | Starts LOW |
| CHG1_STATUS | PE | 14 | Input | HIGH = complete | Pulled up |
| CHG1_OVP_ERRn | PE | 12 | Input | LOW = OVP error | Pulled up |
| CHG2_EN | PE | 11 | Output | HIGH = enabled | Starts LOW |
| CHG2_STATUS | PE | 15 | Input | HIGH = complete | Pulled up |
| CHG2_OVP_ERRn | PE | 13 | Input | LOW = OVP error | Pulled up |
| CHG_RATE1 | PE | 8 | Output | — | LOW for 50 mA |
| CHG_RATE2 | PE | 9 | Output | — | HIGH for 50 mA |
| BATT_MON1 | PA | 0 | Analog | — | Battery A ÷2 |
| BATT_MON2 | PA | 1 | Analog | — | Battery B ÷2 |
| BATT_MON_EN | PA | 2 | Output | HIGH = enabled | |
| THERM_REF | PC | 2 | Analog | — | ADC reference |
| THERM_OUT | PC | 1 | Analog | — | ADC NTC junction |
| THERM_OFST | PC | 0 | Analog | — | ADC ground offset |
| TEMP_EN | PC | 5 | Output | HIGH = enabled | Powers thermistor circuit |

---

## 11. Interrupt and Timer Architecture

### 11.1 EXTI7 (VRECT_DETn)

```
Hardware edge on PC7
    │
    ▼
EXTI7_IRQHandler()                 [Core/Src/stm32u5xx_it.c]
    │
    ▼
HAL_GPIO_EXTI_IRQHandler(VRECT_DETn_Pin)
    │
    ├─ falling edge ──► HAL_GPIO_EXTI_Falling_Callback()  [App/Bsp/Src/bsp_magnet.c]
    │                       └──► app_func_sm_vrect_coil_cb(true)
    │
    └─ rising edge  ──► HAL_GPIO_EXTI_Rising_Callback()   [App/Bsp/Src/bsp_magnet.c]
                            └──► app_func_sm_vrect_coil_cb(false)
```

EXTI3 (MAG_DET) and EXTI7 (VRECT_DETn) share the same callback functions; each is dispatched by pin ID.

### 11.2 WPT 1-Second Sample Timer

```
SysTick (1 ms)
    │
    ▼
HAL_IncTick()                      [App/Src/app.c]
    │
    ├──► app_mode_ble_act_timer_cb()    (BLE active timeout)
    ├──► app_mode_ble_conn_timer_cb()   (BLE connection timeout)
    ├──► app_mode_dvt_acc_timer_cb()    (DVT accelerometer)
    └──► app_mode_wpt_timer_cb()        (WPT 1-second sample timer)
                │
                ▼
         wpt_ms_timer-- (until 0)
```

The WPT timer callback runs every 1 ms regardless of current application state — it simply decrements a counter. The counter is only reset inside `app_mode_wpt_handler()`, so there is no concern about it counting while in other modes.

---

## 12. Constants Reference

All thresholds are defined in `App/Inc/app_mode_wpt.h`:

```c
#define WPT_BATT_ABSENT_THRESHOLD_MV   1000U    // < 1.0 V → battery absent (presence detection only)
#define WPT_BATT_ABSENT_CONSECUTIVE    2U        // 2 consecutive samples required to declare absent
#define WPT_THERM_PAUSE_THRESHOLD_C    42.0f    // > 42°C → pause charging
#define WPT_THERM_RESUME_THRESHOLD_C   42.0f    // < 42°C → may resume (protection condition, not IC-driven)
#define WPT_THERM_SENSE_RESISTOR_OHM   49900.0f // 49.9 kΩ series sense resistor
```

Battery voltage is **not** used to gate charge state transitions. The `WPT_BATT_ABSENT_THRESHOLD_MV` constant is used solely for detecting whether a physical battery is present at all (both at handler entry and during the 2-strike absence check in the main loop). Charge-complete and resume decisions are managed autonomously by the LTC4065; the firmware only intervenes for thermal and OVP protection faults.

---

## 13. How to Verify

This section describes a systematic verification procedure. Work through the steps in order; each step assumes the previous steps passed.

---

### Step 1 — Boot: VCHG_DISABLE starts HIGH

**Purpose:** Confirm the safety fix — the boost converter must not run on power-up.

**Method:**
1. Flash the new firmware.
2. Immediately after power-on (before bringing any charger near), measure `VCHG_DISABLE` (PC8) with a multimeter or logic analyzer.
3. Expected: `VCHG_DISABLE = HIGH` (≈ 3.3 V).
4. Confirm `CHG1_EN` (PE10) and `CHG2_EN` (PE11) are LOW.
5. Confirm `VRECT_MON_EN` (PC10) is LOW.

**Pass criteria:** All charging outputs are de-asserted at boot, regardless of whether a coil is present.

---

### Step 2 — BLE Active Mode: No False WPT Transitions

**Purpose:** Confirm the device stays in BLE active mode when no coil is present.

**Method:**
1. Put the device in range of a BLE scanner (nRF Connect app or similar).
2. Verify it advertises normally with state bits 14–15 = `00` (WPT_ACTIVE = 0, WPT_PAUSED = 0).
3. Observe MSD byte 9 bit 0 (VRECT_DETn) = `1` (pin high, no coil).
4. Leave for several minutes. Confirm no spontaneous WPT state entry.

**Pass criteria:** MSD bit 14 stays 0. Device does not exit BLE active mode.

---

### Step 3 — Coil Detection: EXTI and State Transition

**Purpose:** Confirm VRECT_DETn EXTI fires and drives a state transition to WPT_HIGH.

**Method:**
1. Monitor BLE advertisement MSD with a BLE scanner in continuous scan mode.
2. Bring the WPT charging coil near the device.
3. Observe `VRECT_DETn` (PC8) go LOW with a logic analyzer or voltmeter.
4. Within one BLE advertisement interval (≤ 2 s), observe MSD bit 14 (`WPT_ACTIVE`) set to `1`.
5. Observe MSD bit 15 (`WPT_PAUSED`) = `0` (charging is active, not paused).
6. Observe MSD bit 0 (`VRECT_DETn`) = `0` (pin now low = coil present).
7. Confirm `CHG1_EN` and/or `CHG2_EN` have gone HIGH (depending on battery presence).
8. Confirm `VCHG_DISABLE` is LOW (converter running).
9. Confirm `VCHG_PGOOD` goes HIGH (converter output is good) — visible in MSD bit 2.

**Pass criteria:** State transitions to WPT_HIGH within one advertisement cycle of coil placement. Charging outputs assert correctly.

---

### Step 4 — Charging in Progress: BLE Advertisement Reflects Live Data

**Purpose:** Confirm that MSD values update every second with real measurements.

**Method:**
1. With the coil present and charging active, capture BLE advertisements continuously for at least 30 seconds.
2. Verify MSD bytes 1–2 (battery voltages) gradually increase over time.
3. Verify MSD bytes 5–7 (thermistor voltages) are non-zero and plausible.
4. Verify the advertisement interval is approximately 1 Hz.
5. Confirm MSD byte `LEN_BLE_MSD_MAX - 1` = 21 (hardware version).

**Pass criteria:** Live measurements appear in the advertisement and increment as the battery charges.

---

### Step 5 — Single Battery Operation

**Purpose:** Confirm that a missing battery does not prevent charging of the present battery.

**Method:**
1. Physically disconnect Battery B (or ensure its voltage is < 1 V).
2. Bring the coil near the device.
3. Confirm `CHG1_EN = HIGH` (Battery A charging).
4. Confirm `CHG2_EN = LOW` (Battery B not enabled).
5. Monitor over 2 seconds — `CHG2_EN` must remain LOW.

**Pass criteria:** Only the present battery's charger is enabled. The absent battery does not cause a fault.

---

### Step 6 — 2-Strike Battery Absence Detection

**Purpose:** Confirm that a single low voltage reading does not immediately disable a charger.

**Method:** This test is best done in a DVT harness or via a bench power supply simulating a battery voltage. Applying a single sub-1V pulse should not result in `CHGx_EN` going low; two consecutive low readings (≥ 2 seconds apart) should.

**Alternatively:** Review the code path in `app_mode_wpt.c` lines 202–225 and confirm the counter logic in code review.

**Pass criteria:** `CHGx_EN` only deasserts after `battery_x_low_count >= 2` (two consecutive 1-second samples below 1 V).

---

### Step 7 — WPT Pause: Charge Complete (CHGx_STATUS)

**Purpose:** Confirm that when a battery reports charge complete, charging pauses correctly.

**Method:**
1. Charge to full (or simulate by raising `CHGx_STATUS` line HIGH using a test fixture).
2. Monitor MSD:
   - Bit 3 (`CHG1_STATUS`) or bit 5 (`CHG2_STATUS`) should read `1`.
3. If both present batteries report STATUS = HIGH, observe:
   - `VCHG_DISABLE` goes HIGH.
   - `CHG1_EN`, `CHG2_EN` go LOW.
   - MSD bit 14 (`WPT_ACTIVE`) remains `1`.
   - MSD bit 15 (`WPT_PAUSED`) goes `1`.
4. Leave the coil in place. Verify the device stays in WPT_PAUSED, advertising continuously.

**Pass criteria:** Device enters WPT_PAUSED. VCHG_DISABLE = HIGH. BLE advertising continues.

---

### Step 8 — WPT Resume from Paused

**Purpose:** Confirm that charging resumes once protection faults clear, with the LTC4065 managing the charge cycle autonomously.

**Method:**
1. Start from WPT_PAUSED triggered by a thermal or OVP fault (see Steps 7/9/10).
2. Allow the fault condition to clear (e.g., let the device cool below 42°C, or remove the OVP condition).
3. Confirm `VRECT_OVPn = HIGH` (no rectifier OVP).
4. Observe within one 1-second sample:
   - `VCHG_DISABLE` goes LOW.
   - `CHGx_EN` reasserts for present batteries.
   - MSD bit 15 (`WPT_PAUSED`) returns to `0`.
   - MSD bit 14 (`WPT_ACTIVE`) remains `1`.
5. Confirm `nCHRG` (observed via `CHGx_STATUS` in MSD) goes LOW — indicating the LTC4065 has resumed charging.
6. **Negative test:** Confirm the device does NOT fail to resume simply because battery voltage is high — once faults are cleared and a battery is present, the firmware resumes and the IC manages the charge cycle.

**Pass criteria:** Device transitions back to WPT_HIGH once protection conditions clear, with thermal < 42°C and VRECT_OVPn HIGH. LTC4065 autonomously manages the charge current.

---

### Step 9 — WPT Pause: Thermal (Temperature > 42°C)

**Purpose:** Confirm that over-temperature pauses charging.

**Method:**
1. With charging active (WPT_HIGH), apply heat to the thermistor or simulate a high-temperature reading by reducing `THERM_OUT` voltage relative to `THERM_REF` to raise the measured resistance.
2. When the calculated temperature exceeds 42°C, observe:
   - `VCHG_DISABLE` goes HIGH.
   - `CHGx_EN` goes LOW.
   - MSD bit 15 (`WPT_PAUSED`) = `1`.
3. Allow the device to cool. Observe charging resuming when temperature drops below 42°C.

**Pass criteria:** Charging halts within 1 second of the thermal threshold being crossed.

---

### Step 10 — WPT Exit: Coil Removed

**Purpose:** Confirm the device returns cleanly to BLE active mode when the coil is removed.

**Method:**
1. With charging active (WPT_HIGH or WPT_PAUSED), remove the coil.
2. Observe `VRECT_DETn` (PC7) go HIGH.
3. Confirm within one BLE advertisement cycle:
   - `VCHG_DISABLE = HIGH`.
   - `CHG1_EN = LOW`, `CHG2_EN = LOW`.
   - `VRECT_MON_EN = LOW`.
   - MSD bit 14 (`WPT_ACTIVE`) = `0`.
   - MSD bit 15 (`WPT_PAUSED`) = `0`.
   - MSD bit 0 (`VRECT_DETn`) = `1` (pin high again).
4. Confirm BLE advertising continues (device is now in BLE active mode).

**Pass criteria:** All charging outputs de-assert immediately on coil removal. Device returns to BLE active mode without requiring a reset.

---

### Step 11 — No Battery/Impedance Test During Charging

**Purpose:** Confirm that the RTC-based periodic test scheduler does not interrupt charging.

**Method:**
1. Set a short impedance test or battery test interval (via parameter write in DVT mode or BLE connection).
2. Begin a WPT charging session.
3. Wait for the timer to expire (observe it would normally fire in non-WPT mode).
4. Confirm the device stays in WPT_HIGH or WPT_PAUSED — it does **not** transition to `STATE_ACT_MODE_BATT_TEST` or `STATE_ACT_MODE_IMPED_TEST`.

**Pass criteria:** The `app_func_sm_confirmation_timer_cb()` WPT guard correctly returns early and no test mode is entered during a charging session.

---

### Step 12 — End-to-End Charge Cycle

**Purpose:** Full system validation of a complete charge from depleted to full.

**Method:**
1. Start with both batteries at a low-but-valid voltage (e.g., 3.5 V each).
2. Place the device on the WPT charging pad.
3. Monitor BLE advertisement throughout. Log: timestamps, battery A voltage (MSD[1] × 100 mV), battery B voltage (MSD[2] × 100 mV), WPT_ACTIVE (bit 14), WPT_PAUSED (bit 15), CHG1_STATUS (bit 3), CHG2_STATUS (bit 5).
4. Confirm batteries rise steadily toward 4.2 V.
5. Confirm CHGx_STATUS bits assert as each battery reaches full charge.
6. Confirm device transitions to WPT_PAUSED after both batteries complete.
7. Remove coil. Confirm clean return to BLE active mode.
8. Remove and reinsert coil. Confirm re-entry into WPT_HIGH and immediate correct operation.

**Pass criteria:** Complete charge cycle completes without manual intervention. BLE data accurately reflects hardware state throughout.

---

## 14. Known Limitations and Future Considerations

- **No hysteresis on thermal resume:** The pause and resume thresholds are both 42.0°C. In a real thermal environment this may cause repeated pause/resume cycling near the threshold. Consider adding a lower resume threshold (e.g., 40°C) if this is observed in testing.

- **No deep sleep during WPT:** The WPT handler's main loop calls `HAL_Delay(50)` continuously. The device does not enter LP sleep during a charging session. This is by design (continuous BLE advertising required) but means higher MCU power draw during charging.

- **BLE command parsing during WPT:** The WPT handler does not call `app_func_command_req_parser_set()`. Whichever parser was active before WPT mode (typically the BLE active mode parser) remains set. This means an authenticated BLE connection during WPT mode will transition the state machine to `STATE_ACT_MODE_BLE_CONN`, which will exit the WPT handler's while loop and trigger the clean exit sequence (charging disabled, BLE disabled). If BLE connectivity during charging is required, the WPT mode's command handling should be revisited.

- **VRECT_MON ADC not yet consumed:** `VRECT_MON_EN` is asserted during WPT mode but the `VRECT_MON` ADC channel is not sampled by the WPT handler. The rectified voltage is not yet reported in the BLE advertisement. This could be added as a future enhancement.
