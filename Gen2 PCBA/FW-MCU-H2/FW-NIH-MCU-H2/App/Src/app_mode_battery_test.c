/**
 * @file app_mode_battery_test.c
 * @brief This file provides management of the battery test mode
 * @copyright Copyright (c) 2024
 */
#include "app_mode_battery_test.h"
#include "app_config.h"

#define	COUNT_MAX_ER	3		/*!< The maximum count value that triggers the "ER" event */
#define	COUNT_MAX_EOS	3		/*!< The maximum count value that triggers the "EOS" event */

uint32_t battery_er_counter = 0;
uint32_t battery_eos_counter = 0;

/**
 * @brief Measure, obtain and record battery voltages
 * 
 * @param p_vbatA The battery voltage of battery 1, unit: mV
 * @param p_vbatB The battery voltage of battery 2, unit: mV
 */
void app_mode_battery_test_volt_get(uint16_t* p_vbatA, uint16_t* p_vbatB) {
	app_func_meas_batt_mon_enable(true);
	HAL_Delay(10);
	app_func_meas_batt_mon_meas(p_vbatA, p_vbatB);
	app_func_meas_batt_mon_enable(false);

	app_func_logs_batt_volt_write(*p_vbatA, *p_vbatB);
}

/**
 * @brief Handler for battery test mode
 * 
 */
void app_mode_battery_test_handler(void) {
	uint16_t curr_state = app_func_sm_current_state_get();

	_Float64 battery_er_level = 0.0;
	_Float64 battery_eos_level = 0.0;
	app_func_para_data_get((const uint8_t*)HPID_BATTERY_ER_LEVEL, (uint8_t*)&battery_er_level, (uint8_t)sizeof(battery_er_level));
	app_func_para_data_get((const uint8_t*)HPID_BATTERY_EOS_LEVEL, (uint8_t*)&battery_eos_level, (uint8_t)sizeof(battery_eos_level));

	uint16_t vbatA = 0, vbatB = 0;
	_Float64 battery_level = 0.0, batteryA_level = 0.0, batteryB_level = 0.0;

	app_mode_battery_test_volt_get(&vbatA, &vbatB);
	batteryA_level = ((_Float64)vbatA)/1000.0;
	batteryB_level = ((_Float64)vbatB)/1000.0;

	battery_er_counter 	= HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1);
	battery_eos_counter = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR2);

	if (batteryA_level >= batteryB_level) {
		battery_level = batteryA_level;
	}
	else {
		battery_level = batteryB_level;
	}

	if(battery_level > battery_er_level) {
		battery_er_counter = 0;
		battery_eos_counter = 0;
	}
	else if((battery_level <= battery_er_level) && (battery_level > battery_eos_level)) {
		battery_er_counter++;
	}
	else {
		battery_er_counter++;
		battery_eos_counter++;
	}

	if (battery_er_counter >= (uint8_t)COUNT_MAX_ER) {
		app_func_logs_event_write((const char*)EVENT_ER, NULL);
	}

	if (battery_eos_counter >= (uint8_t)COUNT_MAX_EOS) {
		app_func_logs_event_write((const char*)EVENT_EOS, NULL);
		curr_state = STATE_SHUTDOWN;
	}
	else {
		curr_state = STATE_ACT;
	}
	app_func_sm_current_state_set(curr_state);
	app_func_sm_battery_timer_enable();

	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, battery_er_counter);
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, battery_eos_counter);
}
