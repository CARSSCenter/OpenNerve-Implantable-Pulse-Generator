/**
 * @file DAC8050x_driver.c
 * @brief This file provides the driver for the DAC80502
 * @copyright Copyright (c) 2024
 */
#include "DAC8050x_driver.h"

uint16_t DAC8050x_vout = 0U;

/**
 * @brief Get the format of DAC8050x from register and data
 * 
 * @param reg Register of DAC8050x
 * @param data Data written to DAC8050x register
 * @return DAC8050x_format_t The format of DAC8050x
 */
DAC8050x_format_t DAC8050x_format_get(uint8_t reg, uint16_t data) {
	DAC8050x_format_t format;
	uint8_t* p_data = (uint8_t*)&data;

	format.Register = reg;
	format.Data_MSB = p_data[1];
	format.Data_LSB = p_data[0];

	return format;
}

/**
 * @brief Convert output voltage to data format
 * 
 * @param vout_mv The output voltage in mV
 * @param vref_mv The reference voltage in mV
 * @param ref_div Divisor of the reference voltage, 1 or 2
 * @param gain Gain of output, 1 or 2
 * @return uint16_t 
 */
uint16_t DAC8050x_dac_vout_to_data(uint16_t vout_mv, uint16_t vref_mv, uint8_t ref_div, uint8_t gain) {
	if (vout_mv > vref_mv) {
		DAC8050x_vout = vref_mv;
	}
	else {
		DAC8050x_vout = vout_mv;
	}
	_Float64 data = (_Float64)DAC8050x_vout / (_Float64)gain * (_Float64)ref_div / (_Float64)vref_mv * (_Float64)0xFFFF;
	return (uint16_t)data;
}
