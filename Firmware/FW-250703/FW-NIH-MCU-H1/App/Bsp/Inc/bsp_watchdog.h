/**
 * @file bsp_watchdog.h
 * @brief This file contains all the function prototypes for the bsp_watchdog.c file
 * @copyright Copyright (c) 2024
 */
#ifndef BSP_INC_BSP_WATCHDOG_H_
#define BSP_INC_BSP_WATCHDOG_H_
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Enable / Disable the watchdog
 * 
 * @param enable Enable / Disable
 */
void bsp_wdg_enable(bool enable);

/**
 * @brief Refresh the watchdog
 * 
 */
void bsp_wdg_refresh(void);

#endif /* BSP_INC_BSP_WATCHDOG_H_ */
