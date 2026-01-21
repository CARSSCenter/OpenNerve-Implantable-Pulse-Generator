/**
 * @file app_state.c
 * @brief This file provides management of all states
 * @copyright Copyright (c) 2024
 */
#include "app_state.h"
#include "app_config.h"

extern bool schd_therapy_enable;

/**
 * @brief Turn off the power to all peripheral circuits
 * 
 */
static void app_state_power_off(void) {
	app_func_stim_off();
	app_func_meas_off();
	app_func_ble_enable(false);
}

/**
 * @brief Handler for shutdown state
 * 
 */
void app_state_shutdown_handler(void) {
	bsp_fram_deinit();
	bsp_sp_deinit();
	app_state_power_off();

	HAL_GPIO_WritePin(IPG_SHDN_GPIO_Port, IPG_SHDN_Pin, GPIO_PIN_SET);
}

/**
 * @brief Handler for sleep state
 * 
 */
void app_state_sleep_handler(void) {
	bsp_wdg_refresh();
	bsp_fram_deinit();
	bsp_sp_deinit();
	app_state_power_off();

	HAL_SuspendTick();
	__HAL_RCC_LPTIM1_CLKAM_ENABLE();
	__HAL_RCC_LPTIM3_CLKAM_ENABLE();
	__HAL_RCC_LPTIM4_CLKAM_ENABLE();

	HAL_ERROR_CHECK(HAL_LPTIM_Counter_Start_IT(&HANDLE_WAKEUP_LPTIM));
	HAL_ERROR_CHECK(HAL_LPTIM_Counter_Start_IT(&HANDLE_WDG_REFRESH_LPTIM));

	_Float64 rtc_interrupt_period_f = 0.0;
	app_func_para_data_get((const uint8_t*)HPID_RTC_INTERRUPT_PERIOD, (uint8_t*)&rtc_interrupt_period_f, (uint8_t)sizeof(rtc_interrupt_period_f));
	uint32_t rtc_interrupt_period = (uint32_t)rtc_interrupt_period_f;
	if (rtc_interrupt_period > 0) {
		HAL_ERROR_CHECK(HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, rtc_interrupt_period - 1, RTC_WAKEUPCLOCK_CK_SPRE_16BITS, 0));
	}

	app_func_sm_schd_therapy_enable(schd_therapy_enable);

	uint16_t curr_state = app_func_sm_current_state_get();
	while (curr_state == STATE_SLEEP) {
		HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
		curr_state = app_func_sm_current_state_get();
	}

	HAL_ERROR_CHECK(HAL_LPTIM_Counter_Stop_IT(&HANDLE_WAKEUP_LPTIM));
	HAL_ERROR_CHECK(HAL_LPTIM_Counter_Stop_IT(&HANDLE_WDG_REFRESH_LPTIM));
	HAL_ERROR_CHECK(HAL_RTCEx_DeactivateWakeUpTimer(&hrtc));
	app_func_sm_schd_therapy_enable(false);

	SystemClock_Config();
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_STOPF);
	HAL_ResumeTick();

	bsp_wdg_refresh();
	bsp_sp_init(&app_func_command_parser, &bsp_fram_write_cplt_cb);
	bsp_fram_init(&app_func_logs_write_cplt_cb);
}

/**
 * @brief Handler for active state (No operating mode)
 * 
 */
void app_state_active_handler(void) {
	_Float64 idle_duration_ms_f = 0.0;
	app_func_para_data_get((const uint8_t*)HPID_IDLE_DURATION, (uint8_t*)&idle_duration_ms_f, (uint8_t)sizeof(idle_duration_ms_f));
	idle_duration_ms_f *= 1000.0;
	uint32_t idle_duration_ms = (uint32_t)idle_duration_ms_f;

	app_state_power_off();
	bsp_wdg_enable(false);
	HAL_Delay(idle_duration_ms);
	bsp_wdg_enable(true);

	app_func_sm_current_state_set(STATE_SLEEP);
}
