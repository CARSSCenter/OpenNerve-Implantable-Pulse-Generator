/**
 * @file bsp_fram.c
 * @brief This file provides FRAM management functions
 * @copyright Copyright (c) 2024
 */
#include "bsp_fram.h"
#include "bsp_config.h"

static bool write_cplt = false;

static Log_Write_Callback log_writeCallback = NULL;

/**
 * @brief Initialization of FRAM
 *
 * @param log_wr_cb Log writing completion callback in FRAM
 */
void bsp_fram_init(Log_Write_Callback log_wr_cb) {
	log_writeCallback = log_wr_cb;
	HAL_GPIO_WritePin(FRAM_EN_GPIO_Port, FRAM_EN_Pin, GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_Delay(1);
}

/**
 * @brief Deinitialization of FRAM
 * 
 */
void bsp_fram_deinit(void) {
	while(bsp_sp_CY15B108QN_is_busy()) {
		__NOP();
	};
	HAL_GPIO_WritePin(FRAM_EN_GPIO_Port, FRAM_EN_Pin, GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
}

/**
 * @brief Write data to FRAM
 * 
 * @param addr The address where data is written in FRAM
 * @param p_data Data written in FRAM
 * @param data_len The length of data written in FRAM
 * @param waitfor_cplt Wait for writing to complete
 */
__weak void bsp_fram_write(uint32_t addr, const uint8_t* p_data, uint16_t data_len, bool waitfor_cplt) {
	if (HAL_GPIO_ReadPin(FRAM_EN_GPIO_Port, FRAM_EN_Pin) == GPIO_PIN_RESET) {
		return;
	}
	write_cplt = false;
	bsp_sp_CY15B108QN_write_IT(addr, p_data, data_len);

	if (waitfor_cplt) {
		while(!write_cplt) {
			__NOP();
		};
		HAL_Delay(1);
	}
}

/**
 * @brief Read data from FRAM
 * 
 * @param addr The address to read data from FRAM
 * @param p_data Data read from FRAM
 * @param data_len The length of data read from FRAM
 */
__weak void bsp_fram_read(uint32_t addr, uint8_t* p_data, uint16_t data_len) {
	bsp_sp_CY15B108QN_read(addr, p_data, data_len);
}

/**
 * @brief Erase data in FRAM
 * 
 * @param addr The address to erase data in FRAM
 * @param erase_size The size of the data to erase
 */
__weak void bsp_fram_erase(uint32_t addr, uint32_t erase_size) {
	bsp_sp_CY15B108QN_erase(addr, erase_size);
}

/**
 * @brief Write to FRAM completion callback
 * 
 * @param write_addr The address of the data to write
 * @param write_size The size of the data to write
 */
void bsp_fram_write_cplt_cb(uint32_t write_addr, uint16_t write_size) {
	log_writeCallback(write_addr, write_size);
	write_cplt = true;
}
