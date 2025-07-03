/**
 * @file app_mode_therapy_session.c
 * @brief This file provides management of the therapy session mode
 * @copyright Copyright (c) 2024
 */
#include "app_mode_therapy_session.h"
#include "app_config.h"

static bool therapy_session_status = false;

extern bool schd_therapy_enable;

/**
 * @brief Start therapy session and check for short circuit events
 * 
 * @return true There is no short circuit event and then therapy session start
 * @return false A short circuit event exists, so therapy session is stopped
 */
bool app_mode_therapy_start(void) {
	therapy_session_status = false;
	if (!app_func_logs_event_search(EVENT_SHORT_CIRCUIT)) {
		_Float64 pulse_amplitude_mA = 0.0;
		app_func_para_data_get((const uint8_t*)SPID_PULSE_AMPLITUDE, (uint8_t*)&pulse_amplitude_mA, (uint8_t)sizeof(pulse_amplitude_mA));

		_Float64 max_safe_amplitude_mA = 0.0;
		app_func_para_data_get((const uint8_t*)SPID_MAX_SAFE_AMPLITUDE, (uint8_t*)&max_safe_amplitude_mA, (uint8_t)sizeof(max_safe_amplitude_mA));

		if (pulse_amplitude_mA > max_safe_amplitude_mA) {
			pulse_amplitude_mA = max_safe_amplitude_mA;
			app_func_para_data_set((const uint8_t*)SPID_PULSE_AMPLITUDE, (uint8_t*)&pulse_amplitude_mA);
			app_func_logs_event_write(EVENT_LOWER_STIM_AMP, NULL);
		}
		uint16_t dacVoltage_mv = (uint16_t)(pulse_amplitude_mA * BSP_PULSE_AMP_RATIO);

		_Float64 pulse_width_us = 0.0;
		app_func_para_data_get((const uint8_t*)SPID_PULSE_WIDTH, (uint8_t*)&pulse_width_us, (uint8_t)sizeof(pulse_width_us));

		_Float64 pulse_frequency_hz = 0.0;
		app_func_para_data_get((const uint8_t*)SPID_PULSE_FREQUENCY, (uint8_t*)&pulse_frequency_hz, (uint8_t)sizeof(pulse_frequency_hz));
		_Float64 pulse_period_us = 1.0 / pulse_frequency_hz * 1000000.0;

		_Float64 train_on_duration_s = 0.0;
		app_func_para_data_get((const uint8_t*)SPID_TRAIN_ON_DURATION, (uint8_t*)&train_on_duration_s, (uint8_t)sizeof(train_on_duration_s));

		_Float64 train_off_duration_s = 0.0;
		app_func_para_data_get((const uint8_t*)SPID_TRAIN_OFF_DURATION, (uint8_t*)&train_off_duration_s, (uint8_t)sizeof(train_off_duration_s));

		Stimulus_Waveform_t parameters = {
				.pulseWidth_us 			= (uint32_t)pulse_width_us,
				.pulsePeriod_us 		= (uint32_t)pulse_period_us,
				.trainOnDuration_ms 	= (uint32_t)(train_on_duration_s * 1000.0),
				.trainOffDuration_ms	= (uint32_t)(train_off_duration_s * 1000.0),
		};

		_Float64 ramp_up_duration_s = 0.0;
		app_func_para_data_get((const uint8_t*)SPID_RAMP_UP_DURATION, (uint8_t*)&ramp_up_duration_s, (uint8_t)sizeof(ramp_up_duration_s));
		uint32_t rampUpDuration_ms = (uint32_t)(ramp_up_duration_s * 1000.0);

		_Float64 ramp_down_duration_s = 0.0;
		app_func_para_data_get((const uint8_t*)SPID_RAMP_DOWN_DURATION, (uint8_t*)&ramp_down_duration_s, (uint8_t)sizeof(ramp_down_duration_s));
		uint32_t rampDownDuration_ms = (uint32_t)(ramp_down_duration_s * 1000.0);

		_Float64 cathode_electrode_number = 0.0;
		app_func_para_data_get((const uint8_t*)SPID_CATHODE_ELECTRODE_NUMBER, (uint8_t*)&cathode_electrode_number, (uint8_t)sizeof(cathode_electrode_number));
		_Float64 anode_electrode_number = 0.0;
		app_func_para_data_get((const uint8_t*)SPID_ANODE_ELECTRODE_NUMBER, (uint8_t*)&anode_electrode_number, (uint8_t)sizeof(anode_electrode_number));

		uint8_t src_select = (uint8_t)cathode_electrode_number;
		uint8_t sink_select = (uint8_t)anode_electrode_number;
		Current_Sources_t configuration;
		memset(&configuration, 0, sizeof(configuration));
		Src_Sink_t* p_config = (Src_Sink_t*)&configuration;
		p_config[src_select - 1U].src = true;
		p_config[sink_select - 1U].sink = true;

		app_func_stim_hv_supply_set(true, true);
		HAL_Delay(1);
		HAL_ERROR_CHECK(app_func_stim_hv_sup_volt_set((uint16_t)HV_SUPPLY_MV));
		HAL_Delay(10);
		app_func_stim_vdds_sup_enable(true);
		app_func_stim_stimulus_enable(true);
		app_func_stim_stimulus_mux_enable();
		HAL_Delay(10);

		HAL_ERROR_CHECK(app_func_stim_dac_init(DACTYPE_STIM));
		HAL_ERROR_CHECK(app_func_stim_dac_ramp_volt_set(rampUpDuration_ms, rampDownDuration_ms, dacVoltage_mv));
		HAL_Delay(10);

		app_func_stim_circuit_para_set(parameters);
		app_func_stim_curr_src_set(configuration);
		app_func_stim_stimulus_start(true, true, false);
		therapy_session_status = true;
	}
	return therapy_session_status;
}

/**
 * @brief Stop therapy session
 * 
 */
void app_mode_therapy_stop(void) {
	app_func_stim_off();
	bsp_mux_enable(false);
	therapy_session_status = false;
}

/**
 * @brief Confirm therapy session status
 * 
 * @return true Therapy session has been activated
 * @return false The therapy session is not active
 */
bool app_mode_therapy_confirm(void) {
	return therapy_session_status;
}

/**
 * @brief Handler for therapy session mode
 * 
 */
void app_mode_therapy_handler(void) {
	if (app_mode_therapy_start() == true) {
		_Float64 rtc_interrupt_period_f = 0.0;
		app_func_para_data_get((const uint8_t*)HPID_RTC_INTERRUPT_PERIOD, (uint8_t*)&rtc_interrupt_period_f, (uint8_t)sizeof(rtc_interrupt_period_f));
		uint32_t rtc_interrupt_period = (uint32_t)rtc_interrupt_period_f;
		if (rtc_interrupt_period > 0) {
			HAL_ERROR_CHECK(HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, rtc_interrupt_period - 1, RTC_WAKEUPCLOCK_CK_SPRE_16BITS, 0));
		}

		app_func_sm_schd_therapy_enable(true);
		while(app_func_sm_current_state_get() == STATE_ACT_MODE_THERAPY_SESSION) {
			bsp_wdg_refresh();
		}
		app_func_sm_schd_therapy_enable(false);
		//stimulation stop
		app_mode_therapy_stop();
	}
	else {
		app_func_sm_current_state_set(STATE_ACT);
	}
}
