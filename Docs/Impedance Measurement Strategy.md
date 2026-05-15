# Impedance Measurement Strategy

This document outlines the actions required by the IPG firmware to implement impedance measurement.

**Document Title:** Impedance Measurement Strategy  
**Version:** 1.0  
**Date:** 2024-11-05

---

## 1. Circuit Overview and Signals

The stimulus stage generates a current pulse on any of the eight channels or the enclosure. The measurement circuit then acquires the voltage between the selected electrode pairs.

The stimulus generation is set by the pulse guard circuit and the DAC which controls the voltage control current source and it drives the current pulse to the electrodes or enclosure.

The impedance measurement circuit (Figure 1) consists of three stages:

- **Supply Control:** This circuit enables the 3V3 power rail, supplying power to the entire IMC.
- **Channel Selection:** This circuit selects the measurement nodes among the eight channels and the enclosure.
- **Amplification Circuits:** The selected nodes are fed into an amplification circuit, where a fully differential amplifier with a gain of 7.20 V/V amplifies the differential voltage between them. The outputs, `IMP_INA` and `IMP_INB`, connect to the microcontroller's ADC pins.

> **Figure 1.** Impedance measurement circuit
<img width="1024" height="439" alt="Screenshot 2026-05-15 at 10 00 58 AM" src="https://github.com/user-attachments/assets/d11d8400-e49b-4e49-a8ca-3aac0d05a473" />


> **Figure 2.** Input high pass filter circuit
<img width="609" height="560" alt="Screenshot 2026-05-15 at 10 01 14 AM" src="https://github.com/user-attachments/assets/ac9216cc-a74e-4694-8421-f5050fb63de8" />


The signals involved in this process are described in Table 1 below.

### Table 1: Signal Description

| Label | Type | Description |
|-------|------|-------------|
| **Stimulus generation** | | |
| VPP | Power | Stimulation power rail. |
| VDDS | Power | Digital power rail, used in the stimulus pulse generation. |
| STIM_EN | Input | Power up the current source. |
| I2C2 | Input | I2C bus used to control the DAC80502DRXT. |
| PG_START | Input | Pulse Guard set. |
| AMPL_RSTn | Input | Pulse Guard set. |
| PH1_EN | Input | Pulse Guard set. |
| PH2_EN | Input | Pulse Guard set. |
| SRC[1..8] | Input | Stimulation source channel selection. |
| SNK[1..8] | Input | Stimulation sink channel selection. |
| SRC17 | Input | Enclosure as stimulation source selection. |
| SNK17 | Input | Enclosure as stimulation sink selection. |
| **Voltage measurement** | | |
| VDDS | Power | Digital power rail, used to enable the measurement circuit and drive VIMP_3V3. |
| VIMP_3V3 | Power | 3V3 power rail enabled by the IMP_EN signal. Used to supply the measurement circuit. |
| IMP_EN | Input | Impedance measurement enable. |
| IMPA_SEL | Input | Node A of voltage amplification. |
| IMB_SEL | Input | Node B of voltage amplification. |
| IMP_INA | Output | Node A of voltage measure. |
| IMP_INB | Output | Node B of voltage measure. |

---

## 2. Stimulation Test Signal Characteristics

The stimulation signal used for impedance measurement must be biphasic, with a minimum pulse width of **0.5 ms** and a maximum current of **1 mA**.

---

## 3. Measurement Algorithm

The impedance measurement algorithm consists of three main steps: stimulation, measurement, and impedance calculation.

1. **Stimulation:**
   The load is stimulated using a biphasic signal with the specified pulse width, current amplitude, and other defined characteristics. This signal generates a current pulse of amplitude *Ip* through the load.

2. **Measurement:**
   The voltage across the load is obtained by measuring the absolute difference between the IMC outputs, `IMP_INA` and `IMP_INB`. This differential voltage signal is analyzed to identify relative maxima that correspond to positive stimulation pulses (excluding maxima caused by load-balancing pulses). The relevant maxima are then averaged over the number of pulses applied during stimulation, yielding the required measurement value *m*.

3. **Impedance Calculation:**
   Once the measurement *m* is obtained, it is adjusted to reflect the actual load voltage. This adjustment is performed by dividing *m* by the amplifier gain (7.2) and then multiplying the result by the attenuation factor of the high-pass filter (6). Finally, this adjusted voltage is divided by the stimulation current *Ip* to calculate the load impedance *Z_load*, as shown in Equation 1:

$$|Z_{load}| = \frac{m \times 6}{7.2 \times I_p} \tag{1}$$

> **Figure 3.** IMC output differential voltage (`IMP_INA – IMP_INB`)
<img width="877" height="373" alt="Screenshot 2026-05-15 at 10 01 43 AM" src="https://github.com/user-attachments/assets/75864727-17a0-44ff-b08f-d1c8a3919575" />


> **Figure 4.** Absolute value of IMC output differential voltage (`IMP_INA – IMP_INB`) and its maxima
<img width="883" height="353" alt="Screenshot 2026-05-15 at 10 01 54 AM" src="https://github.com/user-attachments/assets/5bf7b8d8-c8d1-46bc-a1e8-8da77fc85a15" />

