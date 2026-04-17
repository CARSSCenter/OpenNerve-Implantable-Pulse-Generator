/**
 * @file app_func_state_machine.c
 * @brief This file provides state machine management
 * @copyright Copyright (c) 2024
 */
#include "app_func_state_machine.h"
#include "app_config.h"

Sys_Config_t sc = {0};

uint16_t curr_state = STATE_INVALID;

int32_t impedance_test_hour_timer = -1;
int32_t battery_test_hour_timer = -1;

const uint8_t* stid_start[6] = {
		(const uint8_t*)SPID_THERAPY_SESSION_1_START,
		(const uint8_t*)SPID_THERAPY_SESSION_2_START,
		(const uint8_t*)SPID_THERAPY_SESSION_3_START,
		(const uint8_t*)SPID_THERAPY_SESSION_4_START,
		(const uint8_t*)SPID_THERAPY_SESSION_5_START,
		(const uint8_t*)SPID_THERAPY_SESSION_6_START,
};

const uint8_t* stid_stop[6] = {
		(const uint8_t*)SPID_THERAPY_SESSION_1_STOP,
		(const uint8_t*)SPID_THERAPY_SESSION_2_STOP,
		(const uint8_t*)SPID_THERAPY_SESSION_3_STOP,
		(const uint8_t*)SPID_THERAPY_SESSION_4_STOP,
		(const uint8_t*)SPID_THERAPY_SESSION_5_STOP,
		(const uint8_t*)SPID_THERAPY_SESSION_6_STOP,
};

uint16_t schd_therapy_start_minute = 0;
uint16_t schd_therapy_stop_minute = 0;

bool schd_therapy_enable = false;

/**
 * @brief Set the current application state
 *
 * @param state current application state
 */
void app_func_sm_current_state_set(uint16_t state) {
	curr_state = state;
}

/**
 * @brief Get the current application state
 *
 * @return uint16_t current application state
 */
uint16_t app_func_sm_current_state_get(void) {
	return curr_state;
}

/**
 * @brief Enable timer of impedance test.
 *
 */
void app_func_sm_impedance_timer_enable(void) {
	_Float64 impedance_test_interval = 0.0;
	app_func_para_data_get((const uint8_t*)HPID_IMPEDANCE_TEST_INTERVAL, (uint8_t*)&impedance_test_interval, (uint8_t)sizeof(impedance_test_interval));
	impedance_test_hour_timer = (int32_t)impedance_test_interval;
}

/**
 * @brief Enable timer of battery test.
 *
 */
void app_func_sm_battery_timer_enable(void) {
	_Float64 batterry_test_interval = 0.0;
	app_func_para_data_get((const uint8_t*)HPID_IMPEDANCE_TEST_INTERVAL, (uint8_t*)&batterry_test_interval, (uint8_t)sizeof(batterry_test_interval));
	battery_test_hour_timer = (int32_t)batterry_test_interval;
}

/**
 * @brief Initialization of the state machine management
 *
 */
void app_func_sm_init(void) {
	bsp_fram_read(ADDR_SYS_CONFIG, (uint8_t*)&sc, sizeof(sc));
	if (sc.DefaultState == STATE_INVALID || sc.StartState == STATE_INVALID) {
		sc.DefaultState = DEFAULT_STATE;
		sc.StartState = DEFAULT_STATE;
		bsp_fram_write(ADDR_SYS_CONFIG, (uint8_t*)&sc, sizeof(sc), true);
		curr_state = DEFAULT_STATE;
	}
	else if (sc.DefaultState != DEFAULT_STATE) {
		sc.DefaultState = DEFAULT_STATE;
		sc.StartState = DEFAULT_STATE;
		bsp_fram_write(ADDR_SYS_CONFIG, (uint8_t*)&sc, sizeof(sc), true);
		curr_state = DEFAULT_STATE;
	}
	else {
		curr_state = sc.StartState;
	}

	if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST)) {
		app_func_logs_event_write(EVENT_UNRESPONSIVE_FUNCTION, NULL);
	}
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_OBLRST)) {
		if (curr_state != STATE_ACT_MODE_DVT) {
			curr_state = STATE_ACT;
		}
	}
	__HAL_RCC_CLEAR_RESET_FLAGS();

	if (curr_state != STATE_ACT_MODE_DVT && curr_state != STATE_SHUTDOWN) {
		uint32_t eos_counter = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR2);
		if (eos_counter >= COUNT_MAX_EOS) {
			curr_state = STATE_SLEEP;
		}
	}

	app_func_sm_impedance_timer_enable();
	app_func_sm_battery_timer_enable();

	switch (curr_state) {
	case STATE_ACT_MODE_IMPED_TEST:
		impedance_test_hour_timer = -1;
		break;

	case STATE_ACT_MODE_BATT_TEST:
		battery_test_hour_timer = -1;
		break;

	case STATE_ACT_MODE_DVT:
		impedance_test_hour_timer = -1;
		battery_test_hour_timer = -1;
		break;

	default:

		break;
	}
}

