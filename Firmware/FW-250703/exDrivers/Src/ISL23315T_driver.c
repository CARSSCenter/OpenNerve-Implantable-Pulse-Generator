/**
 * @file ISL23315T_driver.c
 * @brief This file provides the driver for the ISL23315T
 * @copyright Copyright (c) 2024
 */
#include "ISL23315T_driver.h"

/**
 * @brief Convert the resistance value of RHW to the data format of WR (Wiper Register)
 * 
 * @param RHW The resistance value of RHW
 * @return uint8_t The data format of WR (Wiper Register)
 */
uint8_t ISL23315T_RHW_to_WR_data(_Float64 RHW) {
	_Float64 R = 0.0;
	if (RHW < 0.0) {
		R = 0.0;
	}
	else if (RHW > ISL23315T_RESISTANCE) {
		R = ISL23315T_RESISTANCE;
	}
	else {
		R = RHW;
	}
	_Float64 WR = (_Float64)ISL23315T_RESISTOR_MAX_TAP - ((R / ISL23315T_RESISTANCE_MI) - ISL23315T_RESISTANCE_OFFSET_MI);
	return (uint8_t)WR;
}
