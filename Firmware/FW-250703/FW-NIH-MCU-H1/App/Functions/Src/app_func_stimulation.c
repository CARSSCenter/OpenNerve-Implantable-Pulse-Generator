/**
 * @file app_func_stimulation.c
 * @brief This file provides stimulation management
 * @copyright Copyright (c) 2024
 */
#include "app_func_stimulation.h"
#include "app_config.h"

#include "stm32u5xx_ll_tim.h"

PulseWave_t pulseWave = {0};
SineWave_t sineWave = {0};
VNSbWave_t vnsbWave = {0};

Current_Sources_t src_sink_latest = {0};			/*!< Latest pin state */
uint32_t ramp_amplitude_latest = 0;
bool pulse_state = false;

static DAC8050x_format_t dac_write = {0};
static DAC8050x_format_t dac_read = {0};
static bool	ph_out_force = false;

/**
 * @brief Generates a sine wave value for a specific point in a waveform.
 *
 * @param total_points The total number of points in the sine wave (i.e., the number of samples per period).
 * @param point The specific point (index) for which to generate the sine wave value. It should be in the range [0, total_points - 1].
 * @param amplitude The amplitude of the sine wave.
 *
 * @return The sine wave value at the given point, scaled by the amplitude.
 */
static _Float64 generate_sine_wave(uint32_t total_points, uint32_t point, _Float64 amplitude) {
    if (point > total_points) {
        return 0.0;
    }
    _Float64 angle = (2.0 * M_PI * point) / total_points;
    return amplitude * sin(angle);
}

/**
 * @brief Configures the source and sink pins for multiple current sources.
 *
 * @param current_sources The current sources configuration structure containing the
 *                        `src` (source) and `sink` states for each source (s1 through s17).
 * @param force           A boolean flag indicating whether to force the pin setting
 *                        regardless of the `src` and `sink` states.
 *
 */
