/**
 * @file bsp_config.h
 * @brief Configuration of BSP in firmware
 * @copyright Copyright (c) 2024
 */
#ifndef INC_APP_CONFIG_BSP_CONFIG_H_
#define INC_APP_CONFIG_BSP_CONFIG_H_
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "main.h"
#include "adc.h"
#include "crc.h"
#include "hash.h"
#include "i2c.h"
#include "icache.h"
#include "lptim.h"
#include "memorymap.h"
#include "pka.h"
#include "rng.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "iwdg.h"
#include "gpio.h"

#include "bsp_adc.h"
#include "bsp_fram.h"
#include "bsp_magnet.h"
#include "bsp_serialport.h"
#include "bsp_watchdog.h"

#include "CY15B108QN_driver.h"
#include "DAC8050x_driver.h"
#include "LT1615_driver.h"
#include "ISL23315T_driver.h"
#include "MIS2DHTR_driver.h"

#define HW_VERSION						21

#define HANDLE_NRF52810_CY15B108QN_SPI	hspi1		/*!< SPI handle for nRF52810 and CY15B108QN */
#define HANDLE_ISL23315T_DAC8050x_I2C	hi2c2		/*!< I2C handle for ISL23315T and DAC80502 */
#define HANDLE_ACC_I2C					hi2c3		/*!< I2C handle for ACC Connector */
#define HANDLE_DEBUG_UART				huart1		/*!< UART handle for debug */
#define HANDLE_WAKEUP_LPTIM				hlptim1		/*!< Low power timer handle for wake up */
#define HANDLE_MAGNET_LPTIM				hlptim2		/*!< Low power timer handle for magnet */
#define HANDLE_BLE_DEFAULT_LPTIM		hlptim3		/*!< Low power timer handle for BLE default settings */
#define HANDLE_WDG_REFRESH_LPTIM		hlptim4		/*!< Low power timer handle for Refresh Watchdog */
#define HANDLE_PULSE1_TIM				htim3		/*!< Timer handle for pulse1 */
#define HANDLE_PULSE2_TIM				htim5		/*!< Timer handle for pulse2 */
#define HANDLE_SINE_TIM					htim4		/*!< Timer handle for sine */
#define HANDLE_ADC1_SAMPLE_TIM			htim6		/*!< Timer handle for ADC1 sampling */
#define HANDLE_ADC4_SAMPLE_TIM			htim15		/*!< Timer handle for ADC4 sampling */

#define TIM_CH_PULSE1_TO_LOW			TIM_CHANNEL_1	/*!< The timer channel for pulse1 */
#define TIM_CH_PULSE2_TO_LOW			TIM_CHANNEL_1	/*!< The timer channel for pulse2 */
#define TIM_CH_PULSE1_BEF_HI			TIM_CHANNEL_2	/*!< The timer channel for pulse1 */
#define TIM_CH_PULSE2_BEF_HI			TIM_CHANNEL_2	/*!< The timer channel for pulse2 */
#define TIM_CH_SINE_POLR				TIM_CHANNEL_1	/*!< The timer channel for sine waveform polarity */
#define TIM_CH_SINE_AMP					TIM_CHANNEL_2	/*!< The timer channel for sine waveform amplitude */

#define TIM_ACH_PULSE1_TO_LOW			HAL_TIM_ACTIVE_CHANNEL_1
#define TIM_ACH_PULSE2_TO_LOW			HAL_TIM_ACTIVE_CHANNEL_1
#define TIM_ACH_PULSE1_BEF_HI			HAL_TIM_ACTIVE_CHANNEL_2
#define TIM_ACH_PULSE2_BEF_HI			HAL_TIM_ACTIVE_CHANNEL_2
#define TIM_ACH_SINE_POLR				HAL_TIM_ACTIVE_CHANNEL_1
#define TIM_ACH_SINE_AMP				HAL_TIM_ACTIVE_CHANNEL_2

#define BLE_RDY_GPIO_Port				BLE_P_1_GPIO_Port
#define BLE_RDY_Pin						BLE_P_1_Pin
#define BLE_REQ_GPIO_Port				BLE_P_2_GPIO_Port
#define BLE_REQ_Pin						BLE_P_2_Pin

#define BSP_MIRROR_RREF					1000.0f
#define BSP_MIRROR_ROUT					150.0f
#define BSP_VT     						0.02585f
#define BSP_STIM_RREF					2800.0f

#define	BSP_BATT_FACTOR					(2)				/*!< Voltage factor for battery monitor */
#define	BSP_IMP_FACTOR					(6.0/7.2)		/*!< Voltage factor for impedance monitor */
#define	BSP_IMP_COMM					1500			/*!< Common-mode voltage of the IMC differential output, mV */

#define	BSP_LT1615_R1					340000.0	/*!< Resistance of LT1615 resistor R1 */
#define	BSP_ISL23315T_RHGND				40200.0		/*!< Resistance of ISL23315T resistor RHGND */

#define	BSP_ISL23315T_DEVICE_ADDR		ISL23315T_DEVICE_ADDR_00	/*!< Device address of ISL23315T */
#define	BSP_DAC80502_DEVICE_ADDR		DAC8050x_DEVICE_ADDR_AGND	/*!< Device address of DAC80502 */

#define	BSP_DAC80502_VREF				2.5f		/*!< The reference output voltage of DAC80502, V */

/**@brief Macro for calling error handler function if supplied error code any other than HAL_OK.
 *
 * @param[in] ERR_CODE Error code supplied to the error handler.
 */
#define HAL_ERROR_CHECK(ERR_CODE)                           \
        if ((uint8_t)(ERR_CODE) != HAL_OK)                  \
        {                                                   \
            Error_Handler();              					\
        }
#endif /* INC_APP_CONFIG_BSP_CONFIG_H_ */
