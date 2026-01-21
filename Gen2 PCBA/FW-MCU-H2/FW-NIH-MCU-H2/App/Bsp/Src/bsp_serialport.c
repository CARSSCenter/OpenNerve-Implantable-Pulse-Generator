/**
 * @file bsp_serialport.c
 * @brief This file provides serial port management functions
 * @copyright Copyright (c) 2024
 */
#include "bsp_serialport.h"
#include "bsp_config.h"

Serialport_Buffer_t sp_spi;
Buffer_t	active_spi_tx;

Serialport_Buffer_t sp_uart;

static uint8_t CY15B108QN_mem_wr_opcode = 0;
static uint8_t CY15B108QN_mem_wr_buffer[500];
static uint16_t CY15B108QN_mem_wr_len = 0;
static uint32_t CY15B108QN_mem_wr_address = 0;
static uint16_t CY15B108QN_mem_wr_datalen = 0;

static bool init = true;
static Cmd_Parser cmdParser = NULL;
static CY15B108QN_Write_Callback CY15B108QN_writeCallback = NULL;

static uint8_t DAC8050x_reg_addr;
static uint8_t DAC8050x_reg_data[2];

static bool XL_en = false;

/**
 * @brief Initialization of serial port
 * 
 * @param cmd_parser The command parser
 * @param CY15B108QN_wr_cb The CY15B108QN write completion callback
 */
