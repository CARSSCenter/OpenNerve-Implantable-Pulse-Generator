/**
 * @file bsp_watchdog.c
 * @brief This file provides watchdog management
 * @copyright Copyright (c) 2024
 */
#include "bsp_watchdog.h"
#include "bsp_config.h"

static bool wdg_enable = false;

/**
 * @brief Enable / Disable the watchdog
 * 
 * @param enable Enable / Disable
 */
void bsp_wdg_enable(bool enable) {
	HAL_ERROR_CHECK(HAL_IWDG_Refresh(&hiwdg));
	wdg_enable = enable;
}

/**
 * @brief Refresh the watchdog
 * 
 */
void bsp_wdg_refresh(void) {
	HAL_ERROR_CHECK(HAL_IWDG_Refresh(&hiwdg));
}

/**
  * @brief  IWDG Early Wakeup callback.
  * @param  hiwdg  pointer to a IWDG_HandleTypeDef structure that contains
  *                the configuration information for the specified IWDG module.
  * @retval None
  */
void HAL_IWDG_EarlyWakeupCallback(IWDG_HandleTypeDef *hiwdg)
{
	if (!wdg_enable) {
		HAL_ERROR_CHECK(HAL_IWDG_Refresh(hiwdg));
	}
}
