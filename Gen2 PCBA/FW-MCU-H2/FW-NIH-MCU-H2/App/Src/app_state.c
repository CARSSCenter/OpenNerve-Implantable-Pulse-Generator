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
	if (app_mode_therapy_confirm()) {
		app_func_logs_event_write(EVENT_STIM_STOP, NULL);
	}
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
	app_func_logs_event_write(EVENT_SLEEP, NULL);
	bsp_fram_deinit();
	bsp_sp_deinit();

	app_state_power_off();

	/* Set pins to hi-Z to prevent passive current paths during sleep.
	 * bsp_sp_deinit/app_func_ble_enable already handle their own pins;
	 * only additional pins that create leakage paths need handling here. */
	{
		/* PH1_EN is initialized HIGH and never cleared by stim_off — drive LOW first */
		HAL_GPIO_WritePin(PH1_EN_GPIO_Port, PH1_EN_Pin, GPIO_PIN_RESET);

		GPIO_InitTypeDef GPIO_InitStruct = {0};
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;

		/* BLE handshake inputs: internal pull-ups inject ~0.8V into VDD_BLE via nRF ESD diodes */
		GPIO_InitStruct.Pin = BLE_P_1_Pin;       HAL_GPIO_Init(BLE_P_1_GPIO_Port,       &GPIO_InitStruct);
		GPIO_InitStruct.Pin = BLE_P_2_Pin;        HAL_GPIO_Init(BLE_P_2_GPIO_Port,       &GPIO_InitStruct);
		/* BLE_PWRn remains driven HIGH — P-FET gate must not float */
		/* SPI1_BLE_CSn is already LOW from bsp_sp_deinit; remove drive */
		GPIO_InitStruct.Pin = SPI1_BLE_CSn_Pin;  HAL_GPIO_Init(SPI1_BLE_CSn_GPIO_Port,  &GPIO_InitStruct);

		/* Stimulation phase enables */
		GPIO_InitStruct.Pin = PH1_EN_Pin;         HAL_GPIO_Init(PH1_EN_GPIO_Port,        &GPIO_InitStruct);
		GPIO_InitStruct.Pin = PH2_EN_Pin;         HAL_GPIO_Init(PH2_EN_GPIO_Port,        &GPIO_InitStruct);
		/* MUX_ENn: hi-Z only — stim_off already set it HIGH (ADG1634 disabled) */
		GPIO_InitStruct.Pin = MUX_ENn_Pin;        HAL_GPIO_Init(MUX_ENn_GPIO_Port,       &GPIO_InitStruct);

		/* Stimulation current sink pins */
		GPIO_InitStruct.Pin = SNK1_Pin;           HAL_GPIO_Init(SNK1_GPIO_Port,          &GPIO_InitStruct);
		GPIO_InitStruct.Pin = SNK2_Pin;           HAL_GPIO_Init(SNK2_GPIO_Port,          &GPIO_InitStruct);
		GPIO_InitStruct.Pin = SNK3_Pin;           HAL_GPIO_Init(SNK3_GPIO_Port,          &GPIO_InitStruct);
		GPIO_InitStruct.Pin = SNK4_Pin;           HAL_GPIO_Init(SNK4_GPIO_Port,          &GPIO_InitStruct);
		GPIO_InitStruct.Pin = SNK5_Pin;           HAL_GPIO_Init(SNK5_GPIO_Port,          &GPIO_InitStruct);

		/* Output multiplexer channel selects */
		GPIO_InitStruct.Pin = STIM_SEL_CH1n_Pin;  HAL_GPIO_Init(STIM_SEL_CH1n_GPIO_Port,  &GPIO_InitStruct);
		GPIO_InitStruct.Pin = STIM_SEL_CH2n_Pin;  HAL_GPIO_Init(STIM_SEL_CH2n_GPIO_Port,  &GPIO_InitStruct);
		GPIO_InitStruct.Pin = STIM_SEL_CH3n_Pin;  HAL_GPIO_Init(STIM_SEL_CH3n_GPIO_Port,  &GPIO_InitStruct);
		GPIO_InitStruct.Pin = STIM_SEL_CH4n_Pin;  HAL_GPIO_Init(STIM_SEL_CH4n_GPIO_Port,  &GPIO_InitStruct);
		GPIO_InitStruct.Pin = STIM_SEL_ENCLn_Pin; HAL_GPIO_Init(STIM_SEL_ENCLn_GPIO_Port, &GPIO_InitStruct);
		GPIO_InitStruct.Pin = STIMA_SELn_Pin;      HAL_GPIO_Init(STIMA_SELn_GPIO_Port,     &GPIO_InitStruct);
		GPIO_InitStruct.Pin = STIMB_SELn_Pin;      HAL_GPIO_Init(STIMB_SELn_GPIO_Port,     &GPIO_InitStruct);

		/* Impedance measurement selects */
		GPIO_InitStruct.Pin = IMP_EN_Pin;          HAL_GPIO_Init(IMP_EN_GPIO_Port,          &GPIO_InitStruct);
		GPIO_InitStruct.Pin = IMP_IN_P_SEL_Pin;    HAL_GPIO_Init(IMP_IN_P_SEL_GPIO_Port,    &GPIO_InitStruct);
		GPIO_InitStruct.Pin = IMP_IN_N_SEL0_Pin;   HAL_GPIO_Init(IMP_IN_N_SEL0_GPIO_Port,   &GPIO_InitStruct);
		GPIO_InitStruct.Pin = IMP_IN_N_SEL1_Pin;   HAL_GPIO_Init(IMP_IN_N_SEL1_GPIO_Port,   &GPIO_InitStruct);
		GPIO_InitStruct.Pin = IMP_IN_N_SEL2_Pin;   HAL_GPIO_Init(IMP_IN_N_SEL2_GPIO_Port,   &GPIO_InitStruct);
	}

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
	PeriphCommonClock_Config();
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_STOPF);
	HAL_ResumeTick();

	/* Restore all pins that were set to hi-Z during sleep.
	 * bsp_sp_init (called below) handles I2C2, SPI1, and SPI1_BLE_CSn.
	 * All other floated pins must be explicitly restored here before any
	 * application code attempts to use them. */
	{

		GPIO_InitTypeDef GPIO_InitStruct = {0};
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

		/* BLE handshake inputs */
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Pin  = BLE_P_1_Pin | BLE_P_2_Pin;
		HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

		/* SPI1_BLE_CSn: restore output mode; bsp_sp_init will drive it HIGH */
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Pin  = SPI1_BLE_CSn_Pin;
		HAL_GPIO_Init(SPI1_BLE_CSn_GPIO_Port, &GPIO_InitStruct);

		/* Stimulation phase enables */
		GPIO_InitStruct.Pin = PH1_EN_Pin;  HAL_GPIO_Init(PH1_EN_GPIO_Port, &GPIO_InitStruct);
		HAL_GPIO_WritePin(PH1_EN_GPIO_Port, PH1_EN_Pin, GPIO_PIN_SET);  /* restore HIGH */
		GPIO_InitStruct.Pin = PH2_EN_Pin;  HAL_GPIO_Init(PH2_EN_GPIO_Port, &GPIO_InitStruct);

		/* MUX_ENn: restore output, keep HIGH (ADG1634 disabled state from stim_off) */
		GPIO_InitStruct.Pin = MUX_ENn_Pin; HAL_GPIO_Init(MUX_ENn_GPIO_Port, &GPIO_InitStruct);
		HAL_GPIO_WritePin(MUX_ENn_GPIO_Port, MUX_ENn_Pin, GPIO_PIN_SET);

		/* Stimulation sink pins */
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Pin = SNK1_Pin;           HAL_GPIO_Init(SNK1_GPIO_Port,          &GPIO_InitStruct);
		GPIO_InitStruct.Pin = SNK2_Pin;           HAL_GPIO_Init(SNK2_GPIO_Port,          &GPIO_InitStruct);
		GPIO_InitStruct.Pin = SNK3_Pin;           HAL_GPIO_Init(SNK3_GPIO_Port,          &GPIO_InitStruct);
		GPIO_InitStruct.Pin = SNK4_Pin;           HAL_GPIO_Init(SNK4_GPIO_Port,          &GPIO_InitStruct);
		GPIO_InitStruct.Pin = SNK5_Pin;           HAL_GPIO_Init(SNK5_GPIO_Port,          &GPIO_InitStruct);
		GPIO_InitStruct.Pin = STIM_SEL_CH1n_Pin;  HAL_GPIO_Init(STIM_SEL_CH1n_GPIO_Port,  &GPIO_InitStruct);
		GPIO_InitStruct.Pin = STIM_SEL_CH2n_Pin;  HAL_GPIO_Init(STIM_SEL_CH2n_GPIO_Port,  &GPIO_InitStruct);
		GPIO_InitStruct.Pin = STIM_SEL_CH3n_Pin;  HAL_GPIO_Init(STIM_SEL_CH3n_GPIO_Port,  &GPIO_InitStruct);
		GPIO_InitStruct.Pin = STIM_SEL_CH4n_Pin;  HAL_GPIO_Init(STIM_SEL_CH4n_GPIO_Port,  &GPIO_InitStruct);
		GPIO_InitStruct.Pin = STIM_SEL_ENCLn_Pin; HAL_GPIO_Init(STIM_SEL_ENCLn_GPIO_Port, &GPIO_InitStruct);
		GPIO_InitStruct.Pin = STIMA_SELn_Pin;      HAL_GPIO_Init(STIMA_SELn_GPIO_Port,     &GPIO_InitStruct);
		GPIO_InitStruct.Pin = STIMB_SELn_Pin;      HAL_GPIO_Init(STIMB_SELn_GPIO_Port,     &GPIO_InitStruct);
		GPIO_InitStruct.Pin = IMP_EN_Pin;          HAL_GPIO_Init(IMP_EN_GPIO_Port,          &GPIO_InitStruct);
		GPIO_InitStruct.Pin = IMP_IN_P_SEL_Pin;    HAL_GPIO_Init(IMP_IN_P_SEL_GPIO_Port,    &GPIO_InitStruct);
		GPIO_InitStruct.Pin = IMP_IN_N_SEL0_Pin;   HAL_GPIO_Init(IMP_IN_N_SEL0_GPIO_Port,   &GPIO_InitStruct);
		GPIO_InitStruct.Pin = IMP_IN_N_SEL1_Pin;   HAL_GPIO_Init(IMP_IN_N_SEL1_GPIO_Port,   &GPIO_InitStruct);
		GPIO_InitStruct.Pin = IMP_IN_N_SEL2_Pin;   HAL_GPIO_Init(IMP_IN_N_SEL2_GPIO_Port,   &GPIO_InitStruct);
	}

	bsp_wdg_refresh();
	bsp_fram_init(&app_func_logs_write_cplt_cb);
	bsp_sp_init(&app_func_command_parser, &bsp_fram_write_cplt_cb);
	app_func_logs_event_write(EVENT_WAKEUP, NULL);
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

	app_func_sm_current_state_set(STATE_SLEEP); //Change to STATE_SHUTDOWN to test shutdown on sleep timer
}
