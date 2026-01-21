/**
 * @file MIS2DHTR_driver.h
 * @brief This file contains all the function prototypes for the MIS2DHTR_driver.c file
 * @copyright Copyright (c) 2025
 */
#ifndef INC_MIS2DHTR_DRIVER_H_
#define INC_MIS2DHTR_DRIVER_H_
#include <stdint.h>
#include <stdbool.h>

#define	MIS2DHTR_BOOT_MS_TIME			5U			/*!< The boot procedure is complete about 5 milliseconds after device power-up. */

#define	MIS2DHTR_DEVICE_ADDR_L			0x30U		/*!< Device address with Pin SA0 (L) */
#define	MIS2DHTR_DEVICE_ADDR_H			0x32U		/*!< Device address with Pin SA0 (H) */

#define	MIS2DHTR_STATUS_REG_AUX			0x07U		/*!< Read */
#define	MIS2DHTR_OUT_TEMP_L				0x0CU		/*!< Read */
#define	MIS2DHTR_OUT_TEMP_H				0x0DU		/*!< Read */
#define	MIS2DHTR_INT_COUNTER_REG		0x0EU		/*!< Read */
#define	MIS2DHTR_WHO_AM_I				0x0FU		/*!< Read */
#define	MIS2DHTR_CTRL_REG1				0x20U		/*!< Read/Write */
#define	MIS2DHTR_CTRL_REG2				0x21U		/*!< Read/Write */
#define	MIS2DHTR_CTRL_REG3				0x22U		/*!< Read/Write */
#define	MIS2DHTR_CTRL_REG4				0x23U		/*!< Read/Write */
#define	MIS2DHTR_CTRL_REG5				0x24U		/*!< Read/Write */
#define	MIS2DHTR_CTRL_REG6				0x25U		/*!< Read/Write */
#define	MIS2DHTR_REFERENCE_DATACAPTURE 	0x26U		/*!< Read/Write */
#define	MIS2DHTR_STATUS_REG				0x27U		/*!< Read */
#define	MIS2DHTR_OUT_X_L				0x28U		/*!< Read */
#define	MIS2DHTR_OUT_X_H				0x29U		/*!< Read */
#define	MIS2DHTR_OUT_Y_L				0x2AU		/*!< Read */
#define	MIS2DHTR_OUT_Y_H				0x2BU		/*!< Read */
#define	MIS2DHTR_OUT_Z_L				0x2CU		/*!< Read */
#define	MIS2DHTR_OUT_Z_H				0x2DU		/*!< Read */
#define	MIS2DHTR_FIFO_CTRL_REG			0x2EU		/*!< Read/Write */
#define	MIS2DHTR_FIFO_SRC_REG			0x2FU		/*!< Read */
#define	MIS2DHTR_INT1_CFG				0x30U		/*!< Read/Write */
#define	MIS2DHTR_INT1_SRC				0x31U		/*!< Read */
#define	MIS2DHTR_INT1_THS				0x32U		/*!< Read/Write */
#define	MIS2DHTR_INT1_DURATION			0x33U		/*!< Read/Write */
#define	MIS2DHTR_INT2_CFG				0x34U		/*!< Read/Write */
#define	MIS2DHTR_INT2_SRC				0x35U		/*!< Read */
#define	MIS2DHTR_INT2_THS				0x36U		/*!< Read/Write */
#define	MIS2DHTR_INT2_DURATION			0x37U		/*!< Read/Write */
#define	MIS2DHTR_CLICK_CFG				0x38U		/*!< Read/Write */
#define	MIS2DHTR_CLICK_SRC				0x39U		/*!< Read */
#define	MIS2DHTR_CLICK_THS				0x3AU		/*!< Read/Write */
#define	MIS2DHTR_TIME_LIMIT				0x3BU		/*!< Read/Write */
#define	MIS2DHTR_TIME_LATENCY			0x3CU		/*!< Read/Write */
#define	MIS2DHTR_TIME_WINDOW			0x3DU		/*!< Read/Write */
#define	MIS2DHTR_ACT_THS				0x3EU		/*!< Read/Write */
#define	MIS2DHTR_ACT_DUR				0x3FU		/*!< Read/Write */

#define	MIS2DHTR_ADDR_AUTO_INCREMENT	0x80U		/*!< Enables address auto increment */

//For MIS2DHTR_WHO_AM_I
#define	MIS2DHTR_WHO_AM_I_DATA			0b00110011

//For MIS2DHTR_CTRL_REG1
#define	MIS2DHTR_ODR_POWER_DOWN			0b00000000
#define	MIS2DHTR_ODR_1HZ				0b00010000
#define	MIS2DHTR_ODR_10HZ				0b00100000
#define	MIS2DHTR_ODR_25HZ				0b00110000
#define	MIS2DHTR_ODR_50HZ				0b01000000
#define	MIS2DHTR_ODR_100HZ				0b01010000
#define	MIS2DHTR_ODR_200HZ				0b01100000
#define	MIS2DHTR_ODR_400HZ				0b01110000
#define	MIS2DHTR_ODR_1620HZ				0b10000000
#define	MIS2DHTR_ODR_N1344HZ_L5376HZ	0b10010000

#define	MIS2DHTR_NOR_XYZ_EN				0b00000111

//For MIS2DHTR_CTRL_REG5
#define	MIS2DHTR_FIFO_EN				0b01000000

//For MIS2DHTR_FIFO_CTRL_REG
#define	MIS2DHTR_FM_BYPASS				0b00000000
#define	MIS2DHTR_FM_FIFO				0b01000000
#define	MIS2DHTR_FM_STREAM				0b10000000
#define	MIS2DHTR_FM_STREAM_FIFO			0b11000000

#define	MIS2DHTR_TR_INT1				0b00000000
#define	MIS2DHTR_TR_INT2				0b00100000

//For MIS2DHTR_FIFO_SRC_REG
#define MIS2DHTR_FIFO_IS_WTM(FIFO_SRC_REG_data)    	(((FIFO_SRC_REG_data) & 0b10000000) ? true : false)
#define MIS2DHTR_FIFO_IS_OVRN(FIFO_SRC_REG_data)    (((FIFO_SRC_REG_data) & 0b01000000) ? true : false)
#define MIS2DHTR_FIFO_IS_EMPTY(FIFO_SRC_REG_data)   (((FIFO_SRC_REG_data) & 0b00100000) ? true : false)
#define MIS2DHTR_FIFO_GET_FSS(FIFO_SRC_REG_data)    ((uint8_t)((FIFO_SRC_REG_data) & 0b00011111))

#define MIS2DHTR_FIFO_LEVEL		32

typedef struct {
	uint16_t Xaxis;
	uint16_t Yaxis;
	uint16_t Zaxis;
} XYZ_t;

typedef struct {
	XYZ_t		Axis[MIS2DHTR_FIFO_LEVEL];
	uint8_t		SrcRegister;
} FIFO_t;

typedef struct {
	uint8_t		DeviceAddress;
	uint8_t		RegisterAddress;
	uint8_t		RegisterData;
	FIFO_t		Fifo;
} MIS2DHTR_t;

/**
 * @brief Get the data of the register CTRL_REG1
 *
 * @param frequency Data rate
 * @return uint8_t The data of the register CTRL_REG1
 */
uint8_t MIS2DHTR_CTRL_REG1_data_get(uint16_t frequency);

#endif /* INC_MIS2DHTR_DRIVER_H_ */
