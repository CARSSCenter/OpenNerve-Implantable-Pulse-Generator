/**
 * @file app_mode_therapy_session.c
 * @brief This file provides management of the therapy session mode
 * @copyright Copyright (c) 2024
 */
#include "app_mode_therapy_session.h"
#include "app_config.h"

static bool therapy_session_status = false;

bool vnsb_en = false;

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
		_Float64 pulse_width_us = 0.0;
		_Float64 pulse_frequency_hz = 0.0;
		_Float64 ramp_up_duration_s = 0.0;
		_Float64 ramp_down_duration_s = 0.0;
		_Float64 train_on_duration_s = 0.0;
		_Float64 train_off_duration_s = 0.0;
		_Float64 sns_cathode_electrode_number = 0.0;
		_Float64 sns_anode_electrode_number = 0.0;
		_Float64 max_safe_amplitude_mA = 0.0;
		_Float64 sine_amplitude_mA = 0.0;
		_Float64 max_safe_sine_amplitude_mA = 0.0;
		_Float64 sine_frequency_hz = 0.0;
		_Float64 vnsb_on_duration_s = 0.0;
		_Float64 vnsb_off_duration_s = 0.0;

		app_func_para_data_get((const uint8_t*)SPID_PULSE_AMPLITUDE, (uint8_t*)&pulse_amplitude_mA, (uint8_t)sizeof(_Float64));
		app_func_para_data_get((const uint8_t*)SPID_PULSE_WIDTH, (uint8_t*)&pulse_width_us, (uint8_t)sizeof(_Float64));
		app_func_para_data_get((const uint8_t*)SPID_PULSE_FREQUENCY, (uint8_t*)&pulse_frequency_hz, (uint8_t)sizeof(_Float64));
		app_func_para_data_get((const uint8_t*)SPID_RAMP_UP_DURATION, (uint8_t*)&ramp_up_duration_s, (uint8_t)sizeof(_Float64));
		app_func_para_data_get((const uint8_t*)SPID_RAMP_DOWN_DURATION, (uint8_t*)&ramp_down_duration_s, (uint8_t)sizeof(_Float64));
		app_func_para_data_get((const uint8_t*)SPID_TRAIN_ON_DURATION, (uint8_t*)&train_on_duration_s, (uint8_t)sizeof(_Float64));
		app_func_para_data_get((const uint8_t*)SPID_TRAIN_OFF_DURATION, (uint8_t*)&train_off_duration_s, (uint8_t)sizeof(_Float64));
		app_func_para_data_get((const uint8_t*)SPID_SNS_CATHODE_ELECTRODE_NUMBER, (uint8_t*)&sns_cathode_electrode_number, (uint8_t)sizeof(_Float64));
		app_func_para_data_get((const uint8_t*)SPID_SNS_ANODE_ELECTRODE_NUMBER, (uint8_t*)&sns_anode_electrode_number, (uint8_t)sizeof(_Float64));
		app_func_para_data_get((const uint8_t*)SPID_MAX_SAFE_AMPLITUDE, (uint8_t*)&max_safe_amplitude_mA, (uint8_t)sizeof(_Float64));
		app_func_para_data_get((const uint8_t*)SPID_SINE_AMPLITUDE, (uint8_t*)&sine_amplitude_mA, (uint8_t)sizeof(_Float64));
		app_func_para_data_get((const uint8_t*)SPID_MAX_SAFE_SINE_AMPLITUDE, (uint8_t*)&max_safe_sine_amplitude_mA, (uint8_t)sizeof(_Float64));
		app_func_para_data_get((const uint8_t*)SPID_SINE_FREQUENCY, (uint8_t*)&sine_frequency_hz, (uint8_t)sizeof(_Float64));
		app_func_para_data_get((const uint8_t*)SPID_VNSB_ON_DURATION, (uint8_t*)&vnsb_on_duration_s, (uint8_t)sizeof(_Float64));
		app_func_para_data_get((const uint8_t*)SPID_VNSB_OFF_DURATION, (uint8_t*)&vnsb_off_duration_s, (uint8_t)sizeof(_Float64));

		if (pulse_amplitude_mA > max_safe_amplitude_mA) {
			pulse_amplitude_mA = max_safe_amplitude_mA;
			app_func_para_data_set((const uint8_t*)SPID_PULSE_AMPLITUDE, (uint8_t*)&pulse_amplitude_mA);
			app_func_logs_event_write(EVENT_LOWER_STIM_AMP, NULL);
		}
		uint16_t pulseDacVoltage_mv = (uint16_t)app_func_stim_iout_to_dac(pulse_amplitude_mA);

		_Float64 pulse_period_us = 1.0 / pulse_frequency_hz * 1000000.0;
		_Float64 max_pulse_width_us = pulse_period_us / 2.0;
		if (pulse_width_us > max_pulse_width_us) {
			pulse_width_us = max_pulse_width_us;
			app_func_para_data_set((const uint8_t*)SPID_PULSE_WIDTH, (uint8_t*)&pulse_width_us);
		}

		Stimulus_Waveform_t parameters = {
				.pulseWidth_us 			= (uint32_t)pulse_width_us,
				.pulsePeriod_us 		= (uint32_t)pulse_period_us,
				.trainOnDuration_ms 	= (uint32_t)(train_on_duration_s * 1000.0),
				.trainOffDuration_ms	= (uint32_t)(train_off_duration_s * 1000.0),
		};

		uint32_t rampUpDuration_ms = (uint32_t)(ramp_up_duration_s * 1000.0);
		uint32_t rampDownDuration_ms = (uint32_t)(ramp_down_duration_s * 1000.0);

		if (sine_amplitude_mA > max_safe_sine_amplitude_mA) {
			sine_amplitude_mA = max_safe_sine_amplitude_mA;
			app_func_para_data_set((const uint8_t*)SPID_SINE_AMPLITUDE, (uint8_t*)&sine_amplitude_mA);
		}
		uint16_t sineDacVoltage_mv = (uint16_t)app_func_stim_iout_to_dac(sine_amplitude_mA);

		_Float64 sine_period_us = 1000000.0 / sine_frequency_hz;
		_Float64 sine_phase_shift_us = sine_period_us * 0.15 + pulse_period_us;
		NerveBlock_Waveform_t sine_para = {
				.sinePeriod_us			= (uint32_t)sine_period_us,
				.sinePhaseShift_us		= (uint32_t)sine_phase_shift_us,
				.amplitude_mV 			= sineDacVoltage_mv,
				.trainOnDuration_ms 	= (uint32_t)(vnsb_on_duration_s * 1000),
				.trainOffDuration_ms 	= (uint32_t)(vnsb_off_duration_s * 1000),
		};

		Current_Sources_t configuration = {
				.src1 = true,
				.src2 = false,
				.snk1 = false,
				.snk2 = false,
				.snk3 = false,
				.snk4 = false,
				.snk5 = false,
		};

		Stim_Sel_t sel = {
				.stimA 			= STIMA_SEL_STIM1,
				.stimB 			= STIMB_SEL_STIM1,
				.sel_ch.encl 	= STIM_SEL_ENCL_SINK_ENCL,
				.sel_ch.ch1 	= STIM_SEL_CH1_SINK_CH1,
				.sel_ch.ch2 	= STIM_SEL_CH2_SINK_CH2,
				.sel_ch.ch3 	= STIM_SEL_CH3_SINK_CH3,
				.sel_ch.ch4 	= STIM_SEL_CH4_SINK_CH4,
		};

		if (vnsb_en) {
			configuration.src2 = true;
			configuration.snk1 = true;
			configuration.snk2 = true;
			configuration.snk3 = true;
			configuration.snk4 = true;
			sel.stimA = STIMA_SEL_STIM1;
			sel.stimB = STIMB_SEL_STIM2;
			sel.sel_ch.ch1 = STIM_SEL_CH1_STIMA;
			sel.sel_ch.ch3 = STIM_SEL_CH3_STIMB;
		}
		else {
			uint8_t sns_snkP_select = (uint8_t)sns_anode_electrode_number;
			uint8_t sns_snkN_select = (uint8_t)sns_cathode_electrode_number;

			switch(sns_snkP_select) {
			case 1:
				configuration.snk1 = true;
				sel.stimA = STIMA_SEL_STIM1;
				sel.stimB = STIMB_SEL_STIM2;
				sel.sel_ch.ch1 = STIM_SEL_CH1_STIMA;
				break;
			case 2:
				configuration.snk2 = true;
				sel.stimA = STIMA_SEL_STIM1;
				sel.stimB = STIMB_SEL_STIM2;
				sel.sel_ch.ch2 = STIM_SEL_CH2_STIMA;
				break;
			case 3:
				configuration.snk3 = true;
				sel.stimA = STIMA_SEL_STIM2;
				sel.stimB = STIMB_SEL_STIM1;
				sel.sel_ch.ch3 = STIM_SEL_CH3_STIMB;
				break;
			case 4:
				configuration.snk4 = true;
				sel.stimA = STIMA_SEL_STIM2;
				sel.stimB = STIMB_SEL_STIM1;
				sel.sel_ch.ch4 = STIM_SEL_CH4_STIMB;
				break;
			case 5:
				configuration.snk5 = true;
				sel.stimA = STIMA_SEL_STIM1;
				sel.stimB = STIMB_SEL_STIM2;
				sel.sel_ch.encl = STIM_SEL_ENCL_STIMA;
				break;
			}

			switch(sns_snkN_select) {
			case 1:
				configuration.snk1 = true;
				break;
			case 2:
				configuration.snk2 = true;
				break;
			case 3:
				configuration.snk3 = true;
				break;
			case 4:
				configuration.snk4 = true;
				break;
			case 5:
				configuration.snk5 = true;
				break;
			}
		}

		bsp_wdg_refresh();
		app_func_stim_off();
		HAL_Delay(100);

		app_func_stim_curr_src_set(configuration);
		app_func_stim_circuit_para1_set(parameters);
		app_func_stim_sine_para_set(sine_para);

		app_func_stim_hv_supply_set(true, true);
		HAL_ERROR_CHECK(app_func_stim_hv_sup_volt_set((uint16_t)HV_SUPPLY_MV));
		app_func_stim_vdds_sup_enable(true);
		HAL_Delay(100);

		HAL_ERROR_CHECK(app_func_stim_dac_init());
		HAL_ERROR_CHECK(app_func_stim_dac_volt_set(0U, 0U));
		app_func_stim_dac1_ramp_set(rampUpDuration_ms, rampDownDuration_ms, pulseDacVoltage_mv);

		app_func_stim_sel_set(sel);
		app_func_stim_stimulus_enable(true);

		HAL_Delay(100);
		bsp_wdg_refresh();

		app_func_stim_mux_enable(true);
		app_func_stim_stim1_start(false);
		if (vnsb_en) {
			app_func_stim_sine_start();
			app_func_stim_sync();
		}
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
