/**
 * @file bsp_fram.h
 * @brief This file contains all the function prototypes for the app_func_fram.c file
 * @copyright Copyright (c) 2024
 */
#ifndef BSP_FRAM_H_
#define BSP_FRAM_H_
#include <stdint.h>
#include <stdbool.h>

#define ADDR_LOG_BASE					0x00000UL	/*!< The base address of the log */
#define SIZE_LOG						0x20000UL	/*!< The FRAM size of the log */
#define ADDR_LOG_INFO					(ADDR_LOG_BASE + SIZE_LOG)	/*!< The address of the log info */

#define ADDR_FW_IMG_BASE				0x40000UL	/*!< The base address of the firmware image */
#define SIZE_FW_IMG						0x40000UL	/*!< The FRAM size of the firmware image */
#define ADDR_FW_IMG_INFO				(ADDR_FW_IMG_BASE + SIZE_FW_IMG)	/*!< The address of the firmware image info */
#define SIZE_FW_IMG_PKG					128U		/*!< The packet size of the firmware image */

#define ADDR_SYS_CONFIG					0xA0000UL	/*!< The address of the system configuration */

typedef struct {
	uint32_t 	ImageDataOffset;					/*!< The offset of the data in the image */
	uint8_t 	ImageData[SIZE_FW_IMG_PKG];			/*!< The data of the packet */
} FW_Image_Packet_t;

typedef struct {
	uint32_t 	ImageSize;							/*!< The size of the image */
	uint8_t 	ImageHash[32];						/*!< The hash of the image */
} FW_Image_Info_t;

typedef struct {
	uint16_t	DefaultState;						/*!< The system's default state. */
	uint16_t 	StartState;							/*!< The system's starting state after reboot. */
} Sys_Config_t;

typedef void (*Log_Write_Callback)(uint32_t write_addr, uint16_t write_size);

/**
 * @brief Initialization of FRAM
 *
 * @param log_wr_cb Log writing completion callback in FRAM
 */
void bsp_fram_init(Log_Write_Callback log_wr_cb);

/**
 * @brief Deinitialization of FRAM
 *
 */
void bsp_fram_deinit(void);

/**
 * @brief Write data to FRAM
 *
 * @param addr The address where data is written in FRAM
 * @param p_data Data written in FRAM
 * @param data_len The length of data written in FRAM
 * @param waitfor_cplt Wait for writing to complete
 */
void bsp_fram_write(uint32_t addr, const uint8_t* p_data, uint16_t data_len, bool waitfor_cplt);

/**
 * @brief Read data from FRAM
 *
 * @param addr The address to read data from FRAM
 * @param p_data Data read from FRAM
 * @param data_len The length of data read from FRAM
 */
void bsp_fram_read(uint32_t addr, uint8_t* p_data, uint16_t data_len);

/**
 * @brief Erase data in FRAM
 *
 * @param addr The address to erase data in FRAM
 * @param erase_size The size of the data to erase
 */
void bsp_fram_erase(uint32_t addr, uint32_t erase_size);

/**
 * @brief Write to FRAM completion callback
 *
 * @param write_addr The address of the data to write
 * @param write_size The size of the data to write
 */
void bsp_fram_write_cplt_cb(uint32_t write_addr, uint16_t write_size);

#endif /* BSP_FRAM_H_ */
