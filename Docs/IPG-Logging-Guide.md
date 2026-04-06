# IPG Logging Guide
OpenNerve Gen2
March 2026

---

## Overview

The main STM32U585 MCU maintains a persistent event log stored in the external FRAM chip. Logs survive power cycles and sleep, are timestamped via the RTC, and can be read back over BLE. The nRF52810 BLE chip has no persistent logging — its debug output only appears on a live J-Link RTT connection during development.

---

## Storage

Logs live in the FRAM (CY15B108QN) at:

| Region | Address | Size |
|---|---|---|
| Log data | `0x00000` | 128 KB |
| Log write pointer | `0x20000` | 4 bytes |

The log is a **circular buffer**. When it fills, the write pointer wraps to `0x00000` and old entries are silently overwritten. There is no overflow flag or warning.

---

## Log Entry Format

Every entry is a human-readable ASCII string terminated by `\r\n`:

```
[20YY-MM-DDThh:mm:ssZ(uuu)]<TYPE>DATA\r\n
```

- Timestamp is UTC from the RTC. `uuu` is sub-second resolution (units of 1/255 s, so 000–255).
- `<TYPE>` is one of four 4-character tags (including the angle brackets).

### Data types

| Tag | Meaning | Example data |
|---|---|---|
| `<EV>` | Event | `ER   ` |
| `<BA>` | Battery voltage | `A=3.7V, B=3.6V` |
| `<IM>` | Impedance measurement | `12,500ohm` |
| `<PA>` | Parameter change | `(HSPID)Val=00010.0` |

### Full example entries
```
[2026-03-15T14:32:05Z(128)]<EV>ER   \r\n
[2026-03-15T14:32:06Z(012)]<BA>A=3.7V, B=3.6V\r\n
[2026-03-15T14:32:10Z(200)]<IM>12,500ohm\r\n
[2026-03-15T14:32:11Z(005)]<PA>(HSPID)Val=00010.0\r\n
```

---

## Event Types (`<EV>`)

Event label strings are exactly 5 bytes (padded with spaces):

| Label | Constant | Trigger |
|---|---|---|
| `PO   ` | `EVENT_POWER_ON` | MCU powered on and fully initialized (written once per clean boot, not on wakeup from sleep) |
| `ER   ` | `EVENT_ER` | Battery voltage has been in the Early Replacement range for 3 consecutive checks |
| `EOS  ` | `EVENT_EOS` | Battery voltage has been at End of Service level for 3 consecutive checks; device transitions to sleep immediately after, then pulls `BATT_SW_EN` LOW to discharge the battery-disconnect timing capacitor (~1–2 min to full disconnect) |
| `UF   ` | `EVENT_UNRESPONSIVE_FUNCTION` | Watchdog timer fired — written on the **next boot** by checking the reset-cause register |
| `MD   ` | `EVENT_MAGNET_DETECTION` | Magnet was held and removed for the configured duration (triggers sleep↔wake transition) |
| `SC   ` | `EVENT_SHORT_CIRCUIT` | Short circuit detected during impedance measurement |
| `HI   ` | `EVENT_HIGH_IMPED` | High impedance detected during impedance measurement |
| `NI   ` | `EVENT_NORMAL_IMPED` | Normal impedance confirmed during impedance measurement |
| `LSA  ` | `EVENT_LOWER_STIM_AMP` | Commanded stimulation amplitude exceeded `MAX_SAFE_AMPLITUDE`; was clamped down automatically |
| `SS   ` | `EVENT_STIM_START` | Stimulation session started (scheduled or manual) |
| `SE   ` | `EVENT_STIM_STOP` | Stimulation session stopped (scheduled end, manual BLE command, or forced stop on sleep/shutdown) |
| `BC   ` | `EVENT_BLE_CONNECT` | BLE client authenticated and connected (auth opcode accepted) |
| `BD   ` | `EVENT_BLE_DISCONNECT` | BLE client disconnected (idle timeout, explicit disconnect request, or hardware-side link loss) |
| `SL   ` | `EVENT_SLEEP` | Device entering sleep (low-power STOP) state |
| `WK   ` | `EVENT_WAKEUP` | Device woken from sleep and peripherals re-initialized |
| `SD   ` | `EVENT_SHUTDOWN` | Intentional shutdown commanded via BLE (`OP_SHUTDOWN_SYSTEM`). Does not appear for EOS shutdowns — those are logged as `EOS  ` instead. |

> **Note:** `OC   ` (open circuit) is defined in the header but has no call site — it is never written by current firmware.

### What is NOT currently logged

- OTA firmware update completion

---

## Reading Logs Over BLE

Both read and erase commands require an active **Admin-authenticated** BLE session (opcode `0xF0`).

### OP_READ_IPG_LOG — `0xAE`

Returns one log entry per call. The entry returned is the **first entry with a timestamp strictly greater than** the timestamp you send.

**Request payload (7 bytes):**

| Byte | Field | Description |
|---|---|---|
| 0 | Year | 2-digit year, e.g. `26` for 2026 |
| 1 | Month | 1–12 |
| 2 | Day | 1–31 |
| 3 | Hour | 0–23 |
| 4 | Minute | 0–59 |
| 5 | Second | 0–59 |
| 6 | Sub-second | 0–255 (units of 1/255 s) |

**Response:**
- `STATUS_SUCCESS` + payload containing the raw ASCII log entry bytes (including the timestamp prefix and `\r\n`)
- `STATUS_SUCCESS` with payload length 0 means no more entries exist after that timestamp — download is complete

**To dump the full log**, start with all-zeros timestamp and loop, using the timestamp from each returned entry as the input for the next call:

