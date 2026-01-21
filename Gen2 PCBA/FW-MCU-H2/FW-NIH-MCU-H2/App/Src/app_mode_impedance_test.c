/**
 * @file app_mode_impedance_test.c
 * @brief This file provides management of the impedance test mode
 * @copyright Copyright (c) 2024
 */
#include "app_mode_impedance_test.h"
#include "app_config.h"

#define IMP_MEAS_HV_SUPPLY_MV	4200U
#define	IMP_MEAS_SAMPLE_FQ_HZ	50000U

static Stimulus_Waveform_t parameters = {
		.pulseWidth_us 			= 500,
		.pulsePeriod_us 		= 10000,
		.trainOnDuration_ms 	= 200,
		.trainOffDuration_ms	= 0,
};

static uint16_t impVoltageBufferA[ADC_MAX_SAMPLE_POINTS];
static uint16_t impVoltageBufferB[ADC_MAX_SAMPLE_POINTS];

static _Float64 impVoltage;

#ifdef SWV_TRACE
static uint16_t swvTrace = 0;
#endif

/**
 * @brief Measure, obtain and record impedance
 * 
 * @return _Float64 The impedance of the load, unit: ohm
 */
_Float64 app_mode_impedance_test_get(void) {
	_Float64 sns_cathode_electrode_number = 0.0;
	_Float64 sns_anode_electrode_number = 0.0;
	_Float64 max_safe_amplitude_mA = 0.0;

	app_func_para_data_get((const uint8_t*)SPID_SNS_CATHODE_ELECTRODE_NUMBER, (uint8_t*)&sns_cathode_electrode_number, (uint8_t)sizeof(_Float64));
	app_func_para_data_get((const uint8_t*)SPID_SNS_ANODE_ELECTRODE_NUMBER, (uint8_t*)&sns_anode_electrode_number, (uint8_t)sizeof(_Float64));
	app_func_para_data_get((const uint8_t*)SPID_MAX_SAFE_AMPLITUDE, (uint8_t*)&max_safe_amplitude_mA, (uint8_t)sizeof(_Float64));

	uint16_t dacVoltage_mv = (uint16_t)app_func_stim_iout_to_dac(max_safe_amplitude_mA);

	uint8_t sns_snkP_select = (uint8_t)sns_anode_electrode_number;
	uint8_t sns_snkN_select = (uint8_t)sns_cathode_electrode_number;
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

	bool imp_n_sel0 = ((sns_snkN_select - 1) >> 0) & 0x01;
	bool imp_n_sel1 = ((sns_snkN_select - 1) >> 1) & 0x01;
	bool imp_n_sel2 = ((sns_snkN_select - 1) >> 2) & 0x01;
	bool imp_p_sel  = IMPIN_P_STIMA;

	switch(sns_snkP_select) {
	case 1:
		configuration.snk1 = true;
		sel.stimA = STIMA_SEL_STIM1;
		sel.stimB = STIMB_SEL_STIM2;
		sel.sel_ch.ch1 = STIM_SEL_CH1_STIMA;
		imp_p_sel  = IMPIN_P_STIMA;
		break;
	case 2:
		configuration.snk2 = true;
		sel.stimA = STIMA_SEL_STIM1;
		sel.stimB = STIMB_SEL_STIM2;
		sel.sel_ch.ch2 = STIM_SEL_CH2_STIMA;
		imp_p_sel  = IMPIN_P_STIMA;
		break;
	case 3:
		configuration.snk3 = true;
		sel.stimA = STIMA_SEL_STIM2;
		sel.stimB = STIMB_SEL_STIM1;
		sel.sel_ch.ch3 = STIM_SEL_CH3_STIMB;
		imp_p_sel  = IMPIN_P_STIMB;
		break;
	case 4:
		configuration.snk4 = true;
		sel.stimA = STIMA_SEL_STIM2;
		sel.stimB = STIMB_SEL_STIM1;
		sel.sel_ch.ch4 = STIM_SEL_CH4_STIMB;
		imp_p_sel  = IMPIN_P_STIMB;
		break;
	case 5:
		configuration.snk5 = true;
		sel.stimA = STIMA_SEL_STIM1;
		sel.stimB = STIMB_SEL_STIM2;
		sel.sel_ch.encl = STIM_SEL_ENCL_STIMA;
		imp_p_sel  = IMPIN_P_STIMA;
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

	uint16_t samplingFrequency_hz = IMP_MEAS_SAMPLE_FQ_HZ;
	uint16_t periodPoints = app_func_meas_imp_sampPoints_get(samplingFrequency_hz, parameters.pulsePeriod_us);
	uint16_t pulsePoints = app_func_meas_imp_sampPoints_get(samplingFrequency_hz, parameters.pulseWidth_us);
	memset(impVoltageBufferA, 0, sizeof(impVoltageBufferA));
	memset(impVoltageBufferB, 0, sizeof(impVoltageBufferB));

	bsp_wdg_refresh();
	app_func_stim_off();
	HAL_Delay(100);

	app_func_stim_curr_src_set(configuration);
	app_func_stim_circuit_para1_set(parameters);

	app_func_stim_hv_supply_set(true, true);
	HAL_ERROR_CHECK(app_func_stim_hv_sup_volt_set((uint16_t)IMP_MEAS_HV_SUPPLY_MV));
	app_func_stim_vdds_sup_enable(true);
	HAL_Delay(100);

	HAL_ERROR_CHECK(app_func_stim_dac_init());
	HAL_ERROR_CHECK(app_func_stim_dac_volt_set(dacVoltage_mv, 0));

	app_func_stim_sel_set(sel);
	app_func_stim_stimulus_enable(true);

	app_func_meas_imp_sel_set(imp_n_sel0, imp_n_sel1, imp_n_sel2, imp_p_sel);
	app_func_meas_imp_enable(true);

	HAL_Delay(100);
	bsp_wdg_refresh();

	app_func_stim_mux_enable(true);
	app_func_stim_stim1_start(true);
	HAL_Delay(parameters.trainOnDuration_ms);
	app_func_stim_sync();
	app_func_meas_imp_volt_meas(IMPIN_CH_P, impVoltageBufferA, periodPoints, samplingFrequency_hz);
	app_func_stim_sync();
	app_func_meas_imp_volt_meas(IMPIN_CH_N, impVoltageBufferB, periodPoints, samplingFrequency_hz);
	app_func_stim_off();
	app_func_meas_imp_enable(false);

#ifdef SWV_TRACE
	swvTrace = 0;
	for(uint16_t i=0;i<periodPoints;i++) {
		swvTrace = impVoltageBufferA[i];
		HAL_Delay(1);
		bsp_wdg_refresh();
	}
	swvTrace = 0;
	HAL_Delay(100);
	bsp_wdg_refresh();
	for(uint16_t i=0;i<periodPoints;i++) {
		swvTrace = impVoltageBufferB[i];
		HAL_Delay(1);
		bsp_wdg_refresh();
	}
	swvTrace = 0;
#endif

	_Float64 impVoltageA = app_func_meas_imp_volt_calc(impVoltageBufferA, periodPoints, pulsePoints);
	_Float64 impVoltageB = app_func_meas_imp_volt_calc(impVoltageBufferB, periodPoints, pulsePoints);
	impVoltage = impVoltageA + impVoltageB;

	_Float64 impedance = app_func_meas_imp_calc(dacVoltage_mv, impVoltage);
	app_func_logs_imped_write((uint32_t)impedance);

	return impedance;
}

/**
 * @brief Handler for impedance test mode
 *
 */
void app_mode_impedance_test_handler(void) {
	_Float64 max_safe_amplitude_mA = 0.0;
	_Float64 min_safe_impedance_ohm = 0.0;

	app_func_para_data_get((const uint8_t*)SPID_MAX_SAFE_AMPLITUDE, (uint8_t*)&max_safe_amplitude_mA, (uint8_t)sizeof(_Float64));
	app_func_para_data_get((const uint8_t*)SPID_MIN_SAFE_IMPEDANCE, (uint8_t*)&min_safe_impedance_ohm, (uint8_t)sizeof(_Float64));

	_Float64 impedance = app_mode_impedance_test_get();
	if (impedance < min_safe_impedance_ohm) {
		app_func_logs_event_write((const char*)EVENT_SHORT_CIRCUIT, NULL);
	}

	_Float64 present_max_amplitude_mA = impVoltage / impedance;
	present_max_amplitude_mA = app_func_para_val_quant_clip((const uint8_t*)SPID_MAX_SAFE_AMPLITUDE, present_max_amplitude_mA);

	if (present_max_amplitude_mA != max_safe_amplitude_mA) {
		if (present_max_amplitude_mA < max_safe_amplitude_mA) {
			app_func_logs_event_write((const char*)EVENT_HIGH_IMPED, NULL);
		}
		else if (present_max_amplitude_mA > max_safe_amplitude_mA) {
			app_func_logs_event_write((const char*)EVENT_NORMAL_IMPED, NULL);
		}
		max_safe_amplitude_mA = present_max_amplitude_mA;
		app_func_para_data_set((const uint8_t*)SPID_MAX_SAFE_AMPLITUDE, (uint8_t*)&max_safe_amplitude_mA);
	}

	app_func_sm_current_state_set(STATE_ACT);
	app_func_sm_impedance_timer_enable();
}
