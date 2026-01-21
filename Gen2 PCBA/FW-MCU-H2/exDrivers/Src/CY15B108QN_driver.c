/**
 * @file CY15B108QN_driver.c
 * @brief This file provides the driver for the CY15B108QN
 * @copyright Copyright (c) 2024
 */
#include "CY15B108QN_driver.h"
#include <string.h>

/**
 * @brief Get the SPI frame of the write command to CY15B108QN
 * 
 * @param p_buffer Buffer to store SPI frame
 * @param addr The address to write to
 * @param p_data The data written
 * @param data_len The length of data written.
 * @return uint16_t The length of the SPI frame
 */
uint16_t CY15B108QN_write_spi_frame_get(uint8_t* p_buffer, uint32_t addr, const uint8_t* p_data, uint16_t data_len) {
	uint16_t len = 0U;
	if ((p_buffer != NULL) && (addr <= CY15B108QN_MAX_ADDR) && (p_data != NULL) && (data_len > 0U)) {
		uint8_t address[4];
		(void)memcpy((uint8_t*)address, (uint8_t*)&addr, sizeof(uint32_t));
		p_buffer[0] = CY15B108QN_CMD_WRITE;
		p_buffer[1] = address[2];
		p_buffer[2] = address[1];
		p_buffer[3] = address[0];
		(void)memcpy(&p_buffer[4], p_data, data_len);
		len = data_len + 4U;
	}
	return len;
}

/**
 * @brief Get the SPI frame of the read command to CY15B108QN
 * 
 * @param p_buffer Buffer to store SPI frame
 * @param addr The address to read from
 * @return uint16_t The length of the SPI frame
 */
uint16_t CY15B108QN_read_spi_frame_get(uint8_t* p_buffer, uint32_t addr) {
	uint16_t len = 0U;
	if ((p_buffer != NULL) && (addr <= CY15B108QN_MAX_ADDR)) {
		uint8_t address[4];
		(void)memcpy((uint8_t*)address, (uint8_t*)&addr, sizeof(uint32_t));
		p_buffer[0] = CY15B108QN_CMD_READ;
		p_buffer[1] = address[2];
		p_buffer[2] = address[1];
		p_buffer[3] = address[0];
		len = 4U;
	}
	return len;
}
