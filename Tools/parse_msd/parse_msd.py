#!/usr/bin/env python3
"""
OpenNerve IPG Gen2 — BLE Manufacturer Specific Data (MSD) Parser
Firmware source: App/Src/app_mode_ble_active.c :: app_mode_ble_act_adv_msd_update()

MSD layout (24 bytes, LEN_BLE_MSD_MAX = 24):
  [0]     DVDD          100 mV / LSB
  [1]     Battery A     100 mV / LSB
  [2]     Battery B     100 mV / LSB
  [3]     Impedance A    10 mV / LSB
  [4]     Impedance B    10 mV / LSB
  [5]     Therm Ref      10 mV / LSB
  [6]     Therm Out      10 mV / LSB
  [7]     Therm Offset   10 mV / LSB
  [8]     GPIO flags lo  (bits 0-7)
  [9]     GPIO flags hi  (bits 8-13)
  [10-22] BLE ID         (variable, up to 13 bytes)
  [23]    HW Version     (raw byte; current firmware = 21)
"""

import math
import sys
import textwrap

LEN_MSD = 24

# ── Thermistor temperature calculation ───────────────────────────────────────
# Mirrors app_mode_wpt_calc_temperature() in app_mode_wpt.c
#
# Circuit:  THERM_REF ──[49.9 kΩ sense]── THERM_OUT ──[NTC]── THERM_OFST
# NTC:      104AP-2, 10 kΩ @ 25 °C
#
# Lookup table from k_therm_table[] in firmware (resistance in Ω, temp in °C):
_THERM_TABLE = [
    (20, 126400),
    (25, 100000),
    (30,  79590),
    (40,  51320),
    (50,  33790),
]
_THERM_SENSE_OHM = 49900.0  # WPT_THERM_SENSE_RESISTOR_OHM


def calc_temperature(therm_ref_mv: float, therm_out_mv: float, therm_ofst_mv: float):
    """Return temperature in °C, or None if the values are invalid/unpowered."""
    v_therm = therm_out_mv - therm_ofst_mv      # voltage across NTC
    v_sense = therm_ref_mv - therm_out_mv        # voltage across sense resistor

    if v_sense <= 0.0 or v_therm <= 0.0:
        return None  # unpowered or invalid

    current    = v_sense / _THERM_SENSE_OHM      # mA (mV / Ω = mA — consistent)
    resistance = v_therm / current               # Ω

    # Clamp to table bounds
    if resistance >= _THERM_TABLE[0][1]:
        return float(_THERM_TABLE[0][0])
    if resistance <= _THERM_TABLE[-1][1]:
        return float(_THERM_TABLE[-1][0])

    # Linear interpolation between bracketing entries
    for i in range(len(_THERM_TABLE) - 1):
        t_lo, r_hi = _THERM_TABLE[i]       # higher resistance → lower temp
        t_hi, r_lo = _THERM_TABLE[i + 1]
        if r_lo <= resistance <= r_hi:
            frac = (r_hi - resistance) / (r_hi - r_lo)
            return t_lo + frac * (t_hi - t_lo)

    return None

# (bit_index, name, active_low, description)
GPIO_BITS = [
    (0,  "VRECT_DETn",    True,  "Wireless rectifier voltage detected"),
    (1,  "VRECT_OVPn",    True,  "Wireless rectifier over-voltage protection"),
    (2,  "VCHG_PGOOD",    False, "Charger power good"),
    (3,  "CHG1_STATUS",   False, "Charger 1 charging status"),
    (4,  "CHG1_OVP_ERRn", True,  "Charger 1 over-voltage error"),
    (5,  "CHG2_STATUS",   False, "Charger 2 charging status"),
    (6,  "CHG2_OVP_ERRn", True,  "Charger 2 over-voltage error"),
    (7,  "ENG2_SDNn",     True,  "Engine 2 shutdown (active-low enable)"),
    (8,  "ENG1_SDNn",     True,  "Engine 1 shutdown (active-low enable)"),
    (9,  "ECG_RLD",       False, "ECG right-leg drive"),
    (10, "ECG_HR_SDNn",   True,  "ECG heart-rate IC shutdown (active-low enable)"),
    (11, "ECG_RR_SDNn",   True,  "ECG resp-rate IC shutdown (active-low enable)"),
    (12, "TEMP_EN",       False, "Temperature measurement enabled"),
    (13, "IMP_EN",        False, "Impedance measurement enabled"),
]


