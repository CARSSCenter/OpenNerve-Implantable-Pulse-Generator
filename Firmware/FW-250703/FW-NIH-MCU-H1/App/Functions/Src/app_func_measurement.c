/**
 * @file app_func_measurement.c
 * @brief This file provides the functionality of the measurement
 * @copyright Copyright (c) 2024
 */
#include "app_func_measurement.h"
#include "app_config.h"

/**
 * @brief Enable / Disable battery monitor
 * 
 * @param enable Enable / Disable
 */
void app_func_meas_batt_mon_enable(bool enable) {
	HAL_GPIO_WritePin(BATT_MON_EN_GPIO_Port, BATT_MON_EN_Pin, (enable)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
}

/**
 * @brief The battery monitor measures and obtains the battery voltages
 * 
 * @param p_vbatA The battery voltage of battery 1, unit: mV
 * @param p_vbatB The battery voltage of battery 2, unit: mV
 */
void app_func_meas_batt_mon_meas(uint16_t* p_vbatA, uint16_t* p_vbatB) {
	uint32_t vref = bsp_adc_vref_get(SYSCFG_VREFBUF_VOLTAGE_SCALE3);
	uint16_t vA[100], vB[100];
	bsp_adc_single_sampling(HANDLE_ID_ADC1, ADC1_CHANNEL_BATT_MON1, vA, 100, 100, vref);
	bsp_adc_single_sampling(HANDLE_ID_ADC1, ADC1_CHANNEL_BATT_MON2, vB, 100, 100, vref);
	uint32_t vAavg = 0;
	uint32_t vBavg = 0;
	for(uint8_t i=0;i<100;i++) {
		vAavg += vA[i];
		vBavg += vB[i];
	}
	vAavg /= 100;
	vBavg /= 100;
	p_vbatA[0] = vAavg * BSP_BATT_FACTOR;
	p_vbatB[0] = vBavg * BSP_BATT_FACTOR;
}

/**
 * @brief Enable / disable the impedance monitor.
 *
 * @param enable Enable / disable
 */
void app_func_meas_imp_enable(bool enable) {
	HAL_GPIO_WritePin(IMP_EN_GPIO_Port, IMP_EN_Pin, (enable)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
}

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
void app_func_meas_imp_sel_set(bool impa_sel0, bool impa_sel1, bool impa_sel2, bool impa_sel3, bool impb_sel0, bool impb_sel1, bool impb_sel2, bool impb_sel3) {
	HAL_GPIO_WritePin(IMPA_SEL_0_GPIO_Port, IMPA_SEL_0_Pin, (impa_sel0)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_GPIO_WritePin(IMPA_SEL_1_GPIO_Port, IMPA_SEL_1_Pin, (impa_sel1)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_GPIO_WritePin(IMPA_SEL_2_GPIO_Port, IMPA_SEL_2_Pin, (impa_sel2)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_GPIO_WritePin(IMPA_SEL_3_GPIO_Port, IMPA_SEL_3_Pin, (impa_sel3)?GPIO_PIN_RESET:GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */

	HAL_GPIO_WritePin(IMPB_SEL_0_GPIO_Port, IMPB_SEL_0_Pin, (impb_sel0)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_GPIO_WritePin(IMPB_SEL_1_GPIO_Port, IMPB_SEL_1_Pin, (impb_sel1)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_GPIO_WritePin(IMPB_SEL_2_GPIO_Port, IMPB_SEL_2_Pin, (impb_sel2)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_GPIO_WritePin(IMPB_SEL_3_GPIO_Port, IMPB_SEL_3_Pin, (impb_sel3)?GPIO_PIN_RESET:GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
}

/**
 * @brief The impedance monitor measures and obtains the voltage
 *
 * @param voltageBuffer Buffer to store sample voltage
 * @param samplingPoints Number of the sampling points
 * @param samplingFrequency_hz 	Sampling frequency of the Measurement
 */
void app_func_meas_imp_volt_meas(uint16_t voltageBuffer[], uint16_t samplingPoints, uint16_t samplingFrequency_hz) {
	uint32_t vref = bsp_adc_vref_get(SYSCFG_VREFBUF_VOLTAGE_SCALE3);
	bsp_adc_differential_sampling(HANDLE_ID_ADC4, ADC4_CHANNEL_IMP_INA, ADC4_CHANNEL_IMP_INB, voltageBuffer, samplingPoints, samplingFrequency_hz, vref);

	for(uint16_t i=0;i<samplingPoints;i++) {
		voltageBuffer[i] = (uint16_t)(voltageBuffer[i] * BSP_IMP_FACTOR);
	}
}

/**
 * @brief Enable / Disable sensor
 *
 * @param sensorID	The ID of sensor
 * @param enable	Enable / Disable
 */
void app_func_meas_sensor_enable(uint8_t sensorID, bool enable) {
	switch(sensorID)
	{
	case SENSOR_ID_ECG_HR:
	{
		HAL_GPIO_WritePin(ECG_HR_SDNn_GPIO_Port, ECG_HR_SDNn_Pin, (enable)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_GPIO_WritePin(ECG_RLD_GPIO_Port, ECG_RLD_Pin, (enable)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}	break;

	case SENSOR_ID_ECG_RR:
	{
		HAL_GPIO_WritePin(ECG_RR_SDNn_GPIO_Port, ECG_RR_SDNn_Pin, (enable)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_GPIO_WritePin(ECG_RLD_GPIO_Port, ECG_RLD_Pin, (enable)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}	break;

	case SENSOR_ID_EMG1:
	{
		HAL_GPIO_WritePin(EMG1_SDNn_GPIO_Port, EMG1_SDNn_Pin, (enable)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}	break;

	case SENSOR_ID_EMG2:
	{
		HAL_GPIO_WritePin(EMG2_SDNn_GPIO_Port, EMG2_SDNn_Pin, (enable)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}	break;

	default:
		break;
	}
}

/**
 * @brief Enable and set the multiplexer of the sensor
 *
 * @param sensorID	The ID of sensor
 */
void app_func_meas_sensor_mux_enable(uint8_t sensorID) {
	bsp_mux_enable(true);
	switch(sensorID)
	{
	case SENSOR_ID_ECG_HR:
	{
		bsp_mux_ecg_sel_set(SEL_ECG_HR);
	}	break;

	case SENSOR_ID_ECG_RR:
	{
		bsp_mux_ecg_sel_set(SEL_ECG_RR);
	}	break;

	case SENSOR_ID_EMG1:
	{

	}	break;

	case SENSOR_ID_EMG2_CH68:	//SENSOR_ID_EMG2 default
	{
		bsp_mux_sr_sel_set(SEL_SR_EMG2, SEL_SR_STIM);
	}	break;

	case SENSOR_ID_EMG2_CH57:
	{
		bsp_mux_sr_sel_set(SEL_SR_STIM, SEL_SR_EMG2);
	}	break;

	default:
		break;
	}
}

/**
 * @brief The sensor measures and obtains the voltages
 *
 * @param sensorID The ID of sensor
 * @param buff 	Data buffer for the voltage
 * @param bufferSize 	Data buffer size
 * @param samplingFrequency_hz 	Sampling frequency of the sensor. The minimum unit is 1Hz, and the range is 15 ~ 65535Hz
 */
void app_func_meas_sensor_meas(uint8_t sensorID, uint8_t* buff, uint8_t bufferSize, uint16_t samplingFrequency_hz) {
	uint16_t samplingPoints = bufferSize / sizeof(uint16_t);
	uint32_t vref;
	switch(sensorID)
	{
	case SENSOR_ID_ECG_HR:
		vref = bsp_adc_vref_get(SYSCFG_VREFBUF_VOLTAGE_SCALE3);
		bsp_adc_single_sampling(HANDLE_ID_ADC1, ADC1_CHANNEL_ECG_HR_OUT, (uint16_t*)buff, samplingPoints, samplingFrequency_hz, vref);
		break;

	case SENSOR_ID_ECG_RR:
		vref = bsp_adc_vref_get(SYSCFG_VREFBUF_VOLTAGE_SCALE3);
		bsp_adc_single_sampling(HANDLE_ID_ADC1, ADC1_CHANNEL_ECG_RR_OUT, (uint16_t*)buff, samplingPoints, samplingFrequency_hz, vref);
		break;

	case SENSOR_ID_EMG1:
		vref = bsp_adc_vref_get(SYSCFG_VREFBUF_VOLTAGE_SCALE3);
		bsp_adc_single_sampling(HANDLE_ID_ADC1, ADC1_CHANNEL_EMG1_OUT, (uint16_t*)buff, samplingPoints, samplingFrequency_hz, vref);
		break;

	case SENSOR_ID_EMG2:
		vref = bsp_adc_vref_get(SYSCFG_VREFBUF_VOLTAGE_SCALE3);
		bsp_adc_single_sampling(HANDLE_ID_ADC4, ADC4_CHANNEL_EMG2_OUT, (uint16_t*)buff, samplingPoints, samplingFrequency_hz, vref);
		break;

	default:
		break;
	}
}

/**
 * @brief The sensor measures and obtains the voltages and then continues sampling
 *
 * @param sensorID The ID of sensor
 * @param buff 	Data buffer for the voltage
 * @param bufferSize 	Data buffer size
 * @param samplingFrequency_hz 	Sampling frequency of the sensor. The minimum unit is 0.1Hz, and the range is 1.5 ~ 6553.5Hz
 */
void app_func_meas_sensor_sampling(uint8_t sensorID, uint8_t* buff, uint8_t bufferSize, float samplingFrequency_hz) {
	app_func_meas_sensor_meas(sensorID, buff, bufferSize, samplingFrequency_hz * 10.0);
	bsp_adc_sampling_continue_IT(10U);
}

/**
 * @brief Convert the sampled value into voltage and continue sampling.
 *
 */
void app_func_meas_sensor_continue(void) {
	bsp_adc_sampling_conversion(10U);
	bsp_adc_sampling_continue_IT(10U);
}

/**
 * @brief Enable / Disable vrect monitor
 *
 * @param enable Enable / Disable
 */
void app_func_meas_vrect_enable(bool enable) {
	HAL_GPIO_WritePin(VRECT_MON_EN_GPIO_Port, VRECT_MON_EN_Pin, (enable)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
}

/**
 * @brief The vrect monitor measures and obtains the voltage
 *
 * @param buff 	Data buffer for the voltage
 * @param bufferSize 	Data buffer size
 * @param samplingFrequency_hz 	Sampling frequency of the vrect monitor
 */
void app_func_meas_vrect_mon_meas(uint8_t* buff, uint8_t bufferSize, uint16_t samplingFrequency_hz) {
	uint16_t samplingPoints = bufferSize / sizeof(uint16_t);
	uint32_t vref = bsp_adc_vref_get(SYSCFG_VREFBUF_VOLTAGE_SCALE3);
	bsp_adc_single_sampling(HANDLE_ID_ADC4, ADC4_CHANNEL_VRECT_MON, (uint16_t*)buff, samplingPoints, samplingFrequency_hz, vref);
}

/**
 * @brief Enable / Disable thermistor
 *
 * @param enable Enable / Disable
 */
void app_func_meas_therm_enable(bool enable) {
	HAL_GPIO_WritePin(TEMP_EN_GPIO_Port, TEMP_EN_Pin, (enable)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
}

/**
 * @brief The thermistor measures and obtains the voltages
 *
 * @param thermID The ID of thermistor
 * @param buff 	Data buffer for the voltage
 * @param bufferSize 	Data buffer size
 * @param samplingFrequency_hz 	Sampling frequency of the thermistor
 */
void app_func_meas_therm_meas(uint8_t thermID, uint8_t* buff, uint8_t bufferSize, uint16_t samplingFrequency_hz) {
	uint16_t samplingPoints = bufferSize / sizeof(uint16_t);
	uint32_t vref;
	switch(thermID)
	{
	case THERM_ID_REF:
		vref = bsp_adc_vref_get(SYSCFG_VREFBUF_VOLTAGE_SCALE3);
		bsp_adc_single_sampling(HANDLE_ID_ADC4, ADC4_CHANNEL_THERM_REF, (uint16_t*)buff, samplingPoints, samplingFrequency_hz, vref);
		break;

	case THERM_ID_OUT:
		vref = bsp_adc_vref_get(SYSCFG_VREFBUF_VOLTAGE_SCALE3);
		bsp_adc_single_sampling(HANDLE_ID_ADC4, ADC4_CHANNEL_THERM_OUT, (uint16_t*)buff, samplingPoints, samplingFrequency_hz, vref);
		break;

	case THERM_ID_OFST:
		vref = bsp_adc_vref_get(SYSCFG_VREFBUF_VOLTAGE_SCALE3);
		bsp_adc_single_sampling(HANDLE_ID_ADC4, ADC4_CHANNEL_THERM_OFST, (uint16_t*)buff, samplingPoints, samplingFrequency_hz, vref);
		break;

	default:
		break;
	}
}

/**
 * @brief Turn off all peripheral circuits of measurement
 *
 */
void app_func_meas_off(void) {
	app_func_meas_batt_mon_enable(false);
	app_func_meas_imp_enable(false);
	app_func_meas_sensor_enable(SENSOR_ID_ECG_HR, false);
	app_func_meas_sensor_enable(SENSOR_ID_ECG_RR, false);
	app_func_meas_sensor_enable(SENSOR_ID_EMG1, false);
	app_func_meas_sensor_enable(SENSOR_ID_EMG2, false);
	app_func_meas_vrect_enable(false);
	app_func_meas_therm_enable(false);
}
