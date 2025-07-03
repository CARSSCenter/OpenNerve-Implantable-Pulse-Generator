/**
 * @file LT1615_driver.h
 * @brief This file contains all the function prototypes for the LT1615_driver.c file
 * @copyright Copyright (c) 2024
 */
#ifndef LT1615_DRIVER_H_
#define LT1615_DRIVER_H_

#define LT1615_FB_CTP	1.23	/*!< FB Comparator Trip Point */

/**
 * @brief Calculate R2 from R1 and Vout
 *
 * @param R1 The resistance value of R1
 * @param Vout The output voltage in V
 * @return _Float64 The resistance value of R2
 */
_Float64 LT1615_R2_calculate(_Float64 R1, _Float64 Vout);

#endif
