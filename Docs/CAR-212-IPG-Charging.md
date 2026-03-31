# CAR-212 IPG Charging requirements
V0.1
Alex Baldwin and Victor Pikov
3/9/2026

## State machine transitions
1.    When External Coil is detected (VRECT.DETn = low), the IPG shall transition to Active State, BLE Active, WPT High mode from any other mode, including: Sleep, Therapy Session, Active State BLE Connection, ER, EOS, Shutdown.
2.    When in WPT High mode, the IPG shall transition to WPT Paused mode when CHGx.OVP_ERRn = low (battery voltage >= 4.277V) for either battery, or Thermistor Level > 42C, or power transfer exceeds the overpotential threshold (VRECT.OVPn = low, around 17V on VRECT). In this mode, VCHG_DISABLE = high, CHGx.EN = low
3.    When in WPT Paused mode, the IPG shall transition to WPT High when Battery Level < CHGx_OVP (4.1V) for all present batteries AND Thermistor Level < 42C AND VRECT.OVPn = high.
4.    CHGx.STATUS shall be configured as an input with an internal pullup. When one battery’s CHGx.STATUS pin goes high, it indicates that this battery is done charging. When both go high, the system shall transition from WPT High mode to WPT Paused mode.
5.    When External Coil is no longer detected (VRECT.DETn = high), the IPG shall transition to Active State, BLE Active mode (VCHG_DISABLE = high, CHG1.EN = low, CHG2.EN = low) from either WPT High or WPT Paused mode.

## Initial States
### Active State, BLE Active (charger not present) – how things initialize
•    VRECT.MON_EN = low
•    CHG1.EN and CHG2.EN = low
•    VCHG_DISABLE = high
•    CHG_RATE1 = low, CHG_RATE2 = high (set to 50mA charging, fine for now but add feedback in future)
### Active State, BLE Active, WPT High
•    VRECT.MON_EN = high
•    CHG1.EN = high and/or CHG2.EN = high
•    VCHG_DISABLE = low
### Active State, BLE Active, WPT Paused
•    CHG1.EN = low, CHG2.EN = low
•    VCHG_DISABLE = high

## Other requirements for WPT Low mode and WPT High modes
8.    When in WPT High mode or WPT Paused mode, the IPG shall not enter the Sleep mode
9.    Continue updating BLE advertisement once per second with Battery Level, Thermistor Level, and WPT Mode status accurately included
10.    IPG shall set charge rates of batteries using CHG_RATEn. For now, that shall be set to 50mA. To set the charging rate to 50mA, CHG_RATE1 shall be low and CHG_RATE2 shall be high.
11.    IPG shall charge each battery independently, and allow charging of any one battery if the other battery is not present or is fully charged
a.    Firmware shall maintain a record of which of the two battery ports has a battery attached, and what that battery’s charge status is. IPGs may be configured with one or two batteries, and if configured with one battery it may be attached to either the first or second battery port.
b.    If CHGx_Status = high impedance (measured as high with MCU pin as input pullup), set CHGx.EN = low for that . If CHGx.Status = low for all present batteries, then transition to WPT Paused mode
c.    If Battery n Level < 1V for two consecutive measurements, set Battery n as absent: CHGx_STATUS = high and CHGx_EN = low




