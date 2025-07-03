/**
 * @file LT1615_driver.c
 * @brief This file provides the driver for the LT1615
 * @copyright Copyright (c) 2024
 */
#include "LT1615_driver.h"

/**
 * @brief Calculate R2 from R1 and Vout
 * 
 * @param R1 The resistance value of R1
 * @param Vout The output voltage in V
 * @return _Float64 The resistance value of R2
 */
_Float64 LT1615_R2_calculate(_Float64 R1, _Float64 Vout) {
	_Float64 factor = (Vout / LT1615_FB_CTP) - 1.0;
	if (factor > 0) {
		return R1 / factor;
	}
	else {
		return R1;
	}
}
