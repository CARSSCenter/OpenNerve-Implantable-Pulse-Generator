/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32u5xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void SystemClock_Config(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PH1_EN_Pin GPIO_PIN_5
#define PH1_EN_GPIO_Port GPIOE
#define AMPL_RSTn_Pin GPIO_PIN_3
#define AMPL_RSTn_GPIO_Port GPIOE
#define VNSb_EN_Pin GPIO_PIN_1
#define VNSb_EN_GPIO_Port GPIOE
#define VDDS_EN_Pin GPIO_PIN_9
#define VDDS_EN_GPIO_Port GPIOB
#define ENG1_LOD_Pin GPIO_PIN_6
#define ENG1_LOD_GPIO_Port GPIOB
#define BLE_P_1_Pin GPIO_PIN_12
#define BLE_P_1_GPIO_Port GPIOG
#define SNK5_Pin GPIO_PIN_6
#define SNK5_GPIO_Port GPIOD
#define SNK4_Pin GPIO_PIN_5
#define SNK4_GPIO_Port GPIOD
#define SRC2_Pin GPIO_PIN_2
#define SRC2_GPIO_Port GPIOD
#define ECG_RR_LOD_Pin GPIO_PIN_11
#define ECG_RR_LOD_GPIO_Port GPIOC
#define BATT_SW_EN_Pin GPIO_PIN_15
#define BATT_SW_EN_GPIO_Port GPIOA
#define PH2_EN_Pin GPIO_PIN_4
#define PH2_EN_GPIO_Port GPIOE
#define STIM_EN_Pin GPIO_PIN_2
#define STIM_EN_GPIO_Port GPIOE
#define ECG_RLD_Pin GPIO_PIN_4
#define ECG_RLD_GPIO_Port GPIOB
#define CB_EN_Pin GPIO_PIN_9
#define CB_EN_GPIO_Port GPIOG
#define SNK3_Pin GPIO_PIN_4
#define SNK3_GPIO_Port GPIOD
#define SNK1_Pin GPIO_PIN_1
#define SNK1_GPIO_Port GPIOD
#define ECG_RR_SDNn_Pin GPIO_PIN_12
#define ECG_RR_SDNn_GPIO_Port GPIOC
#define VRECT_MON_EN_Pin GPIO_PIN_10
#define VRECT_MON_EN_GPIO_Port GPIOC
#define IPG_SHDN_Pin GPIO_PIN_12
#define IPG_SHDN_GPIO_Port GPIOA
#define PG_START_Pin GPIO_PIN_6
#define PG_START_GPIO_Port GPIOE
#define ECG_HR_LOD_Pin GPIO_PIN_13
#define ECG_HR_LOD_GPIO_Port GPIOC
#define ENG1_SDNn_Pin GPIO_PIN_8
#define ENG1_SDNn_GPIO_Port GPIOB
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define ACC_EN_Pin GPIO_PIN_10
#define ACC_EN_GPIO_Port GPIOG
#define SNK2_Pin GPIO_PIN_3
#define SNK2_GPIO_Port GPIOD
#define SRC1_Pin GPIO_PIN_0
#define SRC1_GPIO_Port GPIOD
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define BLE_PWRn_Pin GPIO_PIN_11
#define BLE_PWRn_GPIO_Port GPIOA
#define IMP_IN_P_SEL_Pin GPIO_PIN_0
#define IMP_IN_P_SEL_GPIO_Port GPIOF
#define IMP_IN_N_SEL1_Pin GPIO_PIN_3
#define IMP_IN_N_SEL1_GPIO_Port GPIOF
#define ECG_HR_SDNn_Pin GPIO_PIN_5
#define ECG_HR_SDNn_GPIO_Port GPIOB
#define STIM_SEL_CH1n_Pin GPIO_PIN_7
#define STIM_SEL_CH1n_GPIO_Port GPIOD
#define DEBUG_UART_TX_Pin GPIO_PIN_9
#define DEBUG_UART_TX_GPIO_Port GPIOA
#define DEBUG_UART_RX_Pin GPIO_PIN_10
#define DEBUG_UART_RX_GPIO_Port GPIOA
#define FRAM_EN_Pin GPIO_PIN_8
#define FRAM_EN_GPIO_Port GPIOA
#define IMP_EN_Pin GPIO_PIN_2
#define IMP_EN_GPIO_Port GPIOF
#define IMP_IN_N_SEL0_Pin GPIO_PIN_1
#define IMP_IN_N_SEL0_GPIO_Port GPIOF
#define IMP_IN_N_SEL2_Pin GPIO_PIN_4
#define IMP_IN_N_SEL2_GPIO_Port GPIOF
#define VRECT_DETn_Pin GPIO_PIN_7
#define VRECT_DETn_GPIO_Port GPIOC
#define VCHG_PGOOD_Pin GPIO_PIN_9
#define VCHG_PGOOD_GPIO_Port GPIOC
#define VCHG_DISABLE_Pin GPIO_PIN_8
#define VCHG_DISABLE_GPIO_Port GPIOC
#define THERM_REF_Pin GPIO_PIN_2
#define THERM_REF_GPIO_Port GPIOC
#define BLE_RSTn_Pin GPIO_PIN_6
#define BLE_RSTn_GPIO_Port GPIOG
#define ACC_SCL_Pin GPIO_PIN_7
#define ACC_SCL_GPIO_Port GPIOG
#define VRECT_OVPn_Pin GPIO_PIN_6
#define VRECT_OVPn_GPIO_Port GPIOC
#define ACC_SDA_Pin GPIO_PIN_8
#define ACC_SDA_GPIO_Port GPIOG
#define THERM_OUT_Pin GPIO_PIN_1
#define THERM_OUT_GPIO_Port GPIOC
#define BATT_MON2_Pin GPIO_PIN_1
#define BATT_MON2_GPIO_Port GPIOA
#define BLE_P_2_Pin GPIO_PIN_4
#define BLE_P_2_GPIO_Port GPIOG
#define ENG2_SDNn_Pin GPIO_PIN_2
#define ENG2_SDNn_GPIO_Port GPIOG
#define SPI1_BLE_CSn_Pin GPIO_PIN_5
#define SPI1_BLE_CSn_GPIO_Port GPIOG
#define THERM_OFST_Pin GPIO_PIN_0
#define THERM_OFST_GPIO_Port GPIOC
#define STIMB_SELn_Pin GPIO_PIN_13
#define STIMB_SELn_GPIO_Port GPIOD
#define BATT_MON1_Pin GPIO_PIN_0
#define BATT_MON1_GPIO_Port GPIOA
#define TEMP_EN_Pin GPIO_PIN_5
#define TEMP_EN_GPIO_Port GPIOC
#define IMP_OUT_P_Pin GPIO_PIN_14
#define IMP_OUT_P_GPIO_Port GPIOF
#define CHG_RATE1_Pin GPIO_PIN_8
#define CHG_RATE1_GPIO_Port GPIOE
#define CHG1_EN_Pin GPIO_PIN_10
#define CHG1_EN_GPIO_Port GPIOE
#define CHG1_OVP_ERRn_Pin GPIO_PIN_12
#define CHG1_OVP_ERRn_GPIO_Port GPIOE
#define STIM_SEL_CH3n_Pin GPIO_PIN_9
#define STIM_SEL_CH3n_GPIO_Port GPIOD
#define STIM_SEL_ENCLn_Pin GPIO_PIN_11
#define STIM_SEL_ENCLn_GPIO_Port GPIOD
#define STIMA_SELn_Pin GPIO_PIN_12
#define STIMA_SELn_GPIO_Port GPIOD
#define BATT_MON_EN_Pin GPIO_PIN_2
#define BATT_MON_EN_GPIO_Port GPIOA
#define SPI1_MOSI_Pin GPIO_PIN_7
#define SPI1_MOSI_GPIO_Port GPIOA
#define ENG1_OUT_Pin GPIO_PIN_2
#define ENG1_OUT_GPIO_Port GPIOB
#define ENG2_LOD_Pin GPIO_PIN_1
#define ENG2_LOD_GPIO_Port GPIOG
#define MUX_ENn_Pin GPIO_PIN_7
#define MUX_ENn_GPIO_Port GPIOE
#define CHG1_STATUS_Pin GPIO_PIN_14
#define CHG1_STATUS_GPIO_Port GPIOE
#define VDDA_EN_Pin GPIO_PIN_10
#define VDDA_EN_GPIO_Port GPIOB
#define I2C2_SCL_Pin GPIO_PIN_13
#define I2C2_SCL_GPIO_Port GPIOB
#define I2C2_SDA_Pin GPIO_PIN_14
#define I2C2_SDA_GPIO_Port GPIOB
#define VPPSW_EN_Pin GPIO_PIN_15
#define VPPSW_EN_GPIO_Port GPIOB
#define MAG_DET_Pin GPIO_PIN_3
#define MAG_DET_GPIO_Port GPIOA
#define MAG_DET_EXTI_IRQn EXTI3_IRQn
#define SPI1_MISO_Pin GPIO_PIN_6
#define SPI1_MISO_GPIO_Port GPIOA
#define SPI1_FRAM_CSn_Pin GPIO_PIN_4
#define SPI1_FRAM_CSn_GPIO_Port GPIOA
#define ECG_RR_OUT_Pin GPIO_PIN_1
#define ECG_RR_OUT_GPIO_Port GPIOB
#define IMP_OUT_N_Pin GPIO_PIN_15
#define IMP_OUT_N_GPIO_Port GPIOF
#define CHG2_EN_Pin GPIO_PIN_11
#define CHG2_EN_GPIO_Port GPIOE
#define CHG2_STATUS_Pin GPIO_PIN_15
#define CHG2_STATUS_GPIO_Port GPIOE
#define HVSW_EN_Pin GPIO_PIN_11
#define HVSW_EN_GPIO_Port GPIOB
#define HV_EN_Pin GPIO_PIN_12
#define HV_EN_GPIO_Port GPIOB
#define STIM_SEL_CH2n_Pin GPIO_PIN_8
#define STIM_SEL_CH2n_GPIO_Port GPIOD
#define SPI1_SCK_Pin GPIO_PIN_5
#define SPI1_SCK_GPIO_Port GPIOA
#define VRECT_MON_Pin GPIO_PIN_4
#define VRECT_MON_GPIO_Port GPIOC
#define ECG_HR_OUT_Pin GPIO_PIN_0
#define ECG_HR_OUT_GPIO_Port GPIOB
#define ENG2_OUT_Pin GPIO_PIN_0
#define ENG2_OUT_GPIO_Port GPIOG
#define CHG_RATE2_Pin GPIO_PIN_9
#define CHG_RATE2_GPIO_Port GPIOE
#define CHG2_OVP_ERRn_Pin GPIO_PIN_13
#define CHG2_OVP_ERRn_GPIO_Port GPIOE
#define STIM_SEL_CH4n_Pin GPIO_PIN_10
#define STIM_SEL_CH4n_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
