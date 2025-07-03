/**
 * @file bsp_multiplexer.h
 * @brief This file contains all the function prototypes for the bsp_multiplexer.c file
 * @copyright Copyright (c) 2024
 */
#ifndef BSP_INC_BSP_MULTIPLEXER_H_
#define BSP_INC_BSP_MULTIPLEXER_H_
#include <stdbool.h>

#define	SEL_SR_EMG2	true
#define	SEL_SR_STIM	false

#define	SEL_ECG_RR	true
#define	SEL_ECG_HR	false

/**
 * @brief Enable / disable the output multiplexer.
 *
 * @param enable Enable / disable
 */
void bsp_mux_enable(bool enable);

/**
 * @brief Select the SR channels of the output multiplexer.
 *
 * @param sr1 U1301 IN1 & IN2
 * @param sr2 U1301 IN3 & IN4
 */
void bsp_mux_sr_sel_set(bool sr1, bool sr2);

/**
 * @brief Select the ECG channels of the output multiplexer.
 *
 * @param ecg U1302 IN1 & IN2
 */
void bsp_mux_ecg_sel_set(bool ecg);

#endif /* BSP_INC_BSP_MULTIPLEXER_H_ */