/**
 * @brief Enable / Disable scheduled therapy session
 *
 * @param enable Enable / Disable
 */
void app_func_sm_schd_therapy_enable(bool enable) {
	schd_therapy_enable = enable;

	if (schd_therapy_enable) {
		_Float64 number_of_therapy_sessions = 0.0;
		app_func_para_data_get((const uint8_t*)SPID_NUMBER_OF_THERAPY_SESSIONS, (uint8_t*)&number_of_therapy_sessions, (uint8_t)sizeof(number_of_therapy_sessions));
		uint8_t num = (uint8_t)number_of_therapy_sessions;

		if ((num >= 1U) && (num <= 6U)) {
			_Float64 therapy_session_start = 0.0;
			app_func_para_data_get(stid_start[num-1U], (uint8_t*)&therapy_session_start, (uint8_t)sizeof(therapy_session_start));
			schd_therapy_start_minute = (uint16_t)therapy_session_start;

			_Float64 therapy_session_stop = 0.0;
			app_func_para_data_get(stid_stop[num-1U], (uint8_t*)&therapy_session_stop, (uint8_t)sizeof(therapy_session_stop));
			schd_therapy_stop_minute = (uint16_t)therapy_session_stop;
		}
		else {
			schd_therapy_enable = false;
		}
	}
}

/**
 * @brief Callback for wakeup timers in sleep state
 *
 */
void app_func_sm_wakeup_timer_cb(void) {
	if (impedance_test_hour_timer > 0) {
		impedance_test_hour_timer--;
	}

	if (battery_test_hour_timer > 0) {
		battery_test_hour_timer--;
	}
}

/**
 * @brief Check battery voltage for EOS during active operation.
 *
 * Rate-limited to once per minute via HAL_GetTick(). Skipped in WPT, DVT,
 * sleep, and shutdown states. Uses the same ER/EOS hysteresis and RTC backup
 * registers as app_mode_battery_test_handler() so both paths share a single
 * counter. Transitions to STATE_SLEEP and logs EVENT_EOS after COUNT_MAX_EOS
 * consecutive readings at or below HPID_BATTERY_EOS_LEVEL.
 */
void app_func_sm_active_eos_check(void) {
	static uint32_t last_check_ms = 0;
	uint32_t now = HAL_GetTick();
	if ((now - last_check_ms) < 60000U) return;
	last_check_ms = now;

	if (curr_state == STATE_ACT_MODE_WPT_HIGH  ||
	    curr_state == STATE_ACT_MODE_WPT_PAUSED ||
	    curr_state == STATE_ACT_MODE_DVT        ||
	    curr_state == STATE_SLEEP               ||
	    curr_state == STATE_SHUTDOWN) return;

	_Float64 battery_er_level  = 0.0;
	_Float64 battery_eos_level = 0.0;
	app_func_para_data_get((const uint8_t*)HPID_BATTERY_ER_LEVEL,  (uint8_t*)&battery_er_level,  (uint8_t)sizeof(battery_er_level));
	app_func_para_data_get((const uint8_t*)HPID_BATTERY_EOS_LEVEL, (uint8_t*)&battery_eos_level, (uint8_t)sizeof(battery_eos_level));

	uint16_t vbatA = 0, vbatB = 0;
	app_func_meas_batt_mon_enable(true);
	HAL_Delay(10);
	app_func_meas_batt_mon_meas(&vbatA, &vbatB);
	app_func_meas_batt_mon_enable(false);

	_Float64 battery_level = (vbatA >= vbatB)
	    ? ((_Float64)vbatA / 1000.0)
	    : ((_Float64)vbatB / 1000.0);

	uint32_t er_counter  = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1);
	uint32_t eos_counter = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR2);

	if (battery_level > battery_er_level) {
		er_counter  = 0;
		eos_counter = 0;
	} else if (battery_level > battery_eos_level) {
		er_counter++;
	} else {
		er_counter++;
		eos_counter++;
		if (eos_counter >= COUNT_MAX_EOS) {
			app_func_logs_event_write((const char*)EVENT_EOS, NULL);
			curr_state = STATE_SLEEP;
		}
	}

	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, er_counter);
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, eos_counter);
}

