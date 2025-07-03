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
#define	SENSOR_ID_EMG1		3
#define	SENSOR_ID_EMG2		4

#define	SENSOR_ID_EMG2_CH68	SENSOR_ID_EMG2
#define	SENSOR_ID_EMG2_CH57	5

#define	THERM_ID_REF		1
#define	THERM_ID_OUT		2
#define	THERM_ID_OFST		3

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
 * @param impa_sel0 Select multiplexer A input 0
 * @param impa_sel1 Select multiplexer A input 1
 * @param impa_sel2 Select multiplexer A input 2
 * @param impa_sel3 Select multiplexer A input 3
 * @param impb_sel0 Select multiplexer B input 0
 * @param impb_sel1 Select multiplexer B input 1
 * @param impb_sel2 Select multiplexer B input 2
 * @param impb_sel3 Select multiplexer B input 3
 */
void app_func_meas_imp_sel_set(bool impa_sel0, bool impa_sel1, bool impa_sel2, bool impa_sel3, bool impb_sel0, bool impb_sel1, bool impb_sel2, bool impb_sel3);

/**
 * @brief The impedance monitor measures and obtains the voltage
 *
 * @param voltageBuffer Buffer to store sample voltage
 * @param samplingPoints Number of the sampling points
 * @param samplingFrequency_hz 	Sampling frequency of the Measurement
 */
void app_func_meas_imp_volt_meas(uint16_t voltageBuffer[], uint16_t samplingPoints, uint16_t samplingFrequency_hz);

/**
 * @brief Enable / Disable sensor
 *
 * @param sensorID	The ID of sensor
 * @param enable	Enable / Disable
 */
void app_func_meas_sensor_enable(uint8_t sensorID, bool enable);

/**
 * @brief Enable and set the multiplexer of the sensor
 *
 * @param sensorID	The ID of sensor
 */
void app_func_meas_sensor_mux_enable(uint8_t sensorID);

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
