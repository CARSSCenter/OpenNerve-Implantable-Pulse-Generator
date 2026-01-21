/*
 * @file MIS2DHTR_driver.c
 * @brief This file provides the driver for the MIS2DHTR
 * @copyright Copyright (c) 2025
 */
#include "MIS2DHTR_driver.h"

/**
 * @brief Get the data of the register CTRL_REG1
 *
 * @param frequency Data rate
 * @return uint8_t The data of the register CTRL_REG1
 */
uint8_t MIS2DHTR_CTRL_REG1_data_get(uint16_t frequency) {
	uint8_t data = MIS2DHTR_NOR_XYZ_EN;
	switch (frequency) {
	case 0U:
		data += MIS2DHTR_ODR_POWER_DOWN;
		break;

	case 1U:
		data += MIS2DHTR_ODR_1HZ;
		break;

	case 10U:
		data += MIS2DHTR_ODR_10HZ;
		break;

	case 25U:
		data += MIS2DHTR_ODR_25HZ;
		break;

	case 50U:
		data += MIS2DHTR_ODR_50HZ;
		break;

	case 100U:
		data += MIS2DHTR_ODR_100HZ;
		break;

	case 200U:
		data += MIS2DHTR_ODR_200HZ;
		break;

	case 400U:
		data += MIS2DHTR_ODR_400HZ;
		break;

	default:
		data = 0U;
		break;
	}
	return data;
}
