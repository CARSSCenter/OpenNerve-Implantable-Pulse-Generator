/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins
     PC14-OSC32_IN (PC14)   ------> RCC_OSC32_IN
     PB3 (JTDO/TRACESWO)   ------> DEBUG_JTDO-SWO
     PA13 (JTMS/SWDIO)   ------> DEBUG_JTMS-SWDIO
     PA14 (JTCK/SWCLK)   ------> DEBUG_JTCK-SWCLK
     PC15-OSC32_OUT (PC15)   ------> RCC_OSC32_OUT
     PH0-OSC_IN (PH0)   ------> RCC_OSC_IN
     PH1-OSC_OUT (PH1)   ------> RCC_OSC_OUT
     VREF+   ------> VREFBUF_OUT
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, PH1_EN_Pin|PG_START_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, VNSb_EN_Pin|PH2_EN_Pin|STIM_EN_Pin|CHG_RATE1_Pin
                          |CHG1_EN_Pin|MUX_ENn_Pin|CHG2_EN_Pin|CHG_RATE2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, VDDS_EN_Pin|ECG_RLD_Pin|ENG1_SDNn_Pin|ECG_HR_SDNn_Pin
                          |VDDA_EN_Pin|VPPSW_EN_Pin|HVSW_EN_Pin|HV_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, SNK5_Pin|SNK4_Pin|SRC2_Pin|SNK3_Pin
                          |SNK1_Pin|SNK2_Pin|SRC1_Pin|STIM_SEL_CH1n_Pin
                          |STIMB_SELn_Pin|STIM_SEL_CH3n_Pin|STIM_SEL_ENCLn_Pin|STIMA_SELn_Pin
                          |STIM_SEL_CH2n_Pin|STIM_SEL_CH4n_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, BATT_SW_EN_Pin|BLE_PWRn_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CB_EN_GPIO_Port, CB_EN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, ECG_RR_SDNn_Pin|VRECT_MON_EN_Pin|VCHG_DISABLE_Pin|TEMP_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, IPG_SHDN_Pin|FRAM_EN_Pin|BATT_MON_EN_Pin|SPI1_FRAM_CSn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, ACC_EN_Pin|BLE_RSTn_Pin|ENG2_SDNn_Pin|SPI1_BLE_CSn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, IMP_IN_P_SEL_Pin|IMP_IN_N_SEL1_Pin|IMP_EN_Pin|IMP_IN_N_SEL0_Pin
                          |IMP_IN_N_SEL2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PH1_EN_Pin PH2_EN_Pin PG_START_Pin CHG_RATE1_Pin
                           CHG1_EN_Pin MUX_ENn_Pin CHG2_EN_Pin CHG_RATE2_Pin */
  GPIO_InitStruct.Pin = PH1_EN_Pin|PH2_EN_Pin|PG_START_Pin|CHG_RATE1_Pin
                          |CHG1_EN_Pin|MUX_ENn_Pin|CHG2_EN_Pin|CHG_RATE2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : VNSb_EN_Pin STIM_EN_Pin */
  GPIO_InitStruct.Pin = VNSb_EN_Pin|STIM_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : VDDS_EN_Pin ECG_RLD_Pin ENG1_SDNn_Pin ECG_HR_SDNn_Pin
                           VDDA_EN_Pin VPPSW_EN_Pin HVSW_EN_Pin HV_EN_Pin */
  GPIO_InitStruct.Pin = VDDS_EN_Pin|ECG_RLD_Pin|ENG1_SDNn_Pin|ECG_HR_SDNn_Pin
                          |VDDA_EN_Pin|VPPSW_EN_Pin|HVSW_EN_Pin|HV_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : ENG1_LOD_Pin */
  GPIO_InitStruct.Pin = ENG1_LOD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(ENG1_LOD_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BLE_P_1_Pin BLE_P_2_Pin ENG2_LOD_Pin */
  GPIO_InitStruct.Pin = BLE_P_1_Pin|BLE_P_2_Pin|ENG2_LOD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : SNK5_Pin SNK4_Pin SRC2_Pin SNK3_Pin
                           SNK1_Pin SNK2_Pin SRC1_Pin STIM_SEL_CH1n_Pin
                           STIMB_SELn_Pin STIM_SEL_CH3n_Pin STIM_SEL_ENCLn_Pin STIMA_SELn_Pin
                           STIM_SEL_CH2n_Pin STIM_SEL_CH4n_Pin */
  GPIO_InitStruct.Pin = SNK5_Pin|SNK4_Pin|SRC2_Pin|SNK3_Pin
                          |SNK1_Pin|SNK2_Pin|SRC1_Pin|STIM_SEL_CH1n_Pin
                          |STIMB_SELn_Pin|STIM_SEL_CH3n_Pin|STIM_SEL_ENCLn_Pin|STIMA_SELn_Pin
                          |STIM_SEL_CH2n_Pin|STIM_SEL_CH4n_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : ECG_RR_LOD_Pin ECG_HR_LOD_Pin VRECT_DETn_Pin VRECT_OVPn_Pin */
  GPIO_InitStruct.Pin = ECG_RR_LOD_Pin|ECG_HR_LOD_Pin|VRECT_DETn_Pin|VRECT_OVPn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : BATT_SW_EN_Pin */
  GPIO_InitStruct.Pin = BATT_SW_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BATT_SW_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PH3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pins : CB_EN_Pin ACC_EN_Pin ENG2_SDNn_Pin SPI1_BLE_CSn_Pin */
  GPIO_InitStruct.Pin = CB_EN_Pin|ACC_EN_Pin|ENG2_SDNn_Pin|SPI1_BLE_CSn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : ECG_RR_SDNn_Pin VRECT_MON_EN_Pin VCHG_DISABLE_Pin TEMP_EN_Pin */
  GPIO_InitStruct.Pin = ECG_RR_SDNn_Pin|VRECT_MON_EN_Pin|VCHG_DISABLE_Pin|TEMP_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : IPG_SHDN_Pin BATT_MON_EN_Pin SPI1_FRAM_CSn_Pin */
  GPIO_InitStruct.Pin = IPG_SHDN_Pin|BATT_MON_EN_Pin|SPI1_FRAM_CSn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PE0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : BLE_PWRn_Pin */
  GPIO_InitStruct.Pin = BLE_PWRn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(BLE_PWRn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : IMP_IN_P_SEL_Pin IMP_IN_N_SEL1_Pin IMP_IN_N_SEL0_Pin IMP_IN_N_SEL2_Pin */
  GPIO_InitStruct.Pin = IMP_IN_P_SEL_Pin|IMP_IN_N_SEL1_Pin|IMP_IN_N_SEL0_Pin|IMP_IN_N_SEL2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : PB7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : FRAM_EN_Pin */
  GPIO_InitStruct.Pin = FRAM_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(FRAM_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : IMP_EN_Pin */
  GPIO_InitStruct.Pin = IMP_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(IMP_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : VCHG_PGOOD_Pin */
  GPIO_InitStruct.Pin = VCHG_PGOOD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(VCHG_PGOOD_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PF5 PF11 PF12 PF13 */
  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : PC3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : BLE_RSTn_Pin */
  GPIO_InitStruct.Pin = BLE_RSTn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BLE_RSTn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PG3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : PD14 PD15 */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : CHG1_OVP_ERRn_Pin CHG1_STATUS_Pin CHG2_STATUS_Pin CHG2_OVP_ERRn_Pin */
  GPIO_InitStruct.Pin = CHG1_OVP_ERRn_Pin|CHG1_STATUS_Pin|CHG2_STATUS_Pin|CHG2_OVP_ERRn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : MAG_DET_Pin */
  GPIO_InitStruct.Pin = MAG_DET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MAG_DET_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
