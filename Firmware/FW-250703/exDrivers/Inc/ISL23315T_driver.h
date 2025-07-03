/**
 * @file ISL23315T_driver.h
 * @brief This file contains all the function prototypes for the ISL23315T_driver.c file
 * @copyright Copyright (c) 2024
 */
#ifndef ISL23315T_DRIVER_H_
#define ISL23315T_DRIVER_H_
#include <stdint.h>

#define	ISL23315T_RESISTOR_MAX_TAP		255.0		/*!< The max resistor tap */

#define	ISL23315T_RESISTANCE			100000.0	/*!< The total resistance */

#define	ISL23315T_RESISTANCE_MI			(ISL23315T_RESISTANCE / ISL23315T_RESISTOR_MAX_TAP)		/*!< Minimum increment of resistance */
#define	ISL23315T_RESISTANCE_OFFSET_MI	0.5			/*!< Offset, Wiper at 0 Position */

#define	ISL23315T_DEVICE_ADDR_00		0xA0U		/*!< Device address with Pin A1/A0 (L/L) */
#define	ISL23315T_DEVICE_ADDR_01		0xA2U		/*!< Device address with Pin A1/A0 (L/H) */
#define	ISL23315T_DEVICE_ADDR_10		0xA4U		/*!< Device address with Pin A1/A0 (H/L) */
#define	ISL23315T_DEVICE_ADDR_11		0xA6U		/*!< Device address with Pin A1/A0 (H/H) */

#define	ISL23315T_MEM_ADDR_WR			0x00U		/*!< Register address of the register "WR" */
#define	ISL23315T_MEM_ADDR_ACR			0x10U		/*!< Register address of the register "ACR" */

/**
 * @brief Convert the resistance value of RHW to the data format of WR (Wiper Register)
 *
 * @param RHW The resistance value of RHW
 * @return uint8_t The data format of WR (Wiper Register)
 */
uint8_t ISL23315T_RHW_to_WR_data(_Float64 RHW);

#endif
