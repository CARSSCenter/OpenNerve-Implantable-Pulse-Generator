/**
 * @file bsp_serialport.h
 * @brief This file contains all the function prototypes for the bsp_serialport.c file
 * @copyright Copyright (c) 2024
 */
#ifndef BSP_INC_BSP_SERIALPORT_H_
#define BSP_INC_BSP_SERIALPORT_H_
#include <stdint.h>
#include <stdbool.h>

#define SP_BUF_SIZE		255U		/*!< The buffer size of the serial port */

typedef struct {
	uint8_t data[SP_BUF_SIZE];		/*!< The data buffer of the serial port */
	uint8_t len;					/*!< The effective length of the serial port data buffer */
} Buffer_t;

typedef struct {
	Buffer_t tx;					/*!< The serial port's transmit buffer */
	Buffer_t rx;					/*!< The serial port's receive buffer */
} Serialport_Buffer_t;

typedef uint8_t (*Cmd_Parser)(uint8_t* p_data_rx, uint8_t* p_data_rx_len, uint8_t* p_data_tx);	/*!< The format of the command parser */

typedef void (*CY15B108QN_Write_Callback)(uint32_t write_addr, uint16_t write_size);	/*!< The format of the CY15B108QN write completion callback */

extern Serialport_Buffer_t sp_uart;

/**
 * @brief Initialization of serial port
 * 
 * @param cmd_parser The command parser
 * @param CY15B108QN_wr_cb The CY15B108QN write completion callback
 */
void bsp_sp_init(Cmd_Parser cmd_parser, CY15B108QN_Write_Callback CY15B108QN_wr_cb);

/**
 * @brief Deinitialization of serial port
 * 
 */
void bsp_sp_deinit(void);

/**
 * @brief Send a command on the serial port
 * 
 * @param data The command data to send
 * @param data_len The length of command data to be sent
 */
void bsp_sp_cmd_send(const uint8_t* data, uint8_t data_len);

/**
 * @brief Write data to DAC80502 on serial port
 * 
 * @param reg_addr The address of the DAC80502 register
 * @param reg_data The data to be written to the DAC80502 register
 * @return uint8_t HAL status
 */
uint8_t bsp_sp_DAC80502_write(uint8_t reg_addr, uint8_t* reg_data);

/**
 * @brief Read data from DAC80502 on serial port
 *
 * @param reg_addr The address of the DAC80502 register
 * @param reg_data Data read from DAC80502 register
 * @return uint8_t HAL status
 */
uint8_t bsp_sp_DAC80502_read(uint8_t reg_addr, uint8_t* reg_data);

/**
 * @brief Write data to DAC80502 on serial port in non-blocking mode
 *
 * @param reg_addr The address of the DAC80502 register
 * @param reg_data The data to be written to the DAC80502 register
 */
void bsp_sp_DAC80502_write_IT(uint8_t reg_addr, uint8_t* reg_data);

/**
 * @brief Write data to the ISL23315T register on the serial port
 * 
 * @param reg_addr The address of the ISL23315T register
 * @param reg_data The data to be written to the ISL23315T register
 * @return uint8_t HAL status
 */
uint8_t bsp_sp_ISL23315T_write(uint16_t reg_addr, uint8_t reg_data);

/**
 * @brief Write data to the CY15B108QN on the serial port in non-blocking mode
 * 
 * @param addr The address to be written to CY15B108QN.
 * @param p_data The data to be written to the CY15B108QN
 * @param data_len The length of data to be written to CY15B108QN
 */
void bsp_sp_CY15B108QN_write_IT(uint32_t addr, const uint8_t* p_data, uint16_t data_len);

/**
 * @brief Read data from CY15B108QN on the serial port
 * 
 * @param addr The address of reading data from CY15B108QN.
 * @param p_data Data to be read from CY15B108QN
 * @param data_len The length of data to be read from CY15B108QN
 */
void bsp_sp_CY15B108QN_read(uint32_t addr, uint8_t* p_data, uint16_t data_len);

/**
 * @brief Erase the data of CY15B108QN on the serial port
 * 
 * @param addr The address of the CY15B108QN data to be erased.
 * @param erase_size The length of CY15B108QN data to be erased.
 */
void bsp_sp_CY15B108QN_erase(uint32_t addr, uint32_t erase_size);

/**
 * @brief Confirm whether the serial port of CY15B108QN is busy
 * 
 * @return true The serial port is busy
 * @return false The serial port is not busy
 */
bool bsp_sp_CY15B108QN_is_busy(void);

/**
 * @brief Write data to the nRF52810 on the serial port
 *
 * @param p_data The data to be written to the nRF52810
 * @param data_len The length of data to be written to the nRF52810
 */
void bsp_sp_nRF52810_write(uint8_t* p_data, uint16_t data_len);

/**
 * @brief Read data from nRF52810 on the serial port
 *
 * @param p_data Data to be read from nRF52810
 * @return uint16_t The length of data to be read from nRF52810
 */
uint16_t bsp_sp_nRF52810_read(uint8_t* p_data);

/**
 * @brief Enable / disable the XL I2C serial port
 *
 * @param enable Enable / disable
 */
void bsp_sp_XL_enable(bool enable);

/**
 * @brief Write data to the MIS2DHTR register on the serial port
 *
 * @param dev_addr The address of the MIS2DHTR device
 * @param reg_addr The address of the MIS2DHTR register
 * @param reg_data The data to be written to the MIS2DHTR register
 * @param data_len The length of data to be written to the MIS2DHTR register
 * @return uint8_t HAL status
 */
uint8_t bsp_sp_MIS2DHTR_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t* reg_data, uint16_t data_len);

/**
 * @brief Read data from the MIS2DHTR register on the serial port
 *
 * @param dev_addr The address of the MIS2DHTR device
 * @param reg_addr The address of the MIS2DHTR register
 * @param reg_data Data read from MIS2DHTR register
 * @param data_len The length of data read from MIS2DHTR register
 * @return uint8_t HAL status
 */
uint8_t bsp_sp_MIS2DHTR_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t* reg_data, uint16_t data_len);

/**
 * @brief Handler for command communication on serial port
 * 
 * @return true The handler has been activated
 * @return false The handler has not been activated
 */
bool bsp_sp_cmd_handler(void);

#endif /* BSP_INC_BSP_SERIALPORT_H_ */
