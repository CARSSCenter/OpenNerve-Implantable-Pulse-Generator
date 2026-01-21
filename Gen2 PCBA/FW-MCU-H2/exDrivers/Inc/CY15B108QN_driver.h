/**
 * @file CY15B108QN_driver.h
 * @brief This file contains all the function prototypes for the CY15B108QN_driver.c file
 * @copyright Copyright (c) 2024
 */
#ifndef CY15B108QN_CY15B108QN_DRIVER_H_
#define CY15B108QN_CY15B108QN_DRIVER_H_
#include <stdint.h>

#define CY15B108QN_CMD_WREN		0x06U	/*!< Command opcode. Set write enable latch */
#define CY15B108QN_CMD_WRDI		0x04U	/*!< Command opcode. Reset write enable latch */

#define CY15B108QN_CMD_RDSR		0x05U	/*!< Command opcode. Read status register */
#define CY15B108QN_CMD_WRSR		0x01U	/*!< Command opcode. Write status register */

#define CY15B108QN_CMD_WRITE	0x02U	/*!< Command opcode. Write memory data */

#define CY15B108QN_CMD_READ		0x03U	/*!< Command opcode. Read memory data */
#define CY15B108QN_CMD_FSTRD	0x0BU	/*!< Command opcode. Fast read memory data */

#define CY15B108QN_CMD_SSWR		0x42U	/*!< Command opcode. Special sector write */
#define CY15B108QN_CMD_SSRD		0x4BU	/*!< Command opcode. Special sector read */

#define CY15B108QN_CMD_RDID		0x9FU	/*!< Command opcode. Read device ID */
#define CY15B108QN_CMD_RUID		0x4CU	/*!< Command opcode. Read unique ID */
#define CY15B108QN_CMD_WRSN		0xC2U	/*!< Command opcode. Write serial number */
#define CY15B108QN_CMD_RDSN		0xC3U	/*!< Command opcode. Read serial number */

#define CY15B108QN_CMD_DPD		0xBAU	/*!< Command opcode. Enter deep power-down */
#define CY15B108QN_CMD_HBN		0xB9U	/*!< Command opcode. Enter hibernate mode */

#define CY15B108QN_MAX_ADDR		0xFFFFFU	/*!< The max address */

/**
 * @brief Get the SPI frame of the write command to CY15B108QN
 * 
 * @param p_buffer Buffer to store SPI frame
 * @param addr The address to write to
 * @param p_data The data written
 * @param data_len The length of data written.
 * @return uint16_t The length of the SPI frame
 */
uint16_t CY15B108QN_write_spi_frame_get(uint8_t* p_buffer, uint32_t addr, const uint8_t* p_data, uint16_t data_len);

/**
 * @brief Get the SPI frame of the read command to CY15B108QN
 * 
 * @param p_buffer Buffer to store SPI frame
 * @param addr The address to read from
 * @return uint16_t The length of the SPI frame
 */
uint16_t CY15B108QN_read_spi_frame_get(uint8_t* p_buffer, uint32_t addr);

#endif /* CY15B108QN_CY15B108QN_DRIVER_H_ */