static void src_sink_pin_set(Current_Sources_t current_sources, bool force) {
	if (current_sources.s1.src | current_sources.s1.sink | force) {
		HAL_GPIO_WritePin(SRC1_GPIO_Port, 	SRC1_Pin, 	(current_sources.s1.src)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_GPIO_WritePin(SNK1_GPIO_Port, 	SNK1_Pin, 	(current_sources.s1.sink)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}

	if (current_sources.s2.src | current_sources.s2.sink | force) {
		HAL_GPIO_WritePin(SRC2_GPIO_Port, 	SRC2_Pin, 	(current_sources.s2.src)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_GPIO_WritePin(SNK2_GPIO_Port, 	SNK2_Pin, 	(current_sources.s2.sink)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}

	if (current_sources.s3.src | current_sources.s3.sink | force) {
		HAL_GPIO_WritePin(SRC3_GPIO_Port, 	SRC3_Pin, 	(current_sources.s3.src)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_GPIO_WritePin(SNK3_GPIO_Port, 	SNK3_Pin, 	(current_sources.s3.sink)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}

	if (current_sources.s4.src | current_sources.s4.sink | force) {
		HAL_GPIO_WritePin(SRC4_GPIO_Port, 	SRC4_Pin, 	(current_sources.s4.src)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_GPIO_WritePin(SNK4_GPIO_Port, 	SNK4_Pin, 	(current_sources.s4.sink)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}

	if (current_sources.s5.src | current_sources.s5.sink | force) {
		HAL_GPIO_WritePin(SRC5_GPIO_Port, 	SRC5_Pin, 	(current_sources.s5.src)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_GPIO_WritePin(SNK5_GPIO_Port, 	SNK5_Pin, 	(current_sources.s5.sink)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}

	if (current_sources.s6.src | current_sources.s6.sink | force) {
		HAL_GPIO_WritePin(SRC6_GPIO_Port, 	SRC6_Pin, 	(current_sources.s6.src)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_GPIO_WritePin(SNK6_GPIO_Port, 	SNK6_Pin, 	(current_sources.s6.sink)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}

	if (current_sources.s7.src | current_sources.s7.sink | force) {
		HAL_GPIO_WritePin(SRC7_GPIO_Port, 	SRC7_Pin, 	(current_sources.s7.src)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_GPIO_WritePin(SNK7_GPIO_Port, 	SNK7_Pin, 	(current_sources.s7.sink)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}

	if (current_sources.s8.src | current_sources.s8.sink | force) {
		HAL_GPIO_WritePin(SRC8_GPIO_Port, 	SRC8_Pin, 	(current_sources.s8.src)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_GPIO_WritePin(SNK8_GPIO_Port, 	SNK8_Pin, 	(current_sources.s8.sink)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}

	if (current_sources.s17.src | current_sources.s17.sink | force) {
		HAL_GPIO_WritePin(SRC17_GPIO_Port, 	SRC17_Pin, 	(current_sources.s17.src)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_GPIO_WritePin(SNK17_GPIO_Port, 	SNK17_Pin, 	(current_sources.s17.sink)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}
	src_sink_latest = current_sources;
}

/**
 * @brief Set the state of GPIOs of HV supply
 * 
 * @param turnon Set the state of pin "HVSW_EN"
 * @param enable Set the state of pins "VPPSW_EN" and "HV_EN"
 */
void app_func_stim_hv_supply_set(bool turnon, bool enable) {
	HAL_GPIO_WritePin(HVSW_EN_GPIO_Port, HVSW_EN_Pin, (turnon)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_Delay(10);
	HAL_GPIO_WritePin(HV_EN_GPIO_Port, HV_EN_Pin, (enable)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_Delay(10);
	HAL_GPIO_WritePin(VPPSW_EN_GPIO_Port, VPPSW_EN_Pin, (enable)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
}

/**
 * @brief Set the voltage of HV supply
 * 
 * @param voltage_mv The voltage of HV supply, unit: mV
 * @return uint8_t HAL status
 */
uint8_t app_func_stim_hv_sup_volt_set(uint16_t voltage_mv) {
	HAL_GPIO_WritePin(HV_EN_GPIO_Port, HV_EN_Pin, GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_Delay(1);
	//Control the U13(ISL23315T) digital potentiometer via I2C
	//HW range = 4.21 ~ 11.63V
	_Float64 Vout = (_Float64)voltage_mv / 1000.0;
	_Float64 LT1615_R2 = LT1615_R2_calculate(BSP_LT1615_R1, Vout);
	_Float64 ISL23315T_RHW = LT1615_R2 - BSP_ISL23315T_RHGND;
	uint8_t WR = ISL23315T_RHW_to_WR_data(ISL23315T_RHW);
	return bsp_sp_ISL23315T_write(ISL23315T_MEM_ADDR_WR, WR);
}

/**
 * @brief Enable / Disable VDDS supply
 * 
 * @param enable Enable / Disable
 */
void app_func_stim_vdds_sup_enable(bool enable) {
	HAL_GPIO_WritePin(VDDS_EN_GPIO_Port, VDDS_EN_Pin, (enable)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
}

/**
 * @brief Enable / Disable VDDA supply
 * 
 * @param enable Enable / Disable
 */
void app_func_stim_vdda_sup_enable(bool enable) { /* parasoft-suppress MISRAC2012-RULE_8_7-a "Referenced by DVT firmware." */
	HAL_GPIO_WritePin(VDDA_EN_GPIO_Port, VDDA_EN_Pin, (enable)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
}

/**
 * @brief Initializes the DAC settings for stimulus generation.
 *
 * @param type DAC80501 or DAC80502 or all types.
 * @return uint8_t HAL status
 */
uint8_t app_func_stim_dac_init(uint8_t type) {
	uint8_t err = 0;
	uint16_t data[2] = {
			DAC8050x_FIE_SOFT_RESET,
			(DAC8050x_FIE_GAIN_REF_DIV_2 + DAC8050x_FIE_GAIN_BUFF1_GAIN_2 + DAC8050x_FIE_GAIN_BUFF2_GAIN_2),
	};

	if ((type & DACTYPE_SINE) == DACTYPE_SINE) {
		dac_write = DAC8050x_format_get(DAC8050x_REG_TRIGGER, data[0]);
		err |= bsp_sp_DAC80501_write(dac_write.Register, &dac_write.Data_MSB);
		HAL_Delay(5);
		dac_write = DAC8050x_format_get(DAC8050x_REG_GAIN, data[1]);
		err |= bsp_sp_DAC80501_write(dac_write.Register, &dac_write.Data_MSB);
		HAL_Delay(5);
		dac_read.Register = dac_write.Register;
		err |= bsp_sp_DAC80501_read(dac_read.Register, &dac_read.Data_MSB);
		if (memcmp(&dac_write, &dac_read, sizeof(DAC8050x_format_t)) != 0) {
			err |= HAL_ERROR;
		}
	}

	if ((type & DACTYPE_STIM) == DACTYPE_STIM) {
		dac_write = DAC8050x_format_get(DAC8050x_REG_TRIGGER, data[0]);
		err |= bsp_sp_DAC80502_write(dac_write.Register, &dac_write.Data_MSB);
		HAL_Delay(5);
		dac_write = DAC8050x_format_get(DAC8050x_REG_GAIN, data[1]);
		err |= bsp_sp_DAC80502_write(dac_write.Register, &dac_write.Data_MSB);
		HAL_Delay(5);
		dac_read.Register = dac_write.Register;
		err |= bsp_sp_DAC80502_read(dac_read.Register, &dac_read.Data_MSB);
		if (memcmp(&dac_write, &dac_read, sizeof(DAC8050x_format_t)) != 0) {
			err |= HAL_ERROR;
		}
	}
	return err;
}

/**
 * @brief Set the voltage of DAC
 * 
 * @param voltageA_mv The voltage of VOUTA, unit: mV
 * @param voltageB_mv The voltage of VOUTB, unit: mV
 * @return uint8_t HAL status
 */
uint8_t app_func_stim_dac_volt_set(uint16_t voltageA_mv, uint16_t voltageB_mv) {
	uint16_t data = 0U;
	uint8_t err = 0;

	data = DAC8050x_dac_vout_to_data(voltageA_mv, DAC8050x_VREF_INT_MV, DAC8050x_VREF_DIV_2, DAC8050x_GAIN_2);
	dac_write = DAC8050x_format_get(DAC8050x_REG_DAC1, data);
	err |= bsp_sp_DAC80502_write(dac_write.Register, &dac_write.Data_MSB);

	dac_read.Register = dac_write.Register;
	err |= bsp_sp_DAC80502_read(dac_read.Register, &dac_read.Data_MSB);
	if (memcmp(&dac_write, &dac_read, sizeof(DAC8050x_format_t)) != 0) {
		err |= HAL_ERROR;
	}

	data = DAC8050x_dac_vout_to_data(voltageB_mv, DAC8050x_VREF_INT_MV, DAC8050x_VREF_DIV_2, DAC8050x_GAIN_2);
	dac_write = DAC8050x_format_get(DAC8050x_REG_DAC2, data);
	err |= bsp_sp_DAC80502_write(dac_write.Register, &dac_write.Data_MSB);

	dac_read.Register = dac_write.Register;
	err |= bsp_sp_DAC80502_read(dac_read.Register, &dac_read.Data_MSB);
	if (memcmp(&dac_write, &dac_read, sizeof(DAC8050x_format_t)) != 0) {
		err |= HAL_ERROR;
	}

	(void)memset(&pulseWave.ramp, 0, sizeof(Ramp_t));

	return err;
}

/**
 * @brief Set the ramp voltage of DAC
 *
 * @param ramp_up_duration_ms The duration of the ramp up, unit: ms
 * @param ramp_down_duration_ms The duration of the ramp down, unit: ms
 * @param voltage_mv The max voltage of VOUTA & VOUTB, unit: mV
 * @return uint8_t HAL status
 */
uint8_t app_func_stim_dac_ramp_volt_set(uint32_t ramp_up_duration_ms, uint32_t ramp_down_duration_ms, uint16_t voltage_mv) {
	uint8_t err = 0;

	pulseWave.ramp.rampUpDuration_us 	= ramp_up_duration_ms * 1000;
	pulseWave.ramp.rampDownDuration_us 	= ramp_down_duration_ms * 1000;
	pulseWave.ramp.amplitude_mV 		= voltage_mv;

	uint16_t data = DAC8050x_dac_vout_to_data(0, DAC8050x_VREF_INT_MV, DAC8050x_VREF_DIV_2, DAC8050x_GAIN_2);
	dac_write = DAC8050x_format_get(DAC8050x_REG_BRDCAST, data);
	err |= bsp_sp_DAC80502_write(dac_write.Register, &dac_write.Data_MSB);

	dac_read.Register = dac_write.Register;
	err |= bsp_sp_DAC80502_read(dac_read.Register, &dac_read.Data_MSB);
	if (memcmp(&dac_write, &dac_read, sizeof(DAC8050x_format_t)) != 0) {
		err |= HAL_ERROR;
	}

	return err;
}

/**
 * @brief Enable / disable stimulus output
 * 
 * @param enable Enable / disable
 */
void app_func_stim_stimulus_enable(bool enable) {
	HAL_GPIO_WritePin(STIM_EN_GPIO_Port, STIM_EN_Pin, (enable)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
}

/**
 * @brief Enable and set the multiplexer of the stimulus output
 *
 */
void app_func_stim_stimulus_mux_enable(void) {
	bsp_mux_enable(true);
	bsp_mux_sr_sel_set(SEL_SR_STIM, SEL_SR_STIM);
}

/**
 * @brief Set the waveform settings
 * 
 * @param stimulus_waveform The waveform settings
 */
void app_func_stim_circuit_para_set(Stimulus_Waveform_t stimulus_waveform) {
	pulseWave.pwm_pulse_width_us = stimulus_waveform.pulseWidth_us;
	pulseWave.pwm_period_us = stimulus_waveform.pulsePeriod_us / 2;

	pulseWave.train_period_us = (stimulus_waveform.trainOnDuration_ms + stimulus_waveform.trainOffDuration_ms) * 1000;
	pulseWave.train_on_duration_us = stimulus_waveform.trainOnDuration_ms * 1000;

	pulseWave.ramp.rampUpStart_us = 0;
	pulseWave.ramp.rampUpEnd_us = pulseWave.ramp.rampUpStart_us + pulseWave.ramp.rampUpDuration_us;
	pulseWave.ramp.rampDownStart_us = stimulus_waveform.trainOnDuration_ms * 1000.0 - pulseWave.ramp.rampDownDuration_us;
	pulseWave.ramp.rampDownEnd_us = stimulus_waveform.trainOnDuration_ms * 1000.0;
	pulseWave.ramp.period_us = (stimulus_waveform.trainOnDuration_ms + stimulus_waveform.trainOffDuration_ms) * 1000.0;

	if (pulseWave.is_running) {
		__HAL_TIM_SET_AUTORELOAD(&HANDLE_STIMULATION_TIM, pulseWave.pwm_period_us - 1);
		__HAL_TIM_SET_COMPARE(&HANDLE_STIMULATION_TIM, TIM_CHANNEL_1, pulseWave.pwm_pulse_width_us - 1);
		__HAL_TIM_SET_COMPARE(&HANDLE_STIMULATION_TIM, TIM_CHANNEL_2, pulseWave.pwm_pulse_width_us);
		__HAL_TIM_SET_COMPARE(&HANDLE_STIMULATION_TIM, TIM_CHANNEL_3, pulseWave.pwm_pulse_width_us);
		__HAL_TIM_SET_COUNTER(&HANDLE_STIMULATION_TIM, 0);
	}
}

/**
 * @brief Set current source configuration
 *
 * @param current_sources The current source configuration
 */
void app_func_stim_curr_src_set(Current_Sources_t current_sources) {
	pulseWave.src_sink_positive 	= current_sources;
	Src_Sink_t* p_config_positive 	= (Src_Sink_t*)&pulseWave.src_sink_positive;
	Src_Sink_t* p_config_negative 	= (Src_Sink_t*)&pulseWave.src_sink_negative;
	Src_Sink_t* p_config_discharge 	= (Src_Sink_t*)&pulseWave.src_sink_discharge;
	for(uint8_t i=0;i<(uint8_t)(sizeof(Current_Sources_t)/sizeof(Src_Sink_t));i++) {
		p_config_negative[i].src 	= p_config_positive[i].sink;
		p_config_negative[i].sink 	= p_config_positive[i].src;
		p_config_discharge[i].src   = false;
		p_config_discharge[i].sink 	= p_config_positive[i].src | p_config_positive[i].sink;
	}

	if (!pulseWave.is_running && !sineWave.is_running) {
		src_sink_pin_set(pulseWave.src_sink_positive, true);
	}
}

/**
 * @brief Generate stimulation waveforms based on waveform settings and current source settings
 *
 * @param ph1_enable Enables the function of pin "PH1"
 * @param ph2_enable Enables the function of pin "PH2"
 * @param one_pulse Enables the one pulse cycle function of the stimulus waveform
 */
void app_func_stim_stimulus_start(bool ph1_enable, bool ph2_enable, bool one_pulse) {
	if (pulseWave.is_running) {
		return;
	}

	pulseWave.one_pulse_cycle 	= one_pulse;
	pulseWave.train_timer_us 	= 0;
	pulseWave.ramp.timer_us 	= 0;
	pulseWave.is_positive 		= true;
	pulseWave.pause_output 		= false;

	HAL_GPIO_WritePin(PG_START_GPIO_Port, PG_START_Pin, GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_Delay(1);

	__HAL_TIM_SET_AUTORELOAD(&HANDLE_STIMULATION_TIM, pulseWave.pwm_period_us - 1);

	if (ph2_enable) {
		__HAL_TIM_SET_COMPARE(&HANDLE_STIMULATION_TIM, TIM_CHANNEL_2, pulseWave.pwm_pulse_width_us);
		HAL_ERROR_CHECK(HAL_TIM_OC_Start_IT(&HANDLE_STIMULATION_TIM, TIM_CHANNEL_2));
	}

	if (ph1_enable) {
		if (pulseWave.one_pulse_cycle) {
			__HAL_TIM_SET_COMPARE(&HANDLE_STIMULATION_TIM, TIM_CHANNEL_3, 0);
			HAL_ERROR_CHECK(HAL_TIM_OC_Start_IT(&HANDLE_STIMULATION_TIM, TIM_CHANNEL_3));
			__HAL_TIM_SET_COMPARE(&HANDLE_STIMULATION_TIM, TIM_CHANNEL_3, pulseWave.pwm_pulse_width_us);
		}
		else {
			__HAL_TIM_SET_COMPARE(&HANDLE_STIMULATION_TIM, TIM_CHANNEL_3, pulseWave.pwm_pulse_width_us);
			HAL_ERROR_CHECK(HAL_TIM_OC_Start_IT(&HANDLE_STIMULATION_TIM, TIM_CHANNEL_3));
		}
	}

	__HAL_TIM_SET_COMPARE(&HANDLE_STIMULATION_TIM, TIM_CHANNEL_1, pulseWave.pwm_pulse_width_us - 1);
	HAL_ERROR_CHECK(HAL_TIM_Base_Start_IT(&HANDLE_STIMULATION_TIM));
	HAL_ERROR_CHECK(HAL_TIM_PWM_Start_IT(&HANDLE_STIMULATION_TIM, TIM_CHANNEL_1));
	__HAL_TIM_SET_COUNTER(&HANDLE_STIMULATION_TIM, 0);
	pulseWave.is_running = true;
}

/**
 * @brief Stop stimulation waveform
 * 
 */
void app_func_stim_stimulus_stop(void) {
	if (pulseWave.is_running) {
		HAL_ERROR_CHECK(HAL_TIM_Base_Stop_IT(&HANDLE_STIMULATION_TIM));
		HAL_ERROR_CHECK(HAL_TIM_PWM_Stop_IT(&HANDLE_STIMULATION_TIM, TIM_CHANNEL_1));
		HAL_ERROR_CHECK(HAL_TIM_OC_Stop_IT(&HANDLE_STIMULATION_TIM, TIM_CHANNEL_2));
		HAL_ERROR_CHECK(HAL_TIM_OC_Stop_IT(&HANDLE_STIMULATION_TIM, TIM_CHANNEL_3));
		src_sink_pin_set(pulseWave.src_sink_discharge, false);
		(void)memset(&pulseWave, 0, sizeof(pulseWave));
	}
}

/**
 * @brief Timer callback of stimulus waveform
 *
 * @param on_pulse_start Callback at the start or end of the pulse
 */
void app_func_stim_stimulus_cb(bool on_pulse_start) {
	pulse_state = on_pulse_start;
	if (on_pulse_start) {
		uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&HANDLE_STIMULATION_TIM) + 1;
		uint32_t cnt = __HAL_TIM_GET_COUNTER(&HANDLE_STIMULATION_TIM);
		uint32_t curr_timer = (pulseWave.train_timer_us + cnt) % pulseWave.train_period_us;
		pulseWave.train_timer_us = (pulseWave.train_timer_us + arr) % pulseWave.train_period_us;

		if (curr_timer < pulseWave.train_on_duration_us && !pulseWave.pause_output) {
			if (pulseWave.is_positive) {
				src_sink_pin_set(pulseWave.src_sink_positive, false);
			}
			else {
				src_sink_pin_set(pulseWave.src_sink_negative, false);
			}
		}
		else {
			src_sink_pin_set(pulseWave.src_sink_discharge, false);
		}
	}
	else {
		src_sink_pin_set(pulseWave.src_sink_discharge, false);
		pulseWave.is_positive = !pulseWave.is_positive;

		if (memcmp(&pulseWave.ramp, &(Ramp_t){0}, sizeof(Ramp_t)) != 0) {
			uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&HANDLE_STIMULATION_TIM) + 1;
			pulseWave.ramp.timer_us = (pulseWave.ramp.timer_us + arr) % pulseWave.ramp.period_us;

			uint16_t ramp_amplitude = ramp_amplitude_latest;
			if (pulseWave.ramp.timer_us >= pulseWave.ramp.rampUpStart_us && pulseWave.ramp.timer_us < pulseWave.ramp.rampUpEnd_us) {
				uint32_t sine_timer = pulseWave.ramp.timer_us;
				ramp_amplitude = (uint16_t)generate_sine_wave(pulseWave.ramp.rampUpDuration_us * 4, sine_timer, (_Float64)pulseWave.ramp.amplitude_mV);
			}
			else if (pulseWave.ramp.timer_us >= pulseWave.ramp.rampUpEnd_us && pulseWave.ramp.timer_us < pulseWave.ramp.rampDownStart_us) {
				ramp_amplitude = pulseWave.ramp.amplitude_mV;
			}
			else if (pulseWave.ramp.timer_us >= pulseWave.ramp.rampDownStart_us && pulseWave.ramp.timer_us < pulseWave.ramp.rampDownEnd_us) {
				uint32_t sine_timer = pulseWave.ramp.timer_us - pulseWave.ramp.rampDownStart_us + pulseWave.ramp.rampDownDuration_us;
				ramp_amplitude = (uint16_t)generate_sine_wave(pulseWave.ramp.rampDownDuration_us * 4, sine_timer, (_Float64)pulseWave.ramp.amplitude_mV);
			}
			else {
				ramp_amplitude = 0;
			}

			if (ramp_amplitude != ramp_amplitude_latest) {
				ramp_amplitude_latest = ramp_amplitude;
				uint16_t data = DAC8050x_dac_vout_to_data(ramp_amplitude, DAC8050x_VREF_INT_MV, DAC8050x_VREF_DIV_2, DAC8050x_GAIN_2);
				dac_write = DAC8050x_format_get(DAC8050x_REG_BRDCAST, data);
				bsp_sp_DAC80502_write_IT(dac_write.Register, &dac_write.Data_MSB);
			}
		}

		if (pulseWave.one_pulse_cycle) {
			app_func_stim_stimulus_stop();
		}
	}
}

/**
 * @brief Generate sine waveform based on the waveform settings
 *
 * @param nerveBlock_waveform The waveform settings
 */
void app_func_stim_sine_start(NerveBlock_Waveform_t nerveBlock_waveform) {
	sineWave.period_us 			= nerveBlock_waveform.sinePeriod_us;
	sineWave.update_interval_us = (uint32_t)(1000000 / (_Float64)nerveBlock_waveform.dacUpdateRate_Hz);
	sineWave.timer_us 			= nerveBlock_waveform.sinePhaseShift_us;
	sineWave.amplitude_mV 		= nerveBlock_waveform.amplitude_mV;
	sineWave.pause_output 		= false;

	(void)memset(&sineWave.src_sink_positive, 0, sizeof(Current_Sources_t));
	(void)memset(&sineWave.src_sink_negative, 0, sizeof(Current_Sources_t));
	(void)memset(&sineWave.src_sink_discharge, 0, sizeof(Current_Sources_t));
	sineWave.src_sink_positive.s7.src 	= true;
	sineWave.src_sink_positive.s8.sink 	= true;
	sineWave.src_sink_negative.s8.src 	= true;
	sineWave.src_sink_negative.s7.sink 	= true;
	sineWave.src_sink_discharge.s7.sink = true;
	sineWave.src_sink_discharge.s8.sink = true;

	__HAL_TIM_SET_COMPARE(&HANDLE_STIMULATION_TIM, TIM_CHANNEL_4, sineWave.update_interval_us);
	HAL_ERROR_CHECK(HAL_TIM_OC_Start_IT(&HANDLE_STIMULATION_TIM, TIM_CHANNEL_4));
	sineWave.is_running = true;

	uint16_t data = DAC8050x_dac_vout_to_data(0, DAC8050x_VREF_INT_MV, DAC8050x_VREF_DIV_2, DAC8050x_GAIN_2);
	dac_write = DAC8050x_format_get(DAC8050x_REG_DAC1, data);
	bsp_sp_DAC80501_write_IT(dac_write.Register, &dac_write.Data_MSB);
}

/**
 * @brief Stop sine wave
 *
 */
void app_func_stim_sine_stop(void) {
	if (sineWave.is_running) {
		HAL_ERROR_CHECK(HAL_TIM_OC_Stop_IT(&HANDLE_STIMULATION_TIM, TIM_CHANNEL_4));
		uint16_t data = DAC8050x_dac_vout_to_data(0, DAC8050x_VREF_INT_MV, DAC8050x_VREF_DIV_2, DAC8050x_GAIN_2);
		dac_write = DAC8050x_format_get(DAC8050x_REG_DAC1, data);
		bsp_sp_DAC80501_write(dac_write.Register, &dac_write.Data_MSB);
		src_sink_pin_set(sineWave.src_sink_discharge, false);
		(void)memset(&sineWave, 0, sizeof(sineWave));
	}
}

/**
 * @brief Timer callback of the sine wave
 *
 */
void app_func_stim_sine_cb(void) {
	uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&HANDLE_STIMULATION_TIM) + 1;
	uint32_t cnt = __HAL_TIM_GET_COUNTER(&HANDLE_STIMULATION_TIM);
	uint32_t cnt_next = (cnt + sineWave.update_interval_us) % arr;
	__HAL_TIM_SET_COMPARE(&HANDLE_STIMULATION_TIM, TIM_CHANNEL_4, cnt_next);

	uint32_t curr_timer = (sineWave.timer_us / sineWave.update_interval_us * sineWave.update_interval_us + cnt) % sineWave.period_us;
	if (cnt_next <= cnt) {
		sineWave.timer_us = (sineWave.timer_us + arr) % sineWave.period_us;
	}

	_Float64 vout = generate_sine_wave(sineWave.period_us, curr_timer, (_Float64)sineWave.amplitude_mV);
	if (!sineWave.pause_output) {
		if (vout >= 0.0) {
			src_sink_pin_set(sineWave.src_sink_positive, false);
		}
		else {
			src_sink_pin_set(sineWave.src_sink_negative, false);
		}
	}
	else {
		src_sink_pin_set(sineWave.src_sink_discharge, false);
		vout = 0.0;
	}
	uint16_t sine_vout = (uint16_t)fabs(vout);
	uint16_t data = DAC8050x_dac_vout_to_data(sine_vout, DAC8050x_VREF_INT_MV, DAC8050x_VREF_DIV_2, DAC8050x_GAIN_2);
	dac_write = DAC8050x_format_get(DAC8050x_REG_DAC1, data);
	bsp_sp_DAC80501_write_IT(dac_write.Register, &dac_write.Data_MSB);
}

/**
 * @brief Forces the outputs PH1_OUT (high) and PH2_OUT (low).
 *
 */
void app_func_stim_ph_out_force(void) {
	ph_out_force = true;
	HAL_GPIO_WritePin(PG_START_GPIO_Port, PG_START_Pin, GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_Delay(1);

	LL_TIM_OC_SetMode(HANDLE_STIMULATION_TIM.Instance, LL_TIM_CHANNEL_CH1,
			(LL_TIM_OC_GetPolarity(HANDLE_STIMULATION_TIM.Instance,
					LL_TIM_CHANNEL_CH1) == LL_TIM_OCPOLARITY_HIGH) ?
					LL_TIM_OCMODE_FORCED_ACTIVE :
					LL_TIM_OCMODE_FORCED_INACTIVE);

	LL_TIM_OC_SetMode(HANDLE_STIMULATION_TIM.Instance, LL_TIM_CHANNEL_CH2,
			(LL_TIM_OC_GetPolarity(HANDLE_STIMULATION_TIM.Instance,
					LL_TIM_CHANNEL_CH2) == LL_TIM_OCPOLARITY_LOW) ?
					LL_TIM_OCMODE_FORCED_ACTIVE :
					LL_TIM_OCMODE_FORCED_INACTIVE);

	LL_TIM_OC_SetMode(HANDLE_STIMULATION_TIM.Instance, LL_TIM_CHANNEL_CH3,
			(LL_TIM_OC_GetPolarity(HANDLE_STIMULATION_TIM.Instance,
					LL_TIM_CHANNEL_CH3) == LL_TIM_OCPOLARITY_HIGH) ?
					LL_TIM_OCMODE_FORCED_ACTIVE :
					LL_TIM_OCMODE_FORCED_INACTIVE);

	HAL_ERROR_CHECK(HAL_TIM_OC_Start(&HANDLE_STIMULATION_TIM, TIM_CHANNEL_1));
	HAL_ERROR_CHECK(HAL_TIM_OC_Start(&HANDLE_STIMULATION_TIM, TIM_CHANNEL_2));
	HAL_ERROR_CHECK(HAL_TIM_OC_Start(&HANDLE_STIMULATION_TIM, TIM_CHANNEL_3));
	HAL_ERROR_CHECK(HAL_TIM_PWM_Start_IT(&HANDLE_FORCE_PH_OUT_TIM, TIM_CHANNEL_1));
}

/**
 * @brief Stop the outputs PH1_OUT and PH2_OUT.
 *
 */
void app_func_stim_ph_out_stop(void) {
	if (ph_out_force) {
		HAL_ERROR_CHECK(HAL_TIM_OC_Stop_IT(&HANDLE_STIMULATION_TIM, TIM_CHANNEL_1));
		HAL_ERROR_CHECK(HAL_TIM_OC_Stop_IT(&HANDLE_STIMULATION_TIM, TIM_CHANNEL_2));
		HAL_ERROR_CHECK(HAL_TIM_OC_Stop_IT(&HANDLE_STIMULATION_TIM, TIM_CHANNEL_3));
		HAL_ERROR_CHECK(HAL_TIM_PWM_Stop_IT(&HANDLE_FORCE_PH_OUT_TIM, TIM_CHANNEL_1));
		ph_out_force = false;
	}
}

/**
 * @brief Timer callback to force output.
 *
 */
void app_func_stim_ph_out_force_cb(void) {
	if (ph_out_force) {
		HAL_GPIO_TogglePin(PG_START_GPIO_Port, PG_START_Pin);
	}
}

/**
 * @brief Generate VNSb waveform based on the waveform settings
 *
 * @param VNSb_waveform The waveform settings
 */
void app_func_stim_VNSb_start(VNSb_Waveform_t VNSb_waveform) {
	vnsbWave.train_on_duration_us 	= VNSb_waveform.trainOnDuration_ms * 1000;
	vnsbWave.train_period_us 		= (VNSb_waveform.trainOnDuration_ms + VNSb_waveform.trainOffDuration_ms) * 1000;

	_Float64 pulse_period_us = 1000000.0 / (_Float64)VNSb_waveform.pulseFrequency_Hz;
	_Float64 sine_period_ms = 1000.0 / (_Float64)VNSb_waveform.sineFrequency_Hz;
	_Float64 sine_period_us = sine_period_ms * 1000.0;
	_Float64 sine_phase_shift_us = sine_period_us * 0.15 + pulse_period_us;
	_Float64 sine_update_rate_Hz = (_Float64)VNSb_waveform.sineFrequency_Hz * 1000.0;
	vnsbWave.train_timer_us 		= sine_phase_shift_us;

	Stimulus_Waveform_t parameters = {
			.pulseWidth_us 			= VNSb_waveform.pulseWidth_us,
			.pulsePeriod_us 		= (uint32_t)pulse_period_us,
			.trainOnDuration_ms 	= (uint32_t)(sine_period_ms * 0.2),
			.trainOffDuration_ms 	= (uint32_t)(sine_period_ms * 0.3),
	};
	app_func_stim_circuit_para_set(parameters);

	NerveBlock_Waveform_t sine_parameters = {
			.sinePeriod_us 		= (uint32_t)sine_period_us,
			.sinePhaseShift_us 	= (uint32_t)sine_phase_shift_us,
			.dacUpdateRate_Hz 	= (uint32_t)sine_update_rate_Hz,
			.amplitude_mV 		= VNSb_waveform.sineAmplitude_mV,
	};

	app_func_stim_stimulus_start(true, true, false);
	app_func_stim_sine_start(sine_parameters);
}

/**
 * @brief Stop VNSb waveform
 *
 */
void app_func_stim_VNSb_stop(void) {
	app_func_stim_stimulus_stop();
	app_func_stim_sine_stop();
	(void)memset(&vnsbWave, 0, sizeof(vnsbWave));
}

/**
 * @brief Timer callback of the VNSb waveform
 *
 */
void app_func_stim_VNSb_cb(void) {
	if (vnsbWave.train_on_duration_us > 0 && vnsbWave.train_period_us >= vnsbWave.train_on_duration_us) {
		uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&HANDLE_STIMULATION_TIM) + 1;
		uint32_t cnt = __HAL_TIM_GET_COUNTER(&HANDLE_STIMULATION_TIM);
		uint32_t curr_timer = (vnsbWave.train_timer_us + cnt) % vnsbWave.train_period_us;
		vnsbWave.train_timer_us = (vnsbWave.train_timer_us + arr) % vnsbWave.train_period_us;

		if (curr_timer >= vnsbWave.train_on_duration_us) {
			pulseWave.pause_output = true;
			sineWave.pause_output = true;
		}
		else {
			pulseWave.pause_output = false;
			sineWave.pause_output = false;
		}
	}
}

/**
 * @brief Turn off all peripheral circuits of stimulation
 * 
 */
void app_func_stim_off(void) {
	Current_Sources_t configuration;
	(void)memset(&configuration, 0, sizeof(configuration));
	app_func_stim_stimulus_stop();
	app_func_stim_sine_stop();
	src_sink_pin_set(configuration, true);
	app_func_stim_stimulus_enable(false);
	app_func_stim_vdds_sup_enable(false);
	app_func_stim_vdda_sup_enable(false);
	app_func_stim_hv_supply_set(false, false);
}

