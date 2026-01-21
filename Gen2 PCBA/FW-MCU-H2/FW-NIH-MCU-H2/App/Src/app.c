/**
 * @file app.c
 * @brief This file provides management of the application
 * @copyright Copyright (c) 2024
 */
#include "app.h"
#include "app_config.h"

/**
 * @brief Callback when magnet lost
 *
 * @param detected_time Detected time
 */
static void app_magnet_lost_cb(uint8_t detected_time) {
	app_func_sm_magnet_lost_cb(detected_time);
	app_func_ble_magnet_lost_cb(detected_time);
}

/**
 * @brief Initialization for application
 *
 */
void app_init(void) {
	UNITY_TEST();

	bsp_sp_init(&app_func_command_parser, &bsp_fram_write_cplt_cb);
	bsp_fram_init(&app_func_logs_write_cplt_cb);
	bsp_magnet_init(&app_magnet_lost_cb);
	bsp_adc_init();

	app_func_logs_init();
	app_func_para_init();
	app_func_sm_init();

	bsp_wdg_enable(true);
}

/**
 * @brief Handler for application
 *
 */
void app_handler(void) {
	bsp_wdg_refresh();
	switch(app_func_sm_current_state_get()) {
	case STATE_SHUTDOWN:
		app_state_shutdown_handler();
		break;

	case STATE_SLEEP:
		app_state_sleep_handler();
		break;

	case STATE_ACT:
		app_state_active_handler();
		break;

	case STATE_ACT_MODE_BLE_ACT:
		app_mode_ble_act_handler();
		break;

	case STATE_ACT_MODE_BLE_CONN:
		app_mode_ble_conn_handler();
		break;

	case STATE_ACT_MODE_THERAPY_SESSION:
		app_mode_therapy_handler();
		break;

	case STATE_ACT_MODE_IMPED_TEST:
		app_mode_impedance_test_handler();
		break;

	case STATE_ACT_MODE_BATT_TEST:
		app_mode_battery_test_handler();
		break;

	case STATE_ACT_MODE_OAD:
		app_mode_oad_handler();
		break;

	case STATE_ACT_MODE_BSL:
		app_mode_bsl_handler();
		break;

	case STATE_ACT_MODE_DVT:
		app_mode_dvt_handler();
		break;

	default:
		break;
	}
}

/**
  * @brief This function is called to increment a global variable "uwTick"
  *        used as application time base.
  * @note In the default implementation, this variable is incremented each 1ms
  *       in Systick ISR.
  * @note This function is declared as __weak to be overwritten in case of other
  *      implementations in user file.
  * @retval None
  */
void HAL_IncTick(void)
{
  uwTick += (uint32_t)uwTickFreq;

  app_mode_ble_act_timer_cb();
  app_mode_ble_conn_timer_cb();
  app_mode_dvt_acc_timer_cb();
}

/**
  * @brief Repetition counter underflowed (or contains zero) and LPTIM counter overflowed callback in non-blocking mode.
  * @param  hlptim LPTIM handle
  * @retval None
  */
void HAL_LPTIM_UpdateEventCallback(LPTIM_HandleTypeDef *hlptim) /* parasoft-suppress MISRAC2012-RULE_8_13-a "This definition comes from HAL." */
{
	if (hlptim == &HANDLE_WAKEUP_LPTIM) {
		app_func_sm_wakeup_timer_cb();
	}
	else if (hlptim == &HANDLE_MAGNET_LPTIM) {
		bsp_magnet_timer_cb();
	}
	else if (hlptim == &HANDLE_BLE_DEFAULT_LPTIM) {
		app_func_ble_default_timer_cb();
	}
	else if (hlptim == &HANDLE_WDG_REFRESH_LPTIM) {
		bsp_wdg_refresh();
	}
}

/**
  * @brief  Wake Up Timer callback.
  * @param  hrtc RTC handle
  * @retval None
  */
void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
  UNUSED(hrtc);

  app_func_sm_confirmation_timer_cb();
}

/**
  * @brief  PWM Pulse finished callback in non-blocking mode
  * @param  htim TIM handle
  * @retval None
  */
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) /* parasoft-suppress MISRAC2012-RULE_1_1-b "This definition comes from HAL." */ /* parasoft-suppress MISRAC2012-RULE_1_1-a "This definition comes from HAL." */ /* parasoft-suppress MISRAC2012-RULE_8_13-a "This definition comes from HAL." */
{
	if (htim == &HANDLE_PULSE1_TIM && htim->Channel == TIM_ACH_PULSE1_TO_LOW) {
		app_func_stim_stim1_cb(TO_LOW);
	}
	else if (htim == &HANDLE_PULSE2_TIM && htim->Channel == TIM_ACH_PULSE2_TO_LOW) {
		app_func_stim_stim2_cb(TO_LOW);
	}
	else if (htim == &HANDLE_SINE_TIM && htim->Channel == TIM_ACH_SINE_POLR) {
		app_func_stim_sine_cb(POLR_NEG);
	}
}

/**
  * @brief  Period elapsed callback in non-blocking mode
  * @param  htim TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) /* parasoft-suppress MISRAC2012-RULE_8_13-a "This definition comes from HAL." */
{
	if (htim == &HANDLE_PULSE1_TIM) {
		app_func_stim_stim1_cb(TO_HIGH);
	}
	else if (htim == &HANDLE_PULSE2_TIM) {
		app_func_stim_stim2_cb(TO_HIGH);
	}
	else if (htim == &HANDLE_SINE_TIM) {
		app_func_stim_sine_cb(POLR_POS);
	}
	else if (htim == &HANDLE_ADC1_SAMPLE_TIM || htim == &HANDLE_ADC4_SAMPLE_TIM) {
		bsp_wdg_refresh();
	}
}

/**
  * @brief  Output Compare callback in non-blocking mode
  * @param  htim TIM OC handle
  * @retval None
  */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == &HANDLE_SINE_TIM && htim->Channel == TIM_ACH_SINE_AMP) {
		app_func_stim_sine_cb(AMP);
	}
	else if (htim == &HANDLE_PULSE1_TIM && htim->Channel == TIM_ACH_PULSE1_BEF_HI) {
		app_func_stim_stim1_cb(BEFORE_HIGH);
	}
	else if (htim == &HANDLE_PULSE2_TIM && htim->Channel == TIM_ACH_PULSE2_BEF_HI) {
		app_func_stim_stim2_cb(BEFORE_HIGH);
	}
}