```csharp
byte[] timestamp = new byte[7]; // all zeros = "before everything"

while (true) {
    var response = SendCommand(0xAE, timestamp);
    if (response.Status != STATUS_SUCCESS || response.PayloadLength == 0) break;

    string entry = Encoding.ASCII.GetString(response.Payload, 0, response.PayloadLength);
    Console.WriteLine(entry);

    // Parse the timestamp out of the returned entry for the next request
    // Format: [20YY-MM-DDThh:mm:ssZ(uuu)]
    timestamp = ParseTimestamp(entry);
}
```

The firmware remembers the last-read address internally, so repeated calls with the same timestamp will return the same entry — it does not auto-advance. You must always pass the timestamp of the entry you just received to get the next one.

### OP_ERASE_IPG_LOG — `0xAF`

No payload. Erases all log data and resets the write pointer to `0x00000`.

```csharp
SendCommand(0xAF, new byte[0]);
```

Use with caution — this is irreversible.

---

## Setting the Real-Time Clock

Accurate log timestamps require the RTC to be set before logs are collected. Both commands require an active **Admin-authenticated** BLE session.

### OP_WRITE_TIME_AND_DATE — `0xB1`

Sets the RTC to the specified date and time. All fields are plain binary integers (not BCD).

**Request payload (6 bytes):**

| Byte | Field | Range | Notes |
|---|---|---|---|
| 0 | Year | 0–99 | 2-digit year offset from 2000, e.g. `26` for 2026 |
| 1 | Month | 1–12 | |
| 2 | Day | 1–31 | |
| 3 | Hour | 0–23 | UTC |
| 4 | Minute | 0–59 | |
| 5 | Second | 0–59 | |

The firmware validates all fields before applying. Any out-of-range value returns `STATUS_INVALID` and the RTC is not changed.

**Example — setting 2026-03-24 15:30:00 UTC:**
```csharp
SendCommand(0xB1, new byte[] { 26, 3, 24, 15, 30, 0 });
```

**Practical note:** To minimize clock error, capture the target time in your software immediately before sending the command. Round to the nearest second and account for BLE round-trip latency if sub-second accuracy matters.

---

### OP_READ_TIME_AND_DATE — `0xB0`

Reads the current RTC value. No request payload.

**Response payload (6 bytes):** Same layout as the write command — Year, Month, Day, Hour, Minute, Second.

Use this to verify the time was set correctly:
```csharp
var resp = SendCommand(0xB0, new byte[0]);
// resp.Payload: [YY, MM, DD, hh, mm, ss]
Console.WriteLine($"20{resp.Payload[0]:D2}-{resp.Payload[1]:D2}-{resp.Payload[2]:D2}T{resp.Payload[3]:D2}:{resp.Payload[4]:D2}:{resp.Payload[5]:D2}Z");
```

---

## Timestamp Parsing

The timestamp in a log entry looks like:

```
[2026-03-15T14:32:05Z(128)]
```

To extract the 7-byte timestamp for the next `OP_READ_IPG_LOG` request:

```csharp
// Input: "[2026-03-15T14:32:05Z(128)]..."
// Fields: [20YY-MM-DDThh:mm:ssZ(uuu)]
byte[] ParseTimestamp(string entry) {
    // entry[3..4]   = YY
    // entry[6..7]   = MM
    // entry[9..10]  = DD
    // entry[12..13] = hh
    // entry[15..16] = mm
    // entry[18..19] = ss
    // entry[22..24] = uuu
    return new byte[] {
        byte.Parse(entry.Substring(3, 2)),   // YY
        byte.Parse(entry.Substring(6, 2)),   // MM
        byte.Parse(entry.Substring(9, 2)),   // DD
        byte.Parse(entry.Substring(12, 2)),  // hh
        byte.Parse(entry.Substring(15, 2)),  // mm
        byte.Parse(entry.Substring(18, 2)),  // ss
        byte.Parse(entry.Substring(22, 3)),  // uuu
    };
}
```

---

## Relevant Source Files

| File | Description |
|---|---|
| `App/Functions/Src/app_func_logs.c` | All log read/write/erase logic |
| `App/Functions/Inc/app_func_logs.h` | Event label constants and function prototypes |
| `App/Bsp/Inc/bsp_fram.h` | FRAM address map (`ADDR_LOG_BASE`, `SIZE_LOG`, etc.) |
| `App/Src/app.c` | Writes `PO` event on boot |
| `App/Src/app_state.c` | Writes `SL` (sleep entry), `WK` (wakeup), and `SE` (stim stop on forced power-off) events |
| `App/Src/app_mode_battery_test.c` | Writes `<BA>` and `ER` events; writes `EOS` during sleep-based battery checks |
| `App/Functions/Src/app_func_state_machine.c` (continued) | Also writes `EOS` event via `app_func_sm_active_eos_check()` — the active-mode EOS path that fires once per minute during BLE, connection, and therapy states |
| `App/Src/app_mode_impedance_test.c` | Writes `<IM>`, `SC`, `HI`, `NI` events |
| `App/Src/app_mode_therapy_session.c` | Writes `LSA`, `SS`, `SE` events |
| `App/Src/app_mode_ble_active.c` | Writes `BC` event on successful BLE authentication |
| `App/Src/app_mode_ble_connection.c` | Handles `OP_READ_IPG_LOG` and `OP_ERASE_IPG_LOG` opcodes; writes `<PA>` entries on parameter changes; writes `BD` (BLE disconnect) and `SD` (shutdown) events |
| `App/Functions/Src/app_func_state_machine.c` | Writes `UF` (watchdog reset) and `MD` (magnet) events |
| `App/Functions/Src/app_func_ble.c` | Also writes `MD` event on magnet detection |
