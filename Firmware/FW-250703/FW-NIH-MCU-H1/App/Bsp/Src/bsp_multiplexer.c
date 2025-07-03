/**
 * @file bsp_multiplexer.c
 * @brief This file provides the functionality of the multiplexer
 * @copyright Copyright (c) 2024
 */
#include "bsp_multiplexer.h"
#include "bsp_config.h"

/**
 * @brief Enable / disable the output multiplexer.
 *
 * @param enable Enable / disable
 */
void bsp_mux_enable(bool enable) {
	HAL_GPIO_WritePin(MUX_EN_GPIO_Port, MUX_EN_Pin, (enable)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
}

/**
 * @brief Select the SR channels of the output multiplexer.
 *
 * @param sr1 U1301 IN1 & IN2
 * @param sr2 U1301 IN3 & IN4
 */
void bsp_mux_sr_sel_set(bool sr1, bool sr2) {
	HAL_GPIO_WritePin(SR_SEL1_GPIO_Port, SR_SEL1_Pin, (sr1)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_GPIO_WritePin(SR_SEL2_GPIO_Port, SR_SEL2_Pin, (sr2)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
}

/**
 * @brief Select the ECG channels of the output multiplexer.
 *
 * @param ecg U1302 IN1 & IN2
 */
void bsp_mux_ecg_sel_set(bool ecg) {
	HAL_GPIO_WritePin(ECG_SEL_GPIO_Port, ECG_SEL_Pin, (ecg)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
}
