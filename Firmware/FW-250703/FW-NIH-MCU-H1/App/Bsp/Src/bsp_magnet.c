/**
 * @file bsp_magnet.c
 * @brief This file provides magnet detection functionality
 * @copyright Copyright (c) 2024
 */
#include "bsp_magnet.h"
#include "app_config.h"

int32_t magnet_detected_sec_timer = -1;
Magnet_Lost_Callback magLostCallback = NULL;

/**
 * @brief Initialization of magnet
 *
 */
void bsp_magnet_init(Magnet_Lost_Callback mag_lost_cb) {
	magLostCallback = mag_lost_cb;
}

/**
 * @brief Magnet detected timer callback, unit: sec
 * 
 */
void bsp_magnet_timer_cb(void) {
	if (magnet_detected_sec_timer >= 0) {
		magnet_detected_sec_timer++;
	}
}

/**
 * @brief Callback when magnet detection changes
 * 
 * @param detected Detected or not
 */
void bsp_magnet_detect_cb(bool detected) {
	if (detected) {
		magnet_detected_sec_timer = 0;
		HAL_ERROR_CHECK(HAL_LPTIM_Counter_Start_IT(&HANDLE_MAGNET_LPTIM));
	}
	else {
		HAL_ERROR_CHECK(HAL_LPTIM_Counter_Stop_IT(&HANDLE_MAGNET_LPTIM));
		uint8_t magnet_detected_sec_time = (uint8_t)magnet_detected_sec_timer;
		magnet_detected_sec_timer = -1;
		if (magLostCallback != NULL) {
			magLostCallback(magnet_detected_sec_time);
		}
	}
}

/**
  * @brief  EXTI line rising detection callback.
  * @param  GPIO_Pin: Specifies the port pin connected to corresponding EXTI line.
  * @retval None
  */
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == MAG_DET_Pin) {
		bsp_magnet_detect_cb(true);
	}
}

/**
  * @brief  EXTI line falling detection callback.
  * @param  GPIO_Pin: Specifies the port pin connected to corresponding EXTI line.
  * @retval None
  */
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == MAG_DET_Pin) {
		bsp_magnet_detect_cb(false);
	}
}
