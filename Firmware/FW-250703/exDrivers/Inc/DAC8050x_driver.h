/**
 * @file DAC8050x_driver.h
 * @brief This file contains all the function prototypes for the DAC80502_driver.c file
 * @copyright Copyright (c) 2024
 */
#ifndef DAC8050X_DRIVER_H_
#define DAC8050X_DRIVER_H_
#include <stdint.h>

#define	DAC8050x_DEVICE_ADDR_AGND	0x90
#define	DAC8050x_DEVICE_ADDR_VDD	0x92
#define	DAC8050x_DEVICE_ADDR_SDA	0x94
#define	DAC8050x_DEVICE_ADDR_SCL	0x96

#define	DAC8050x_VREF_INT_MV		2500		/*!< The internal reference voltage, unit: mV */

#define	DAC8050x_REG_NOOP			0x00		/*!< The offset of register "NOOP" */
#define	DAC8050x_REG_DEVID			0x01		/*!< The offset of register "DEVID" */
#define	DAC8050x_REG_SYNC			0x02		/*!< The offset of register "SYNC" */
#define	DAC8050x_REG_CONFIG			0x03		/*!< The offset of register "CONFIG" */
#define	DAC8050x_REG_GAIN			0x04		/*!< The offset of register "GAIN" */
#define	DAC8050x_REG_TRIGGER		0x05		/*!< The offset of register "TRIGGER" */
#define	DAC8050x_REG_BRDCAST		0x06		/*!< The offset of register "BRDCAST" */
#define	DAC8050x_REG_STATUS			0x07		/*!< The offset of register "STATUS" */
#define	DAC8050x_REG_DAC1			0x08		/*!< The offset of register "DAC-1-DATA" */
#define	DAC8050x_REG_DAC2			0x09		/*!< The offset of register "DAC-2-DATA" */

#define	DAC8050x_FIE_GAIN_REF_DIV_2		0x0100U	/*!< The field of bit "REF-DIV" in register "GAIN". Divided by a factor of 2 */
#define	DAC8050x_FIE_GAIN_BUFF1_GAIN_2	0x0001U	/*!< The field of bit "BUFF-1-GAIN" in register "GAIN". Has a gain of 2 */
#define	DAC8050x_FIE_GAIN_BUFF2_GAIN_2	0x0002U	/*!< The field of bit "BUFF-2-GAIN" in register "GAIN". Has a gain of 2 */

#define	DAC8050x_FIE_SOFT_RESET			0x000AU	/*!< The field of bits "SOFT-RESET" in register "TRIGGER". */

#define	DAC8050x_VREF_DIV_1				1
#define	DAC8050x_VREF_DIV_2				2

#define	DAC8050x_GAIN_1					1
#define	DAC8050x_GAIN_2					2

typedef struct {
	uint8_t Register;		/*!< The offset of the register */
	uint8_t Data_MSB;		/*!< The MSB data of the register */
	uint8_t Data_LSB;		/*!< The LSB data of the register */
} DAC8050x_format_t;

/**
 * @brief Get the format of DAC8050x from register and data
 * 
 * @param reg Register of DAC8050x
 * @param data Data written to DAC8050x register
 * @return DAC8050x_format_t The format of DAC8050x
 */
DAC8050x_format_t DAC8050x_format_get(uint8_t reg, uint16_t data);

/**
 * @brief Convert output voltage to data format
 * 
 * @param vout_mv The output voltage in mV
 * @param vref_mv The reference voltage in mV
 * @param ref_div Divisor of the reference voltage, 1 or 2
 * @param gain Gain of output, 1 or 2
 * @return uint16_t 
 */
uint16_t DAC8050x_dac_vout_to_data(uint16_t vout_mv, uint16_t vref_mv, uint8_t ref_div, uint8_t gain);

#endif
