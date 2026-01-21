/**
 * @file app_func_measurement.c
 * @brief This file provides the functionality of the measurement
 * @copyright Copyright (c) 2024
 */
#include "app_func_measurement.h"
#include "app_config.h"

#include <math.h>
#include <stdlib.h>

uint16_t battA[100];
uint16_t battB[100];

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
	bsp_adc_single_sampling(HANDLE_ID_ADC1, ADC1_CHANNEL_BATT_MON1, battA, 100, 100);
	bsp_adc_single_sampling(HANDLE_ID_ADC1, ADC1_CHANNEL_BATT_MON2, battB, 100, 100);
	uint32_t vAavg = 0;
	uint32_t vBavg = 0;
	for(uint8_t i=0;i<100;i++) {
		vAavg += battA[i];
		vBavg += battB[i];
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
 * @param impin_n_sel0 Select multiplexer U900 input X(SINK_CH1(LL)/CH2(LH)/CH3(HL)/CH4(HH)) to IMP_SINK
 * @param impin_n_sel1 Select multiplexer U900 input X(SINK_CH1(LL)/CH2(LH)/CH3(HL)/CH4(HH)) to IMP_SINK
 * @param impin_n_sel2 Select multiplexer U901 input X(IMP_SINK(L)/SINK_ENCL(H)) to VIMC_IN-
 * @param impin_p_sel Select multiplexer U901 input Y(STIMA(L)/STIMB(H)) to VIMC_IN+
 */
void app_func_meas_imp_sel_set(bool impin_n_sel0, bool impin_n_sel1, bool impin_n_sel2, bool impin_p_sel) {
	HAL_GPIO_WritePin(IMP_IN_N_SEL0_GPIO_Port, IMP_IN_N_SEL0_Pin, (impin_n_sel0)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_GPIO_WritePin(IMP_IN_N_SEL1_GPIO_Port, IMP_IN_N_SEL1_Pin, (impin_n_sel1)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_GPIO_WritePin(IMP_IN_N_SEL2_GPIO_Port, IMP_IN_N_SEL2_Pin, (impin_n_sel2)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_GPIO_WritePin(IMP_IN_P_SEL_GPIO_Port, IMP_IN_P_SEL_Pin, (impin_p_sel)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
}

/**
 * @brief Calculate the required ADC sampling points
 *
 * @param samplingFrequency_hz Sampling frequency of the ADC in Hz
 * @param samplingTime_us Sampling duration in µs
 *
 * @return uint16_t Number of sampling points (in samples) required to cover the given sampling time
 */
uint16_t app_func_meas_imp_sampPoints_get(uint32_t samplingFrequency_hz, uint32_t samplingTime_us) {
	_Float64 samplingTime_s = samplingTime_us * 1e-6;

    uint32_t samplingPoints = (uint32_t)(samplingFrequency_hz * samplingTime_s + 0.5);

    if (samplingPoints == 0)
    	samplingPoints = 1;
    else if (samplingPoints > ADC_MAX_SAMPLE_POINTS)
    	samplingPoints = ADC_MAX_SAMPLE_POINTS;

    return samplingPoints;
}

/**
 * @brief The impedance monitor measures and obtains the voltage
 *
 * @param channel The channel to sample, IMP_OUT+ or IMP_OUT-
 * @param voltageBuffer Buffer for storing the sampled voltage from IMP_OUT+ or IMP_OUT-
 * @param samplingPoints Number of the sampling points
 * @param samplingFrequency_hz 	Sampling frequency of the Measurement
 */
void app_func_meas_imp_volt_meas(uint32_t channel, uint16_t voltageBuffer[], uint16_t samplingPoints, uint16_t samplingFrequency_hz) {
	bsp_adc_single_sampling(HANDLE_ID_ADC4, channel, voltageBuffer, samplingPoints, samplingFrequency_hz);
}

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
_Float64 app_func_meas_imp_volt_calc(uint16_t voltageBuffer[], uint16_t periodPoints, uint16_t widthPoints) {
	uint16_t* p_buff = (uint16_t*)voltageBuffer;
	_Float64 impVolt = 0;

	uint8_t periodPulses = 2;
	uint16_t halfPeriodPoints = periodPoints / 2;
	uint16_t halfWidthPoints = widthPoints / 2;
	uint16_t startPoint, stopPoint;
	uint16_t maxV, minV;

	for (uint8_t pulse = 0;pulse < periodPulses;pulse++) {
		maxV = 0;
		minV = 0xFFFF;
		startPoint = halfWidthPoints + (halfPeriodPoints * pulse);
		stopPoint = startPoint + (halfPeriodPoints / 2);

		for(uint16_t i = startPoint; i < stopPoint; i++) {
			maxV = (maxV > p_buff[i])?maxV:p_buff[i];
			minV = (minV < p_buff[i])?minV:p_buff[i];
		}
		impVolt += (_Float64)(maxV - minV);
	}
	impVolt = impVolt / (_Float64)periodPulses;

	return (impVolt * BSP_IMP_FACTOR);
}

/**
 * @brief Calculate the resistance of the load
 *
 * @param vdac_mV The output voltage of the DAC that generates the current IREF
 * @param vload_mV The voltage difference across the load
 * @return _Float64 The resistance of the load
 */
_Float64 app_func_meas_imp_calc(_Float64 vdac_mV, _Float64 vload_mV) {
	_Float64 iref = vdac_mV / 1000.0f / BSP_STIM_RREF;
    if (iref <= 0.0f)
    	return 0.0f;

    _Float64 iout = iref * (BSP_MIRROR_RREF / BSP_MIRROR_ROUT);

    for (uint8_t iter = 0; iter < 20; iter++) {
    	_Float64 f  = BSP_VT * logf(iref / iout) - (iout * BSP_MIRROR_ROUT - iref * BSP_MIRROR_RREF);
    	_Float64 df = -(BSP_VT / iout) - BSP_MIRROR_ROUT;
    	_Float64 delta = f / df;
        iout -= delta;

        if (fabsf(delta) < 1e-12f)
        	break;

        if (iout <= 0.0f) {
            iout = 1e-12f;
            break;
        }
    }

    return (vload_mV / (iout * 1000.0f));
}

/**
 * @brief Enable / Disable VDDA supply
 *
 * @param enable Enable / Disable
 */
void app_func_meas_vdda_sup_enable(bool enable) { /* parasoft-suppress MISRAC2012-RULE_8_7-a "Referenced by DVT firmware." */
	HAL_GPIO_WritePin(VDDA_EN_GPIO_Port, VDDA_EN_Pin, (enable)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
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

	case SENSOR_ID_ENG1:
	{
		HAL_GPIO_WritePin(ENG1_SDNn_GPIO_Port, ENG1_SDNn_Pin, (enable)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}	break;

	case SENSOR_ID_ENG2:
	{
		HAL_GPIO_WritePin(ENG2_SDNn_GPIO_Port, ENG2_SDNn_Pin, (enable)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
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
	switch(sensorID)
	{
	case SENSOR_ID_ECG_HR:
		bsp_adc_single_sampling(HANDLE_ID_ADC1, ADC1_CHANNEL_ECG_HR_OUT, (uint16_t*)buff, samplingPoints, samplingFrequency_hz);
		break;

	case SENSOR_ID_ECG_RR:
		bsp_adc_single_sampling(HANDLE_ID_ADC1, ADC1_CHANNEL_ECG_RR_OUT, (uint16_t*)buff, samplingPoints, samplingFrequency_hz);
		break;

	case SENSOR_ID_ENG1:
		bsp_adc_single_sampling(HANDLE_ID_ADC1, ADC1_CHANNEL_ENG1_OUT, (uint16_t*)buff, samplingPoints, samplingFrequency_hz);
		break;

	case SENSOR_ID_ENG2:
		bsp_adc_single_sampling(HANDLE_ID_ADC4, ADC4_CHANNEL_ENG2_OUT, (uint16_t*)buff, samplingPoints, samplingFrequency_hz);
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
	bsp_adc_single_sampling(HANDLE_ID_ADC4, ADC4_CHANNEL_VRECT_MON, (uint16_t*)buff, samplingPoints, samplingFrequency_hz);
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
	switch(thermID)
	{
	case THERM_ID_REF:
		bsp_adc_single_sampling(HANDLE_ID_ADC4, ADC4_CHANNEL_THERM_REF, (uint16_t*)buff, samplingPoints, samplingFrequency_hz);
		break;

	case THERM_ID_OUT:
		bsp_adc_single_sampling(HANDLE_ID_ADC4, ADC4_CHANNEL_THERM_OUT, (uint16_t*)buff, samplingPoints, samplingFrequency_hz);
		break;

	case THERM_ID_OFST:
		bsp_adc_single_sampling(HANDLE_ID_ADC4, ADC4_CHANNEL_THERM_OFST, (uint16_t*)buff, samplingPoints, samplingFrequency_hz);
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
	app_func_meas_sensor_enable(SENSOR_ID_ENG1, false);
	app_func_meas_sensor_enable(SENSOR_ID_ENG2, false);
	app_func_meas_vrect_enable(false);
	app_func_meas_therm_enable(false);
}