def parse_msd(data: bytes) -> None:
    n = len(data)
    if n < 10:
        print(f"  [!] Only {n} bytes provided — need at least 10 to decode analog + GPIO fields.")

    def byte(i, default=None):
        if i < n:
            return data[i]
        return default

    print()
    print("=" * 60)
    print("  OpenNerve IPG Gen2 — MSD Decoded")
    print("=" * 60)

    # ── Analog measurements ──────────────────────────────────────
    print("\n  ── Analog Measurements ──────────────────────────────")

    dvdd = byte(0)
    if dvdd is not None:
        print(f"  [0] DVDD             : {dvdd:#04x}  →  {dvdd * 100:4d} mV  ({dvdd * 0.1:.2f} V)")

    battA = byte(1)
    if battA is not None:
        print(f"  [1] Battery A        : {battA:#04x}  →  {battA * 100:4d} mV  ({battA * 0.1:.2f} V)")

    battB = byte(2)
    if battB is not None:
        print(f"  [2] Battery B        : {battB:#04x}  →  {battB * 100:4d} mV  ({battB * 0.1:.2f} V)")

    impA = byte(3)
    if impA is not None:
        print(f"  [3] Impedance A      : {impA:#04x}  →  {impA * 10:4d} mV")

    impB = byte(4)
    if impB is not None:
        print(f"  [4] Impedance B      : {impB:#04x}  →  {impB * 10:4d} mV")

    tRef = byte(5)
    if tRef is not None:
        print(f"  [5] Therm Ref        : {tRef:#04x}  →  {tRef * 10:4d} mV")

    tOut = byte(6)
    if tOut is not None:
        print(f"  [6] Therm Out        : {tOut:#04x}  →  {tOut * 10:4d} mV")

    tOfst = byte(7)
    if tOfst is not None:
        print(f"  [7] Therm Offset     : {tOfst:#04x}  →  {tOfst * 10:4d} mV")

    if tRef is not None and tOut is not None and tOfst is not None:
        temp_c = calc_temperature(tRef * 10.0, tOut * 10.0, tOfst * 10.0)
        if temp_c is not None:
            print(f"      → Temperature    : {temp_c:.1f} °C  ({temp_c * 9/5 + 32:.1f} °F)")
        else:
            print(f"      → Temperature    : -- (thermistor unpowered or invalid voltages)")

    # ── GPIO flag bits ───────────────────────────────────────────
    print("\n  ── GPIO Flags  (bytes 8-9, 14 bits) ─────────────────")

    gpio_lo = byte(8, 0)
    gpio_hi = byte(9, 0)
    gpio_word = gpio_lo | (gpio_hi << 8)

    col_w = 18
    for bit_idx, name, active_low, desc in GPIO_BITS:
        raw = (gpio_word >> bit_idx) & 1
        # Logical state: active-low pins are asserted when raw = 0
        asserted = (raw == 0) if active_low else (raw == 1)
        indicator = "ACTIVE" if asserted else "inactive"
        al_tag    = " (active-low)" if active_low else ""
        print(f"  bit {bit_idx:>2}  {name:<18}{al_tag:<14}  raw={raw}  →  {indicator:<8}  {desc}")

    # ── BLE ID ──────────────────────────────────────────────────
    if n > 10:
        ble_id_end = min(n - 1, 23)   # byte 23 reserved for HW_VERSION
        ble_id = data[10:ble_id_end]
        if ble_id:
            print(f"\n  ── BLE ID  (bytes 10-{ble_id_end - 1}) ─────────────────────────────")
            hex_str = " ".join(f"{b:02x}" for b in ble_id)
            ascii_str = "".join(chr(b) if 32 <= b < 127 else "." for b in ble_id)
            print(f"  hex   : {hex_str}")
            print(f"  ascii : {ascii_str}")

    # ── HW Version ──────────────────────────────────────────────
    hw = byte(23)
    if hw is not None:
        print(f"\n  ── Hardware Version (byte 23) ────────────────────────")
        print(f"  HW_VERSION = {hw}  (0x{hw:02x})")

    print()
    print("=" * 60)
    print()


def normalize_hex(raw: str) -> bytes:
    """Accept space-separated, colon-separated, or plain hex strings."""
    cleaned = raw.strip().replace(":", "").replace(" ", "").replace("-", "")
    if len(cleaned) % 2 != 0:
        raise ValueError(f"Odd number of hex nibbles: '{raw}'")
    return bytes.fromhex(cleaned)


def main():
    banner = textwrap.dedent("""\
        ┌──────────────────────────────────────────────┐
        │  OpenNerve IPG Gen2  ·  MSD Parser           │
        │  Paste hex bytes then press Enter.           │
        │  Accepted formats:                           │
        │    AA BB CC ...   (space-separated)          │
        │    AA:BB:CC:...   (colon-separated)          │
        │    AABBCC...      (plain hex string)         │
        │  Type 'q' or Ctrl-C to quit.                 │
        └──────────────────────────────────────────────┘
    """)
    print(banner)

    # If hex data was passed as a command-line argument, parse it once and exit
    if len(sys.argv) > 1:
        raw = " ".join(sys.argv[1:])
        try:
            data = normalize_hex(raw)
            parse_msd(data)
        except Exception as e:
            print(f"  [!] Error: {e}")
        return

    while True:
        try:
            raw = input("  MSD hex > ").strip()
        except (EOFError, KeyboardInterrupt):
            print("\n  Bye.")
            break

        if not raw:
            continue
        if raw.lower() in ("q", "quit", "exit"):
            print("  Bye.")
            break

        try:
            data = normalize_hex(raw)
            print(f"  ({len(data)} bytes parsed)")
            parse_msd(data)
        except ValueError as e:
            print(f"  [!] {e}\n")


if __name__ == "__main__":
    main()
