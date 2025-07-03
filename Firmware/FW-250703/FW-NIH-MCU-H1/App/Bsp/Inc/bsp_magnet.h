/**
 * @file bsp_magnet.h
 * @brief This file contains all the function prototypes for the app_func_magnet.c file
 * @copyright Copyright (c) 2024
 */
#ifndef APP_FUNC_MAGNET_H_
#define APP_FUNC_MAGNET_H_
#include <stdbool.h>
#include <stdint.h>

typedef void (*Magnet_Lost_Callback)(uint8_t detected_time);	/*!< The format of the magnet lost callback */

/**
 * @brief Initialization of magnet
 *
 */
void bsp_magnet_init(Magnet_Lost_Callback mag_lost_cb);

/**
 * @brief Magnet detected timer callback, unit: sec
 * 
 */
void bsp_magnet_timer_cb(void);

/**
 * @brief Callback when magnet detection changes
 *
 * @param detected Detected or not
 */
void bsp_magnet_detect_cb(bool detected);

#endif /* APP_FUNC_MAGNET_H_ */
