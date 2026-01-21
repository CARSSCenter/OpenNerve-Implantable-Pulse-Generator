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

#define MONTH_IS_JAN (__DATE__[0]=='J' && __DATE__[1]=='a' && __DATE__[2]=='n')
#define MONTH_IS_FEB (__DATE__[0]=='F')
#define MONTH_IS_MAR (__DATE__[0]=='M' && __DATE__[1]=='a' && __DATE__[2]=='r')
#define MONTH_IS_APR (__DATE__[0]=='A' && __DATE__[1]=='p')
#define MONTH_IS_MAY (__DATE__[0]=='M' && __DATE__[1]=='a' && __DATE__[2]=='y')
#define MONTH_IS_JUN (__DATE__[0]=='J' && __DATE__[1]=='u' && __DATE__[2]=='n')
#define MONTH_IS_JUL (__DATE__[0]=='J' && __DATE__[1]=='u' && __DATE__[2]=='l')
#define MONTH_IS_AUG (__DATE__[0]=='A' && __DATE__[1]=='u')
#define MONTH_IS_SEP (__DATE__[0]=='S')
#define MONTH_IS_OCT (__DATE__[0]=='O')
#define MONTH_IS_NOV (__DATE__[0]=='N')
#define MONTH_IS_DEC (__DATE__[0]=='D')

#define BUILD_MONTH ( \
    MONTH_IS_JAN?1:MONTH_IS_FEB?2:MONTH_IS_MAR?3:MONTH_IS_APR?4:MONTH_IS_MAY?5:MONTH_IS_JUN?6: \
    MONTH_IS_JUL?7:MONTH_IS_AUG?8:MONTH_IS_SEP?9:MONTH_IS_OCT?10:MONTH_IS_NOV?11:12 )

#define BUILD_DAY ((__DATE__[4]==' '? __DATE__[5]-'0' : (__DATE__[4]-'0')*10 + (__DATE__[5]-'0')))

#define BUILD_YEAR ((__DATE__[9]-'0')*10 + (__DATE__[10]-'0'))

 /*!< Firmware version based on compilation date */
#define APP_FW_VER_STR \
    { \
        (char)(BUILD_YEAR/10+'0'), \
        (char)(BUILD_YEAR%10+'0'), \
        (char)(BUILD_MONTH/10+'0'), \
        (char)(BUILD_MONTH%10+'0'), \
        (char)(BUILD_DAY/10+'0'), \
        (char)(BUILD_DAY%10+'0') \
    }

#ifdef DVT
#define	DEFAULT_STATE	STATE_ACT_MODE_DVT			/*!< The default state after power-on */
#else
#define	DEFAULT_STATE	STATE_ACT_MODE_BLE_ACT		/*!< The default state after power-on */
#endif

//#define SWV_TRACE

#ifdef USE_UNITY
extern void unity_main(void);
#define UNITY_TEST()    unity_main()
#else
#define UNITY_TEST()    (void)0
#endif

#endif /* INC_APP_CONFIG_APP_CONFIG_H_ */
