# WPT Wireless Charging — Implementation Plan

## Context
The IPG hardware is fully wired for wireless power transfer (WPT) charging but the production firmware does not control it. All charging GPIO outputs are initialized low (chargers disabled), and only the digital status signals are passively read into BLE advertisements. This plan implements the autonomous charging state machine defined in CAR-212 and the WPT-State-Machine-Transitions diagram.

---

## Thermistor Temperature Formula

Found in the charger firmware at `OpenNerve-Charger/Firmware/Source-Code/src/service_layer/wpt/svc_wpt_manager.cpp` — function `CalculateTemperatureFromBle()`. This function was written specifically to interpret the IPG's three thermistor ADC values. Port it into `app_mode_wpt.c` as a static helper.

**Thermistor part:** 104AP-2 (10kΩ NTC)
**Series sense resistor:** 49.9 kΩ (k_resistor_value = 49900)

**Formula (port to C float arithmetic):**
```c
// All voltages in mV (from app_func_meas_therm_meas)
float voltage      = therm_out_mv - therm_ofst_mv;   // across thermistor
float voltage_drop = therm_ref_mv - therm_out_mv;    // across 49.9kΩ sense resistor
float current      = voltage_drop / 49900.0f;        // proportional current
float resistance   = voltage / current;              // thermistor resistance in Ω
// Then interpolate against lookup table (see below)
```

**Lookup table (104AP-2, from charger firmware `svc_wpt_manager.h`):**
```c
// {temp_C, resistance_ohms}
{20, 126400},
{25, 100000},
{30,  79590},
{40,  51320},
{50,  33790},
```

**42°C threshold resistance** (interpolated between 40°C and 50°C entries):
- 51320 + (42-40)/(50-40) × (33790-51320) = 51320 - 3506 = **~47,814 Ω**

Implement as `static float app_mode_wpt_calc_temperature(uint16_t therm_ref_mv, uint16_t therm_out_mv, uint16_t therm_ofst_mv)` in `app_mode_wpt.c`. Use the full interpolation (not just the hardcoded threshold) so the value is available for the BLE advertisement too.

---

## New States

Add to `app_func_state_machine.h` alongside existing state constants:
```c
#define STATE_ACT_MODE_WPT_HIGH    0x0209   // Coil present, charging active
#define STATE_ACT_MODE_WPT_PAUSED  0x020A   // Coil present, charging paused
```

---

## File-by-File Changes

### 1. `Core/Src/gpio.c` — Fix VCHG_DISABLE default (safety fix)
CAR-212 specifies VCHG_DISABLE = HIGH in the no-charger default state. Currently it is initialized LOW (converter enabled). Change line 84:
```c
// FROM:
HAL_GPIO_WritePin(GPIOC, ECG_RR_SDNn_Pin|VRECT_MON_EN_Pin|VCHG_DISABLE_Pin|TEMP_EN_Pin, GPIO_PIN_RESET);
// TO:
HAL_GPIO_WritePin(GPIOC, ECG_RR_SDNn_Pin|VRECT_MON_EN_Pin|TEMP_EN_Pin, GPIO_PIN_RESET);
HAL_GPIO_WritePin(VCHG_DISABLE_GPIO_Port, VCHG_DISABLE_Pin, GPIO_PIN_SET);
```
Also reconfigure VRECT_DETn (PC7) from plain input to EXTI falling+rising edge interrupt.

### 2. `Core/Src/stm32u5xx_it.c` — New EXTI7 interrupt handler
Add handler for VRECT_DETn (PC7 = EXTI line 7). Calls `HAL_GPIO_EXTI_IRQHandler(VRECT_DETn_Pin)`.

### 3. `App/Functions/Inc/app_func_state_machine.h` + `.c` — VRECT detection, WPT state suppression

**Header additions:**
- New state constants (see §New States)
- Declare `app_func_sm_vrect_coil_cb(bool coil_present)`

**In `app_func_state_machine.c`, new function `app_func_sm_vrect_coil_cb(bool coil_present)`:**
- Mirrors the magnet callback pattern
- When `coil_present = true`: set `STATE_ACT_MODE_WPT_HIGH` from any state
- When `coil_present = false`: if in WPT_HIGH or WPT_PAUSED, set `STATE_ACT_MODE_BLE_ACT`

**In `app_func_sm_confirmation_timer_cb()`:**
- Add guard: if current state is WPT_HIGH or WPT_PAUSED, do not trigger `STATE_ACT_MODE_BATT_TEST` (battery voltage is elevated during charging and would give false ER/EOS readings).
- Still allow sleep prevention: WPT states never transition to sleep.

### 4. `App/Src/app.c` — Wire in new states and VRECT EXTI callback
- Add `case STATE_ACT_MODE_WPT_HIGH:` and `case STATE_ACT_MODE_WPT_PAUSED:` to the switch in `app_handler()`, both calling `app_mode_wpt_handler()`
- Add `HAL_GPIO_EXTI_Falling_Callback` / `Rising_Callback` cases for `VRECT_DETn_Pin` that call `app_func_sm_vrect_coil_cb()`

### 5. New file: `App/Inc/app_mode_wpt.h` + `App/Src/app_mode_wpt.c`

**Public interface:**
```c
void app_mode_wpt_handler(void);
void app_mode_wpt_timer_cb(void);   // called from SysTick / HAL_IncTick
```

