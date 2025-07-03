/**
 * @file app_config.h
 * @brief Configuration of applications in firmware
 * @copyright Copyright (c) 2024
 */
#ifndef INC_APP_CONFIG_APP_CONFIG_H_
#define INC_APP_CONFIG_APP_CONFIG_H_

#include "bsp_config.h"

#include "app_func_authentication.h"
#include "app_func_ble.h"
#include "app_func_command.h"
#include "app_func_logs.h"
#include "app_func_measurement.h"
#include "app_func_parameter.h"
#include "app_func_state_machine.h"
#include "app_func_stimulation.h"

#include "app_mode_battery_test.h"
#include "app_mode_ble_active.h"
#include "app_mode_ble_connection.h"
#include "app_mode_bsl.h"
#include "app_mode_impedance_test.h"
#include "app_mode_oad.h"
#include "app_mode_therapy_session.h"
#include "app_mode_dvt.h"
#include "app_state.h"
#include "app.h"

#include "eeprom_emul.h"
#include "ecc_publickey.h"
#include "prime256v1.h"

#define	APP_FW_VER_STR						"1.0.00"            /*!< The version of the firmware */

#ifdef DVT
#define	DEFAULT_STATE	STATE_ACT_MODE_DVT			/*!< The default state after power-on */
#else
#define	DEFAULT_STATE	STATE_ACT_MODE_BLE_ACT		/*!< The default state after power-on */
#endif

#ifdef USE_UNITY
extern void unity_main(void);
#define UNITY_TEST()    unity_main()
#else
#define UNITY_TEST()    (void)0
#endif

#endif /* INC_APP_CONFIG_APP_CONFIG_H_ */