void bsp_sp_init(Cmd_Parser cmd_parser, CY15B108QN_Write_Callback CY15B108QN_wr_cb) {
	(void)memset(&sp_spi, 0, sizeof(sp_spi));
	(void)memset(&sp_uart, 0, sizeof(sp_uart));
	(void)memset(&active_spi_tx, 0, sizeof(active_spi_tx));
	cmdParser = cmd_parser;
	CY15B108QN_writeCallback = CY15B108QN_wr_cb;

	HAL_GPIO_WritePin(SPI1_FRAM_CSn_GPIO_Port, SPI1_FRAM_CSn_Pin, GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_GPIO_WritePin(SPI1_BLE_CSn_GPIO_Port, SPI1_BLE_CSn_Pin, GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */

	if (!init) {
		HAL_SPI_MspInit(&HANDLE_NRF52810_CY15B108QN_SPI);
		HAL_I2C_MspInit(&HANDLE_ISL23315T_DAC8050x_I2C);
		HAL_UART_MspInit(&HANDLE_DEBUG_UART);
		init = true;
	}

	HAL_ERROR_CHECK(HAL_UARTEx_ReceiveToIdle_IT(&HANDLE_DEBUG_UART, sp_uart.rx.data, (uint16_t)sizeof(sp_uart.rx.data)));
}

/**
 * @brief Deinitialization of serial port
 * 
 */
void bsp_sp_deinit(void) {
	HAL_ERROR_CHECK(HAL_SPI_Abort(&HANDLE_NRF52810_CY15B108QN_SPI));
	HAL_ERROR_CHECK(HAL_UART_Abort(&HANDLE_DEBUG_UART));

	if (init) {
		HAL_SPI_MspDeInit(&HANDLE_NRF52810_CY15B108QN_SPI);
		HAL_I2C_MspDeInit(&HANDLE_ISL23315T_DAC8050x_I2C);
		HAL_UART_MspDeInit(&HANDLE_DEBUG_UART);
		init = false;
	}

	HAL_GPIO_WritePin(SPI1_FRAM_CSn_GPIO_Port, SPI1_FRAM_CSn_Pin, GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_GPIO_WritePin(SPI1_BLE_CSn_GPIO_Port, SPI1_BLE_CSn_Pin, GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	CY15B108QN_mem_wr_opcode = 0;
}

/**
 * @brief Send a command on the serial port
 * 
 * @param data The command data to send
 * @param data_len The length of command data to be sent
 */
void bsp_sp_cmd_send(const uint8_t* data, uint8_t data_len) {
	(void)memcpy((uint8_t*)active_spi_tx.data, data, data_len);
	active_spi_tx.len = data_len;
}

/**
 * @brief Write data to DAC80502 on serial port
 * 
 * @param reg_addr The address of the DAC80502 register
 * @param reg_data The data to be written to the DAC80502 register
 * @return uint8_t HAL status
 */
__weak uint8_t bsp_sp_DAC80502_write(uint8_t reg_addr, uint8_t* reg_data) {
	return (uint8_t)HAL_I2C_Mem_Write(&HANDLE_ISL23315T_DAC8050x_I2C, BSP_DAC80502_DEVICE_ADDR, reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data, 2, 5);
}

/**
 * @brief Read data from DAC80502 on serial port
 *
 * @param reg_addr The address of the DAC80502 register
 * @param reg_data Data read from DAC80502 register
 * @return uint8_t HAL status
 */
__weak uint8_t bsp_sp_DAC80502_read(uint8_t reg_addr, uint8_t* reg_data) {
	return (uint8_t)HAL_I2C_Mem_Read(&HANDLE_ISL23315T_DAC8050x_I2C, BSP_DAC80502_DEVICE_ADDR, reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data, 2, 5);
}

/**
 * @brief Write data to DAC80502 on serial port in non-blocking mode
 *
 * @param reg_addr The address of the DAC80502 register
 * @param reg_data The data to be written to the DAC80502 register
 */
__weak void bsp_sp_DAC80502_write_IT(uint8_t reg_addr, uint8_t* reg_data) {
	DAC8050x_reg_addr = reg_addr;
	(void)memcpy((uint8_t*)DAC8050x_reg_data, reg_data, 2);
	(void)HAL_I2C_Mem_Write_IT(&HANDLE_ISL23315T_DAC8050x_I2C, BSP_DAC80502_DEVICE_ADDR, DAC8050x_reg_addr, I2C_MEMADD_SIZE_8BIT, DAC8050x_reg_data, 2);
}

/**
 * @brief Write data to the ISL23315T register on the serial port
 * 
 * @param reg_addr The address of the ISL23315T register
 * @param reg_data The data to be written to the ISL23315T register
 * @return uint8_t HAL status
 */
__weak uint8_t bsp_sp_ISL23315T_write(uint16_t reg_addr, uint8_t reg_data) {
	uint8_t data = reg_data;
	return (uint8_t)HAL_I2C_Mem_Write(&HANDLE_ISL23315T_DAC8050x_I2C, BSP_ISL23315T_DEVICE_ADDR, reg_addr, I2C_MEMADD_SIZE_8BIT, &data, 1, 5);
}

/**
 * @brief Write data to the CY15B108QN on the serial port in non-blocking mode
 * 
 * @param addr The address to be written to CY15B108QN.
 * @param p_data The data to be written to the CY15B108QN
 * @param data_len The length of data to be written to CY15B108QN
 */
void bsp_sp_CY15B108QN_write_IT(uint32_t addr, const uint8_t* p_data, uint16_t data_len) {
	CY15B108QN_mem_wr_len = CY15B108QN_write_spi_frame_get(CY15B108QN_mem_wr_buffer, addr, p_data, data_len);
	CY15B108QN_mem_wr_datalen = CY15B108QN_mem_wr_len - 4U;
	CY15B108QN_mem_wr_address = addr;

    CY15B108QN_mem_wr_opcode = CY15B108QN_CMD_WREN;
    HAL_GPIO_WritePin(SPI1_FRAM_CSn_GPIO_Port, SPI1_FRAM_CSn_Pin, GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
    (void)HAL_SPI_Transmit_IT(&HANDLE_NRF52810_CY15B108QN_SPI, &CY15B108QN_mem_wr_opcode, 1);
}

/**
 * @brief Read data from CY15B108QN on the serial port
 * 
 * @param addr The address of reading data from CY15B108QN.
 * @param p_data Data to be read from CY15B108QN
 * @param data_len The length of data to be read from CY15B108QN
 */
void bsp_sp_CY15B108QN_read(uint32_t addr, uint8_t* p_data, uint16_t data_len) {
	uint8_t buffer[4] = {0,0,0,0};
	(void)CY15B108QN_read_spi_frame_get(buffer, addr);

    HAL_GPIO_WritePin(SPI1_FRAM_CSn_GPIO_Port, SPI1_FRAM_CSn_Pin, GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
    HAL_ERROR_CHECK(HAL_SPI_Transmit(&HANDLE_NRF52810_CY15B108QN_SPI, buffer, (uint16_t)sizeof(buffer), 5));
    HAL_ERROR_CHECK(HAL_SPI_Receive(&HANDLE_NRF52810_CY15B108QN_SPI, p_data, data_len, 5));
	HAL_GPIO_WritePin(SPI1_FRAM_CSn_GPIO_Port, SPI1_FRAM_CSn_Pin, GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
}

/**
 * @brief Erase the data of CY15B108QN on the serial port
 * 
 * @param addr The address of the CY15B108QN data to be erased.
 * @param erase_size The length of CY15B108QN data to be erased.
 */
void bsp_sp_CY15B108QN_erase(uint32_t addr, uint32_t erase_size) {
	uint8_t CY15B108QN_mem_erase_buffer[5] = {0,0,0,0,0};
	uint8_t CY15B108QN_mem_erase_data = 0x00;
	(void)CY15B108QN_write_spi_frame_get(CY15B108QN_mem_erase_buffer, addr, &CY15B108QN_mem_erase_data, 1);

    uint8_t wren = CY15B108QN_CMD_WREN;
    HAL_GPIO_WritePin(SPI1_FRAM_CSn_GPIO_Port, SPI1_FRAM_CSn_Pin, GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
    HAL_ERROR_CHECK(HAL_SPI_Transmit(&HANDLE_NRF52810_CY15B108QN_SPI, &wren, 1, 5));
    HAL_GPIO_WritePin(SPI1_FRAM_CSn_GPIO_Port, SPI1_FRAM_CSn_Pin, GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */

    uint32_t micros = HAL_RCC_GetSysClockFreq() / 4000000UL;
    while(micros > 0U) {
    	micros--;
    }

    HAL_GPIO_WritePin(SPI1_FRAM_CSn_GPIO_Port, SPI1_FRAM_CSn_Pin, GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
    HAL_ERROR_CHECK(HAL_SPI_Transmit(&HANDLE_NRF52810_CY15B108QN_SPI, CY15B108QN_mem_erase_buffer, 5, 5));
    for(uint32_t i = (addr + 1U);i<(addr+erase_size);i++) {
    	HAL_ERROR_CHECK(HAL_SPI_Transmit(&HANDLE_NRF52810_CY15B108QN_SPI, &CY15B108QN_mem_erase_data, 1, 5));
    }
    HAL_GPIO_WritePin(SPI1_FRAM_CSn_GPIO_Port, SPI1_FRAM_CSn_Pin, GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
}

/**
 * @brief Confirm whether the serial port of CY15B108QN is busy
 * 
 * @return true The serial port is busy
 * @return false The serial port is not busy
 */
bool bsp_sp_CY15B108QN_is_busy(void) {
	bool ret = false;
	if (CY15B108QN_mem_wr_opcode > 0U) {
		ret = true;
	}
	return ret;
}

/**
 * @brief Write data to the nRF52810 on the serial port
 *
 * @param p_data The data to be written to the nRF52810
 * @param data_len The length of data to be written to the nRF52810
 */
void bsp_sp_nRF52810_write(uint8_t* p_data, uint16_t data_len) {
	HAL_GPIO_WritePin(SPI1_BLE_CSn_GPIO_Port, SPI1_BLE_CSn_Pin, GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_ERROR_CHECK(HAL_SPI_Transmit(&HANDLE_NRF52810_CY15B108QN_SPI, p_data, data_len, 10));
	HAL_GPIO_WritePin(SPI1_BLE_CSn_GPIO_Port, SPI1_BLE_CSn_Pin, GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */

}

/**
 * @brief Read data from nRF52810 on the serial port
 *
 * @param p_data Data to be read from nRF52810
 * @return uint16_t The length of data to be read from nRF52810
 */
uint16_t bsp_sp_nRF52810_read(uint8_t* p_data) {
	uint8_t data_len = 0;
	HAL_GPIO_WritePin(SPI1_BLE_CSn_GPIO_Port, SPI1_BLE_CSn_Pin, GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_ERROR_CHECK(HAL_SPI_Receive(&HANDLE_NRF52810_CY15B108QN_SPI, &data_len, 1, 5));
	if (data_len > 0U) {
		HAL_ERROR_CHECK(HAL_SPI_Receive(&HANDLE_NRF52810_CY15B108QN_SPI, (uint8_t*)p_data, data_len, 10));
	}
	HAL_GPIO_WritePin(SPI1_BLE_CSn_GPIO_Port, SPI1_BLE_CSn_Pin, GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	return (uint16_t)data_len;
}

/**
 * @brief Enable / disable the XL I2C serial port
 *
 * @param enable Enable / disable
 */
void bsp_sp_XL_enable(bool enable) {
	if (XL_en != enable) {
		HAL_GPIO_WritePin(ACC_EN_GPIO_Port, ACC_EN_Pin, GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_GPIO_WritePin(CB_EN_GPIO_Port, CB_EN_Pin, GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_Delay(10);
		HAL_GPIO_WritePin(ACC_EN_GPIO_Port, ACC_EN_Pin, (enable)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_GPIO_WritePin(CB_EN_GPIO_Port, CB_EN_Pin, (!enable)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_Delay(100);
		XL_en = enable;
	}
}

/**
 * @brief Write data to the MIS2DHTR register on the serial port
 *
 * @param dev_addr The address of the MIS2DHTR device
 * @param reg_addr The address of the MIS2DHTR register
 * @param reg_data The data to be written to the MIS2DHTR register
 * @param data_len The length of data to be written to the MIS2DHTR register
 * @return uint8_t HAL status
 */
uint8_t bsp_sp_MIS2DHTR_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t* reg_data, uint16_t data_len) {
	return (uint8_t)HAL_I2C_Mem_Write(&HANDLE_ACC_I2C, dev_addr, reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data, data_len, 10);
}

/**
 * @brief Read data from the MIS2DHTR register on the serial port
 *
 * @param dev_addr The address of the MIS2DHTR device
 * @param reg_addr The address of the MIS2DHTR register
 * @param reg_data Data read from MIS2DHTR register
 * @param data_len The length of data read from MIS2DHTR register
 * @return uint8_t HAL status
 */
uint8_t bsp_sp_MIS2DHTR_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t* reg_data, uint16_t data_len) {
	return (uint8_t)HAL_I2C_Mem_Read(&HANDLE_ACC_I2C, dev_addr, reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data, data_len, 10);
}

/**
 * @brief Handler for command communication on serial port
 * 
 * @return true The handler has been activated
 * @return false The handler has not been activated
 */
__weak bool bsp_sp_cmd_handler(void) {
	bool activated = false;
	while(bsp_sp_CY15B108QN_is_busy()) {
		__NOP();
	}

	if (HAL_GPIO_ReadPin(BLE_REQ_GPIO_Port, BLE_REQ_Pin) == GPIO_PIN_SET) { /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_Delay(1);
		sp_spi.rx.len = bsp_sp_nRF52810_read(sp_spi.rx.data);
	}

	if (sp_spi.rx.len > 0U) {
		sp_spi.tx.len = cmdParser((uint8_t*)sp_spi.rx.data, &sp_spi.rx.len, (uint8_t*)sp_spi.tx.data);
		activated = true;
	}

	while(bsp_sp_CY15B108QN_is_busy()) {
		__NOP();
	}

	if ((HAL_GPIO_ReadPin(BLE_RDY_GPIO_Port, BLE_RDY_Pin) == GPIO_PIN_SET) && (HAL_GPIO_ReadPin(BLE_REQ_GPIO_Port, BLE_REQ_Pin) == GPIO_PIN_RESET)) { /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		if (sp_spi.tx.len > 0U) {
			bsp_sp_nRF52810_write(sp_spi.tx.data, sp_spi.tx.len);
			sp_spi.tx.len = 0U;
		}
		else if (active_spi_tx.len > 0U){
			bsp_sp_nRF52810_write(active_spi_tx.data, active_spi_tx.len);
			active_spi_tx.len = 0U;
		}
		else {
			__NOP();
		}
	}

	//Debug Port
	if (sp_uart.rx.len > 0U) {
		sp_uart.tx.len = cmdParser((uint8_t*)sp_uart.rx.data, &sp_uart.rx.len, (uint8_t*)sp_uart.tx.data);
		activated = true;
	}

	if (sp_uart.tx.len > 0U) {
		HAL_ERROR_CHECK(HAL_UART_Transmit_IT(&HANDLE_DEBUG_UART, (uint8_t*)sp_uart.tx.data, sp_uart.tx.len));
		sp_uart.tx.len = 0U;
	}

	HAL_Delay(1);
	return activated;
}

/**
  * @brief SPI error callback.
  * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
	HAL_ERROR_CHECK(HAL_SPI_Abort_IT(hspi));
}

/**
  * @brief  SPI Abort Complete callback.
  * @param  hspi SPI handle.
  * @retval None
  */
void HAL_SPI_AbortCpltCallback(SPI_HandleTypeDef *hspi) { /* parasoft-suppress MISRAC2012-RULE_8_13-a "This definition comes from HAL." */
	if (hspi == &HANDLE_NRF52810_CY15B108QN_SPI) {
		(void)memset(&sp_spi, 0, sizeof(sp_spi));
		HAL_GPIO_WritePin(SPI1_BLE_CSn_GPIO_Port, SPI1_BLE_CSn_Pin, GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_GPIO_WritePin(SPI1_FRAM_CSn_GPIO_Port, SPI1_FRAM_CSn_Pin, GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		CY15B108QN_mem_wr_opcode = 0;
	}
}

/**
  * @brief Tx Transfer completed callback.
  * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) /* parasoft-suppress MISRAC2012-RULE_8_13-a "This definition comes from HAL." */
{
	if (hspi == &HANDLE_NRF52810_CY15B108QN_SPI) {
		if (HAL_GPIO_ReadPin(SPI1_FRAM_CSn_GPIO_Port, SPI1_FRAM_CSn_Pin) == GPIO_PIN_RESET) { /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
			HAL_GPIO_WritePin(SPI1_FRAM_CSn_GPIO_Port, SPI1_FRAM_CSn_Pin, GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
			if (CY15B108QN_mem_wr_opcode == CY15B108QN_CMD_WREN) {
				CY15B108QN_mem_wr_opcode = CY15B108QN_CMD_WRITE;
				uint32_t micros = HAL_RCC_GetSysClockFreq() / 4000000UL;
			    while(micros > 0U) {
			    	micros--;
			    }
				HAL_GPIO_WritePin(SPI1_FRAM_CSn_GPIO_Port, SPI1_FRAM_CSn_Pin, GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
				HAL_ERROR_CHECK(HAL_SPI_Transmit_IT(&HANDLE_NRF52810_CY15B108QN_SPI, CY15B108QN_mem_wr_buffer, CY15B108QN_mem_wr_len));
			}
			else if (CY15B108QN_mem_wr_opcode == CY15B108QN_CMD_WRITE) {
				CY15B108QN_mem_wr_opcode = 0;
				CY15B108QN_writeCallback(CY15B108QN_mem_wr_address, CY15B108QN_mem_wr_datalen);
			}
			else {
				CY15B108QN_mem_wr_opcode = 0;
			}
		}
	}
	else {
		__NOP();
	}
}

/**
  * @brief  UART error callback.
  * @param  huart UART handle.
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) /* parasoft-suppress MISRAC2012-RULE_8_13-a "This definition comes from HAL." */
{
	HAL_ERROR_CHECK(HAL_UART_Abort_IT(huart));
}

/**
  * @brief  UART Abort Complete callback.
  * @param  huart UART handle.
  * @retval None
  */
void HAL_UART_AbortCpltCallback(UART_HandleTypeDef *huart) /* parasoft-suppress MISRAC2012-RULE_8_13-a "This definition comes from HAL." */
{
	if (huart == &HANDLE_DEBUG_UART) {
		sp_uart.rx.len = 0;
		HAL_ERROR_CHECK(HAL_UARTEx_ReceiveToIdle_IT(&HANDLE_DEBUG_UART, sp_uart.rx.data, (uint16_t)sizeof(sp_uart.rx.data)));
	}
}

/**
  * @brief  Reception Event Callback (Rx event notification called after use of advanced reception service).
  * @param  huart UART handle
  * @param  Size  Number of data available in application reception buffer (indicates a position in
  *               reception buffer until which, data are available)
  * @retval None
  */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) /* parasoft-suppress MISRAC2012-RULE_8_13-a "This definition comes from HAL." */
{
	if (huart == &HANDLE_DEBUG_UART) {
		sp_uart.rx.len = (uint8_t)Size;
		HAL_ERROR_CHECK(HAL_UARTEx_ReceiveToIdle_IT(&HANDLE_DEBUG_UART, sp_uart.rx.data, (uint16_t)sizeof(sp_uart.rx.data)));
	}
}