/**
 * @brief Callback when VRECT coil is detected or lost
 *
 * @param coil_present true = coil present (VRECT_DETn low), false = coil removed (VRECT_DETn high)
 */
void app_func_sm_vrect_coil_cb(bool coil_present) {
	if (coil_present) {
		if ((curr_state == STATE_SLEEP || curr_state >= STATE_ACT) && curr_state != STATE_SHUTDOWN && curr_state != STATE_ACT_MODE_DVT) {
			if (curr_state == STATE_SLEEP) {
				/* Wake via BLE_ACT so BLE is fully cold-started before entering
				 * WPT mode. app_mode_ble_act_handler polls VRECT_DETn every 50 ms
				 * and will self-transition to STATE_ACT_MODE_WPT_HIGH immediately. */
				curr_state = STATE_ACT_MODE_BLE_ACT;
			} else {
				curr_state = STATE_ACT_MODE_WPT_HIGH;
			}
		}
	} else {
		if (curr_state == STATE_ACT_MODE_WPT_HIGH || curr_state == STATE_ACT_MODE_WPT_PAUSED) {
			curr_state = STATE_ACT_MODE_BLE_ACT;
		}
	}
}

/**
 * @brief Callback for confirmation timer in sleep state
 *
 */
void app_func_sm_confirmation_timer_cb(void) {
	/* Battery voltage is elevated during WPT charging — suppress test transitions */
	if (curr_state == STATE_ACT_MODE_WPT_HIGH || curr_state == STATE_ACT_MODE_WPT_PAUSED) {
		return;
	}

	RTC_TimeTypeDef curr_time;
	RTC_DateTypeDef curr_date;
	HAL_ERROR_CHECK(HAL_RTC_GetTime(&hrtc, &curr_time, RTC_FORMAT_BIN));
	HAL_ERROR_CHECK(HAL_RTC_GetDate(&hrtc, &curr_date, RTC_FORMAT_BIN));
	uint16_t current_minute = (curr_time.Hours*60) + curr_time.Minutes;

	if (curr_state == STATE_SLEEP) {
		if (impedance_test_hour_timer == 0) {
			curr_state = STATE_ACT_MODE_IMPED_TEST;
			impedance_test_hour_timer = -1;
		}
		else if (battery_test_hour_timer == 0) {
			curr_state = STATE_ACT_MODE_BATT_TEST;
			battery_test_hour_timer = -1;
		}
		else if (schd_therapy_enable) {
			if (schd_therapy_start_minute == current_minute) {
				curr_state = STATE_ACT_MODE_THERAPY_SESSION;
			}
		}
	}
	else if (curr_state == STATE_ACT_MODE_THERAPY_SESSION) {
		if (schd_therapy_stop_minute == current_minute) {
			curr_state = STATE_ACT;
		}
	}
}

/**
 * @brief Callback when magnet lost
 *
 * @param detected_time Detected time
 */
void app_func_sm_magnet_lost_cb(uint8_t detected_time) {
	if (curr_state != STATE_SHUTDOWN && curr_state != STATE_ACT_MODE_DVT) {
		_Float64 magnet_wakeup_min_time_f = 0.0;
		app_func_para_data_get((const uint8_t*)HPID_MAGNET_WAKEUP_MIN_TIME, (uint8_t*)&magnet_wakeup_min_time_f, (uint8_t)sizeof(magnet_wakeup_min_time_f));
		uint8_t magnet_wakeup_min_time = (uint8_t)magnet_wakeup_min_time_f;

		_Float64 magnet_wakeup_max_time_f = 0.0;
		app_func_para_data_get((const uint8_t*)HPID_MAGNET_WAKEUP_MAX_TIME, (uint8_t*)&magnet_wakeup_max_time_f, (uint8_t)sizeof(magnet_wakeup_max_time_f));
		uint8_t magnet_wakeup_max_time = (uint8_t)magnet_wakeup_max_time_f;

		if ((detected_time >= magnet_wakeup_min_time) && (detected_time <= magnet_wakeup_max_time)) {
			app_func_logs_event_write(EVENT_MAGNET_DETECTION, NULL);
			if (curr_state == STATE_SLEEP) {
				curr_state = STATE_ACT_MODE_BLE_ACT;
			}
			else if (curr_state >= STATE_ACT) {
				curr_state = STATE_SLEEP;
			}
		}
	}
}
