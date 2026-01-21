/**
 * @file app_func_measurement.h
 * @brief This file contains all the function prototypes for the app_func_measurement.c file
 * @copyright Copyright (c) 2024
 */
#ifndef FUNCTIONS_INC_APP_FUNC_MEASUREMENT_H_
#define FUNCTIONS_INC_APP_FUNC_MEASUREMENT_H_
#include <stdint.h>
#include <stdbool.h>

#define	SENSOR_ID_IDLE		0
#define	SENSOR_ID_ECG_HR	1
#define	SENSOR_ID_ECG_RR	2
#define	SENSOR_ID_ENG1		3
#define	SENSOR_ID_ENG2		4

#define	IMPIN_N_CH1			0
#define	IMPIN_N_CH2			1
#define	IMPIN_N_CH3			2
#define	IMPIN_N_CH4			3
#define	IMPIN_N_ENCL		4

#define	IMPIN_P_STIMA		0
#define	IMPIN_P_STIMB		1

#define	THERM_ID_REF		1
#define	THERM_ID_OUT		2
#define	THERM_ID_OFST		3

#define	IMPIN_CH_P			ADC4_CHANNEL_IMP_INA
#define	IMPIN_CH_N			ADC4_CHANNEL_IMP_INB

/**
 * @brief Enable / Disable battery monitor
 * 
 * @param enable Enable / Disable
 */
void app_func_meas_batt_mon_enable(bool enable);

/**
 * @brief The battery monitor measures and obtains the battery voltages
 * 
 * @param p_vbatA The battery voltage of battery 1, unit: mV
 * @param p_vbatB The battery voltage of battery 2, unit: mV
 */
void app_func_meas_batt_mon_meas(uint16_t* p_vbatA, uint16_t* p_vbatB);

/**
 * @brief Enable / disable the impedance monitor.
 *
 * @param enable Enable / disable
 */
void app_func_meas_imp_enable(bool enable);

/**
 * @brief Set the channels of the impedance monitor.
 *
 * @param impin_n_sel0 Select multiplexer U900 input X(SINK_CH1(LL)/CH2(LH)/CH3(HL)/CH4(HH)) to IMP_SINK
 * @param impin_n_sel1 Select multiplexer U900 input X(SINK_CH1(LL)/CH2(LH)/CH3(HL)/CH4(HH)) to IMP_SINK
 * @param impin_n_sel2 Select multiplexer U901 input X(IMP_SINK(L)/SINK_ENCL(H)) to VIMC_IN-
 * @param impin_p_sel Select multiplexer U901 input Y(STIMA(L)/STIMB(H)) to VIMC_IN+
 */
void app_func_meas_imp_sel_set(bool impin_n_sel0, bool impin_n_sel1, bool impin_n_sel2, bool impin_p_sel);

/**
 * @brief Calculate the required ADC sampling points
 *
 * @param samplingFrequency_hz Sampling frequency of the ADC in Hz
 * @param samplingTime_us Sampling duration in µs
 *
 * @return uint16_t Number of sampling points (in samples) required to cover the given sampling time
 */
uint16_t app_func_meas_imp_sampPoints_get(uint32_t samplingFrequency_hz, uint32_t samplingTime_us);

/**
 * @brief The impedance monitor measures and obtains the voltage
 *
 * @param channel The channel to sample, IMP_OUT+ or IMP_OUT-
 * @param voltageBuffer Buffer for storing the sampled voltage from IMP_OUT+ or IMP_OUT-
 * @param samplingPoints Number of the sampling points
 * @param samplingFrequency_hz 	Sampling frequency of the Measurement
 */
void app_func_meas_imp_volt_meas(uint32_t channel, uint16_t voltageBuffer[], uint16_t samplingPoints, uint16_t samplingFrequency_hz);

/**
 * @brief Calculate the differential load voltage in mV.
 *
 * @param voltageBuffer Pointer to the ADC sampling buffer for IMP_OUT+ or IMP_OUT-
 * @param periodPoints Total number of sampling points acquired for one full
 *                       positive-negative pulse period.
 * @param widthPoints Number of sampling points used in the calculation,
 *                   corresponding to the pulse width.
 *
 * @return _Float64 Differential load voltage in mV
 */
_Float64 app_func_meas_imp_volt_calc(uint16_t voltageBuffer[], uint16_t periodPoints, uint16_t widthPoints);

/**
 * @brief Calculate the resistance of the load
 *
 * @param vdac_mV The output voltage of the DAC that generates the current IREF
 * @param vload_mV The voltage difference across the load
 * @return _Float64 The resistance of the load
 */
_Float64 app_func_meas_imp_calc(_Float64 vdac_mV, _Float64 vload_mV);

/**
 * @brief Enable / Disable VDDA supply
 *
 * @param enable Enable / Disable
 */
void app_func_meas_vdda_sup_enable(bool enable);

/**
 * @brief Enable / Disable sensor
 *
 * @param sensorID	The ID of sensor
 * @param enable	Enable / Disable
 */
void app_func_meas_sensor_enable(uint8_t sensorID, bool enable);

/**
 * @brief The sensor measures and obtains the voltages
 *
 * @param sensorID The ID of sensor
 * @param buff 	Data buffer for the voltage
 * @param bufferSize 	Data buffer size
 * @param samplingFrequency_hz 	Sampling frequency of the sensor. The minimum unit is 1Hz, and the range is 15 ~ 65535Hz
 */
void app_func_meas_sensor_meas(uint8_t sensorID, uint8_t* buff, uint8_t bufferSize, uint16_t samplingFrequency_hz);

/**
 * @brief The sensor measures and obtains the voltages and then continues sampling
 *
 * @param sensorID The ID of sensor
 * @param buff 	Data buffer for the voltage
 * @param bufferSize 	Data buffer size
 * @param samplingFrequency_hz 	Sampling frequency of the sensor. The minimum unit is 0.1Hz, and the range is 1.5 ~ 6553.5Hz
 */
void app_func_meas_sensor_sampling(uint8_t sensorID, uint8_t* buff, uint8_t bufferSize, float samplingFrequency_hz);

/**
 * @brief Convert the sampled value into voltage and continue sampling.
 *
 */
void app_func_meas_sensor_continue(void);

/**
 * @brief Enable / Disable vrect monitor
 *
 * @param enable Enable / Disable
 */
void app_func_meas_vrect_enable(bool enable);

/**
 * @brief The vrect monitor measures and obtains the voltage
 *
 * @param buff 	Data buffer for the voltage
 * @param bufferSize 	Data buffer size
 * @param samplingFrequency_hz 	Sampling frequency of the vrect monitor
 */
void app_func_meas_vrect_mon_meas(uint8_t* buff, uint8_t bufferSize, uint16_t samplingFrequency_hz);

/**
 * @brief Enable / Disable thermistor
 *
 * @param enable Enable / Disable
 */
void app_func_meas_therm_enable(bool enable);

/**
 * @brief The thermistor measures and obtains the voltages
 *
 * @param thermID The ID of thermistor
 * @param buff 	Data buffer for the voltage
 * @param bufferSize 	Data buffer size
 * @param samplingFrequency_hz 	Sampling frequency of the thermistor
 */
void app_func_meas_therm_meas(uint8_t thermID, uint8_t* buff, uint8_t bufferSize, uint16_t samplingFrequency_hz);

/**
 * @brief Turn off all peripheral circuits of measurement
 *
 */
void app_func_meas_off(void);

#endif /* FUNCTIONS_INC_APP_FUNC_MEASUREMENT_H_ */