**Internal state (module-level statics):**
```c
static bool battery_a_present;
static bool battery_b_present;
static uint8_t battery_a_low_count;   // for absence detection (2-strike rule)
static uint8_t battery_b_low_count;
```

**Handler logic (`app_mode_wpt_handler`):**

*Entry:*
- Set CHG_RATE1 = LOW, CHG_RATE2 = HIGH (50 mA)
- Set VRECT_MON_EN = HIGH
- Set VCHG_DISABLE = LOW (enable converter)
- Detect battery presence (sample both voltages; if < 1V on first sample, mark absent immediately; otherwise mark present)
- Enable CHGx_EN for present batteries
- Start BLE advertising (same setup as BLE active mode — reuse `app_mode_ble_act_adv_msd_update` with WPT status bits added)

*Main loop (runs while STATE_ACT_MODE_WPT_HIGH or WPT_PAUSED):*

Every ~1 second (gated by a ms timer like `adv_ms_timer` pattern):
1. Sample BATT_MON1 and BATT_MON2 (reuse `app_func_meas_batt_mon_meas`)
2. Sample THERM_REF, THERM_OUT, THERM_OFST (reuse `app_func_meas_therm_meas`)
3. Check CHG1_STATUS and CHG2_STATUS GPIO pins
4. Run battery absence detection (2-consecutive-below-1V rule per battery)
5. Evaluate state transitions (see below)
6. Update BLE MSD with fresh values

*Transition logic — WPT High → WPT Paused:*
- `CHGx_OVP_ERRn = low` for any present battery (voltage ≥ 4.277V), OR
- Thermistor > 42°C threshold, OR
- `VRECT_OVPn = low`
- Also: if `CHGx_STATUS = high` for a battery → disable that battery's CHGx_EN; if ALL present batteries have STATUS high → transition to WPT Paused
- On entering Paused: VCHG_DISABLE = HIGH, CHG1_EN = LOW, CHG2_EN = LOW

*Transition logic — WPT Paused → WPT High:*
- All present batteries voltage < 4.1V AND thermistor < 42°C AND VRECT_OVPn = high
- On entering High: VCHG_DISABLE = LOW, enable CHGx_EN for present (non-done) batteries

*Exit (field lost — VRECT_DETn = high, set by EXTI callback):*
- VCHG_DISABLE = HIGH
- CHG1_EN = LOW, CHG2_EN = LOW
- VRECT_MON_EN = LOW
- Stop BLE advertising
- Return (app_handler will pick up STATE_ACT_MODE_BLE_ACT next loop)

### 6. `App/Src/app_mode_ble_active.c` — Two changes

**a) Poll VRECT_DETn in the main loop (belt-and-suspenders alongside EXTI):**
In the 50ms polling loop, read `VRECT_DETn_Pin`. If low, call `app_func_sm_current_state_set(STATE_ACT_MODE_WPT_HIGH)` — the while condition `curr_state == STATE_ACT_MODE_BLE_ACT` will then exit.

**b) Add WPT status bits to MSD (bits 14–15 in the existing status area):**
```
bit 14: WPT_ACTIVE  (1 = device is in WPT_HIGH or WPT_PAUSED state)
bit 15: WPT_PAUSED  (1 = specifically in WPT_PAUSED state)
```
These go into `p_msd[9]` bits 6–7, reusing the existing `setBitFromGpioState` pattern but driven by state rather than a GPIO pin.

---

## Battery Absence Detection Detail
In the WPT loop, per battery:
- If `vbat < 1000 mV` (1V): increment `battery_x_low_count`
- If `battery_x_low_count >= 2`: set `battery_x_present = false`, disable `CHGx_EN`
- If `vbat >= 1000 mV`: reset `battery_x_low_count` to 0
- If both batteries absent: stay in WPT Paused and do not attempt charging

---

## Constants to Define (in `app_mode_wpt.h`)
```c
#define WPT_BATT_ABSENT_THRESHOLD_MV     1000    // < 1V = battery absent
#define WPT_BATT_ABSENT_CONSECUTIVE      2       // 2 consecutive samples
#define WPT_BATT_RESUME_THRESHOLD_MV     4100    // < 4.1V → resume charging from Paused
#define WPT_THERM_PAUSE_THRESHOLD_C      42.0f   // °C — pause charging above this
#define WPT_THERM_RESUME_THRESHOLD_C     42.0f   // °C — resume when below this
#define WPT_THERM_SENSE_RESISTOR_OHM     49900.0f  // 49.9kΩ series sense resistor
```

---

## Summary of All Modified/Created Files

| File | Change Type | What Changes |
|------|-------------|--------------|
| `Core/Src/gpio.c` | Modify | VCHG_DISABLE default HIGH; VRECT_DETn as EXTI |
| `Core/Src/stm32u5xx_it.c` | Modify | Add EXTI7_IRQHandler for VRECT_DETn |
| `App/Functions/Inc/app_func_state_machine.h` | Modify | New state constants, new callback declaration |
| `App/Functions/Src/app_func_state_machine.c` | Modify | New vrect callback, suppress battery test in WPT states |
| `App/Src/app.c` | Modify | New switch cases for WPT states, EXTI callbacks for VRECT |
| `App/Src/app_mode_ble_active.c` | Modify | VRECT_DETn poll in loop, WPT bits in MSD |
| `App/Inc/app_mode_wpt.h` | **Create** | WPT handler public interface + constants |
| `App/Src/app_mode_wpt.c` | **Create** | Full WPT charging state handler |
