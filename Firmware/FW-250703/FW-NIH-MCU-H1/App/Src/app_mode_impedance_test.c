/**
 * @file app_mode_impedance_test.c
 * @brief This file provides management of the impedance test mode
 * @copyright Copyright (c) 2024
 */
#include "app_mode_impedance_test.h"
#include "app_config.h"

#define	IMP_MEAS_STIM_AMP_MA	1.0
#define	IMP_MEAS_STIM_PW_US		500
#define	IMP_MEAS_STIM_PP_US		10000
#define	IMP_MEAS_SAMPLE_FQ_HZ	50000

static uint16_t voltageBuffer[ADC_MAX_SAMPLE_POINTS / 2];

/**
 * @brief Measure, obtain and record impedance
 * 
 * @return _Float64 The impedance of the load, unit: ohm
 */
_Float64 app_mode_impedance_test_get(void) {
	_Float64 cathode_electrode_number = 0.0;
	app_func_para_data_get((const uint8_t*)SPID_CATHODE_ELECTRODE_NUMBER, (uint8_t*)&cathode_electrode_number, (uint8_t)sizeof(cathode_electrode_number));
	_Float64 anode_electrode_number = 0.0;
	app_func_para_data_get((const uint8_t*)SPID_ANODE_ELECTRODE_NUMBER, (uint8_t*)&anode_electrode_number, (uint8_t)sizeof(anode_electrode_number));

	Stimulus_Waveform_t parameters = {
			.pulseWidth_us 			= IMP_MEAS_STIM_PW_US,
			.pulsePeriod_us 		= IMP_MEAS_STIM_PP_US,
			.trainOnDuration_ms 	= 1000,
			.trainOffDuration_ms	= 0,
	};

	uint8_t src_select = (uint8_t)cathode_electrode_number;
	uint8_t sink_select = (uint8_t)anode_electrode_number;

	Current_Sources_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	Src_Sink_t* p_config = (Src_Sink_t*)&configuration;
	p_config[src_select - 1U].src = true;
	p_config[sink_select - 1U].sink = true;

	_Float64 pulse_amplitude_mA = IMP_MEAS_STIM_AMP_MA;
	uint16_t dacVoltage_mv = (uint16_t)(pulse_amplitude_mA * BSP_PULSE_AMP_RATIO);

	uint8_t chA = (uint8_t)(cathode_electrode_number - 1U);
	uint8_t chB = (uint8_t)(anode_electrode_number - 1U);

	app_func_stim_hv_supply_set(true, true);
	HAL_Delay(1);
	HAL_ERROR_CHECK(app_func_stim_hv_sup_volt_set((uint16_t)HV_SUPPLY_MV));
	HAL_Delay(10);
	app_func_stim_vdds_sup_enable(true);
	app_func_stim_stimulus_enable(true);
	app_func_stim_stimulus_mux_enable();
	HAL_Delay(10);
	app_func_meas_imp_enable(true);
	app_func_meas_imp_sel_set(	((chA >> 0) & 0x1), ((chA >> 1) & 0x1), ((chA >> 2) & 0x1), ((chA >> 3) & 0x1),
								((chB >> 0) & 0x1), ((chB >> 1) & 0x1), ((chB >> 2) & 0x1), ((chB >> 3) & 0x1)	);
	HAL_Delay(10);

	HAL_ERROR_CHECK(app_func_stim_dac_init(DACTYPE_STIM));
	HAL_ERROR_CHECK(app_func_stim_dac_volt_set(dacVoltage_mv, dacVoltage_mv));
	HAL_Delay(10);

	uint16_t samplingPoints = sizeof(voltageBuffer) / sizeof(uint16_t);
	uint16_t samplingFrequency_hz = IMP_MEAS_SAMPLE_FQ_HZ;

	bsp_wdg_refresh();
	app_func_stim_circuit_para_set(parameters);
	app_func_stim_curr_src_set(configuration);
	app_func_stim_stimulus_start(true, true, false);
	app_func_meas_imp_volt_meas(voltageBuffer, samplingPoints, samplingFrequency_hz);

	app_func_stim_off();
	bsp_mux_enable(false);
	app_func_meas_imp_enable(false);

	uint16_t maxVoltage = 0;
	for(uint16_t i=0;i<samplingPoints;i++) {
		maxVoltage = (maxVoltage > voltageBuffer[i])?maxVoltage:voltageBuffer[i];
	}


	_Float64 impedance = (_Float64)maxVoltage / pulse_amplitude_mA;
	app_func_logs_imped_write((uint32_t)impedance);

	return impedance;
}

/**
 * @brief Handler for impedance test mode
 *
 */
void app_mode_impedance_test_handler(void) {
	_Float64 impedance = app_mode_impedance_test_get();

	_Float64 min_safe_impedance_ohm = 0.0;
	app_func_para_data_get((const uint8_t*)SPID_MIN_SAFE_IMPEDANCE, (uint8_t*)&min_safe_impedance_ohm, (uint8_t)sizeof(min_safe_impedance_ohm));
	if (impedance < min_safe_impedance_ohm) {
		app_func_logs_event_write((const char*)EVENT_SHORT_CIRCUIT, NULL);
	}

	_Float64 present_max_amplitude_mA = (_Float64)HV_SUPPLY_MV / impedance;
	present_max_amplitude_mA = app_func_para_val_quant_clip((const uint8_t*)SPID_MAX_SAFE_AMPLITUDE, present_max_amplitude_mA);

	_Float64 max_safe_amplitude_mA = 0.0;
	app_func_para_data_get((const uint8_t*)SPID_MAX_SAFE_AMPLITUDE, (uint8_t*)&max_safe_amplitude_mA, (uint8_t)sizeof(max_safe_amplitude_mA));
	if (present_max_amplitude_mA < max_safe_amplitude_mA) {
		app_func_logs_event_write((const char*)EVENT_HIGH_IMPED, NULL);
	}
	else {
		app_func_logs_event_write((const char*)EVENT_NORMAL_IMPED, NULL);
	}

	max_safe_amplitude_mA = present_max_amplitude_mA;
	app_func_para_data_set((const uint8_t*)SPID_MAX_SAFE_AMPLITUDE, (uint8_t*)&max_safe_amplitude_mA);

	app_func_sm_current_state_set(STATE_ACT);
	app_func_sm_impedance_timer_enable();
}
