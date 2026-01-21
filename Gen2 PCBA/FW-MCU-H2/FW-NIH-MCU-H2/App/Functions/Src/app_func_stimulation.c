/**
 * @file app_func_stimulation.c
 * @brief This file provides stimulation management
 * @copyright Copyright (c) 2024
 */
#include "app_func_stimulation.h"
#include "app_config.h"

#include "stm32u5xx_ll_tim.h"

#define	BER_HI_TIME_US	10
#define	ZERO_HOLD		0.0

PulseWave_t pulseWave1 = {0};
PulseWave_t pulseWave2 = {0};
SineWave_t sineWave = {0};

Stim_Sel_t stimSel = {0};
Current_Sources_t srcSnk = {0};

uint16_t ramp_amplitude = 0;

static DAC8050x_format_t dac_write = {0};
static DAC8050x_format_t dac_read = {0};

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
    if (point > total_points)
    	return 0.0;

    if (ZERO_HOLD == 0.0) {
    	_Float64 angle = (2.0 * M_PI * point) / total_points;
    	return amplitude * sin(angle);
    }
    else {
		uint32_t half_points = total_points / 2;
		uint32_t zero_hold_points = (uint32_t)(ZERO_HOLD * half_points);
		uint32_t pos_in_half = point % half_points;

		if (pos_in_half < zero_hold_points)
			return 0.0;

		_Float64 effective_points = (half_points - zero_hold_points);
		_Float64 phase = (pos_in_half - zero_hold_points) / effective_points;
		_Float64 angle = M_PI * phase;

		if (point < half_points)
			return amplitude * sin(angle);
		else
			return -amplitude * sin(angle);
    }
}

/**
 * @brief Set the corresponding SRC according to STIM_SEL.CH channels
 *
 * @param change The U1500 and U1501 channels whose status is to be changed
 * @param connect Connect or disconnect SRC
 */
static void sel_ch_src_set(Stim_Sel_Ch_t change, bool connect) {
	if (change.ch1) {
		if (stimSel.stimA == STIMA_SEL_STIM1)
			HAL_GPIO_WritePin(SRC1_GPIO_Port, 	SRC1_Pin, 	(connect)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		else
			HAL_GPIO_WritePin(SRC2_GPIO_Port, 	SRC2_Pin, 	(connect)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}
	if (change.ch2) {
		if (stimSel.stimA == STIMA_SEL_STIM1)
			HAL_GPIO_WritePin(SRC1_GPIO_Port, 	SRC1_Pin, 	(connect)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		else
			HAL_GPIO_WritePin(SRC2_GPIO_Port, 	SRC2_Pin, 	(connect)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}
	if (change.ch3) {
		if (stimSel.stimB == STIMB_SEL_STIM1)
			HAL_GPIO_WritePin(SRC1_GPIO_Port, 	SRC1_Pin, 	(connect)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		else
			HAL_GPIO_WritePin(SRC2_GPIO_Port, 	SRC2_Pin, 	(connect)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}
	if (change.ch4) {
		if (stimSel.stimB == STIMB_SEL_STIM1)
			HAL_GPIO_WritePin(SRC1_GPIO_Port, 	SRC1_Pin, 	(connect)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		else
			HAL_GPIO_WritePin(SRC2_GPIO_Port, 	SRC2_Pin, 	(connect)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}
	if (change.encl) {
		if (stimSel.stimA == STIMA_SEL_STIM1)
			HAL_GPIO_WritePin(SRC1_GPIO_Port, 	SRC1_Pin, 	(connect)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		else
			HAL_GPIO_WritePin(SRC2_GPIO_Port, 	SRC2_Pin, 	(connect)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}
}

/**
 * @brief Set the corresponding SNK according to STIM_SEL.CH channels
 *
 * @param change The U1500 and U1501 channels whose status is to be changed
 * @param connect Connect or disconnect SNK
 */
static void sel_ch_snk_set(Stim_Sel_Ch_t change, bool connect) {
	if (change.ch1) {
		HAL_GPIO_WritePin(SNK1_GPIO_Port, 	SNK1_Pin, 	(connect)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}
	if (change.ch2) {
		HAL_GPIO_WritePin(SNK2_GPIO_Port, 	SNK2_Pin, 	(connect)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}
	if (change.ch3) {
		HAL_GPIO_WritePin(SNK3_GPIO_Port, 	SNK3_Pin, 	(connect)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}
	if (change.ch4) {
		HAL_GPIO_WritePin(SNK4_GPIO_Port, 	SNK4_Pin, 	(connect)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}
	if (change.encl) {
		HAL_GPIO_WritePin(SNK5_GPIO_Port, 	SNK5_Pin, 	(connect)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}
}

/**
 * @brief Select the STIM_SEL.CH channel of the output multiplexer
 *
 * @param newState New state of U1500 and U1501 channels
 * @param change Whether to change the state of U1500 and U1501 channels
 */
static void sel_ch_set(Stim_Sel_Ch_t newState, Stim_Sel_Ch_t change) {
	if (change.ch1) {
		HAL_GPIO_WritePin(STIM_SEL_CH1n_GPIO_Port, STIM_SEL_CH1n_Pin, (newState.ch1)?GPIO_PIN_RESET:GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		stimSel.sel_ch.ch1 = newState.ch1;
	}
	if (change.ch2) {
		HAL_GPIO_WritePin(STIM_SEL_CH2n_GPIO_Port, STIM_SEL_CH2n_Pin, (newState.ch2)?GPIO_PIN_RESET:GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		stimSel.sel_ch.ch2 = newState.ch2;
	}
	if (change.ch3) {
		HAL_GPIO_WritePin(STIM_SEL_CH3n_GPIO_Port, STIM_SEL_CH3n_Pin, (newState.ch3)?GPIO_PIN_RESET:GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		stimSel.sel_ch.ch3 = newState.ch3;
	}
	if (change.ch4) {
		HAL_GPIO_WritePin(STIM_SEL_CH4n_GPIO_Port, STIM_SEL_CH4n_Pin, (newState.ch4)?GPIO_PIN_RESET:GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		stimSel.sel_ch.ch4 = newState.ch4;
	}
	if (change.encl) {
		HAL_GPIO_WritePin(STIM_SEL_ENCLn_GPIO_Port, STIM_SEL_ENCLn_Pin, (newState.encl)?GPIO_PIN_RESET:GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		stimSel.sel_ch.encl = newState.encl;
	}
}

/**
 * @brief Select the STIM_SEL.CH channel of the output multiplexer and control the corresponding SRC and SNK
 *
 * @param newState New state of U1500 and U1501 channels
 * @param change Whether to change the state of U1500 and U1501 channels
 */
static void sel_ch_srcsnk_set(Stim_Sel_Ch_t newState, Stim_Sel_Ch_t change) {
	sel_ch_src_set(change, false);
	sel_ch_snk_set(change, false);

	sel_ch_set(newState, change);

	sel_ch_snk_set(change, true);
	sel_ch_src_set(change, true);
}

/**
 * @brief Set the impedance monitor channels based on the STIM multiplexer settings.
 *
 * @param newState New state of U1500 and U1501 channels
 * @param change Whether to change the state of U1500 and U1501 channels.
 */
static void imp_sel_set(Stim_Sel_Ch_t newState, Stim_Sel_Ch_t change) {
	uint8_t snkN_select = 0xFF;
	bool imp_n_sel0 = false;
	bool imp_n_sel1 = false;
	bool imp_n_sel2 = false;
	if (change.ch1 == true && newState.ch1 == false) {
		snkN_select = 1;
	}
	if (change.ch2 == true && newState.ch2 == false) {
		snkN_select = 2;
	}
	if (change.ch3 == true && newState.ch3 == false) {
		snkN_select = 3;
	}
	if (change.ch4 == true && newState.ch4 == false) {
		snkN_select = 4;
	}
	if (change.encl == true && newState.encl == false) {
		snkN_select = 5;
	}

	if (snkN_select <= 5) {
		imp_n_sel0 = ((snkN_select - 1) >> 0) & 0x01;
		imp_n_sel1 = ((snkN_select - 1) >> 1) & 0x01;
		imp_n_sel2 = ((snkN_select - 1) >> 2) & 0x01;
		HAL_GPIO_WritePin(IMP_IN_N_SEL0_GPIO_Port, IMP_IN_N_SEL0_Pin, (imp_n_sel0)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_GPIO_WritePin(IMP_IN_N_SEL1_GPIO_Port, IMP_IN_N_SEL1_Pin, (imp_n_sel1)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_GPIO_WritePin(IMP_IN_N_SEL2_GPIO_Port, IMP_IN_N_SEL2_Pin, (imp_n_sel2)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}
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
	HAL_Delay(10);
}

/**
 * @brief Set the voltage of HV supply
 * 
 * @param voltage_mv The voltage of HV supply, unit: mV
 * @return uint8_t HAL status
 */
uint8_t app_func_stim_hv_sup_volt_set(uint16_t voltage_mv) {
	HAL_GPIO_WritePin(HV_EN_GPIO_Port, HV_EN_Pin, GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_Delay(10);
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
 * @brief Initializes the DAC settings for stimulus generation.
 *
 * @return uint8_t HAL status
 */
uint8_t app_func_stim_dac_init(void) {
	uint8_t err = 0;
	uint16_t data[2] = {
			DAC8050x_FIE_SOFT_RESET,
			(DAC8050x_FIE_GAIN_REF_DIV_2 + DAC8050x_FIE_GAIN_BUFF1_GAIN_2 + DAC8050x_FIE_GAIN_BUFF2_GAIN_2),
	};

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

	return err;
}

/**
 * @brief Set the ramp settings of DAC1
 *
 * @param ramp_up_duration_ms The duration of the ramp up, unit: ms
 * @param ramp_down_duration_ms The duration of the ramp down, unit: ms
 * @param voltage_mv The max voltage of VOUTA, unit: mV
 */
void app_func_stim_dac1_ramp_set(uint32_t ramp_up_duration_ms, uint32_t ramp_down_duration_ms, uint16_t voltage_mv) {
	pulseWave1.ramp.rampUpDuration_us 	= ramp_up_duration_ms * 1000;
	pulseWave1.ramp.rampDownDuration_us = ramp_down_duration_ms * 1000;
	pulseWave1.ramp.max_amplitude_mV 	= voltage_mv;
	pulseWave1.ramp.curr_amplitude_mV 	= 0;

	pulseWave1.ramp.rampUpStart_us = 0;
	pulseWave1.ramp.rampUpEnd_us = pulseWave1.ramp.rampUpStart_us + pulseWave1.ramp.rampUpDuration_us;
	pulseWave1.ramp.rampDownStart_us = pulseWave1.train_on_duration_us - pulseWave1.ramp.rampDownDuration_us;
	pulseWave1.ramp.rampDownEnd_us = pulseWave1.train_on_duration_us;
	pulseWave1.ramp.period_us = pulseWave1.train_period_us;
}

/**
 * @brief   Calculate the required VDAC voltage to achieve a target output current
 *          in the current mirror circuit.
 *
 * @param iout_mA  Desired output current in mA
 * @return _Float64 The corresponding VDAC voltage in mV required to produce iout_mA
 */
_Float64 app_func_stim_iout_to_dac(_Float64 iout_mA)
{
	_Float64 iout_target = iout_mA / 1000.0f;
    if (iout_target <= 0.0f)
    	return 0.0f;

    _Float64 vdac = iout_target * (BSP_MIRROR_ROUT / BSP_MIRROR_RREF) * BSP_STIM_RREF;

    for (int iter = 0; iter < 20; iter++) {
    	_Float64 iref = 1.0 / BSP_STIM_RREF * vdac;
    	_Float64 f = iout_target * BSP_MIRROR_ROUT - BSP_VT * logf(iref / iout_target) - iref * BSP_MIRROR_RREF;
    	_Float64 df = -BSP_VT / vdac - 1.0 / BSP_STIM_RREF * BSP_MIRROR_RREF;

    	_Float64 delta = f / df;
        vdac -= delta;

        if (fabsf(delta) < 1e-12f)
        	break;

        if (vdac <= 0.0f) {
            vdac = 0.0f;
            break;
        }
    }

    if (vdac > BSP_DAC80502_VREF)
        vdac = BSP_DAC80502_VREF;

    return (vdac * 1000.0f);
}

/**
 * @brief Enable / disable VNSb stimulation output
 *
 * @param enable Enable / disable
 */
void app_func_stim_vnsb_enable(bool enable) {
	HAL_GPIO_WritePin(VNSb_EN_GPIO_Port, VNSb_EN_Pin, (enable)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
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
 * @param enable Enable / disable
 */
void app_func_stim_mux_enable(bool enable) {
	HAL_GPIO_WritePin(MUX_ENn_GPIO_Port, MUX_ENn_Pin, (enable)?GPIO_PIN_RESET:GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
}

/**
 * @brief Select the STIM_SEL channels of the output multiplexer.
 *
 * @param sel U1500 & U1501 Select Settings
 */
void app_func_stim_sel_set(Stim_Sel_t sel) {
	HAL_GPIO_WritePin(STIMA_SELn_GPIO_Port, STIMA_SELn_Pin, (sel.stimA)?GPIO_PIN_RESET:GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_GPIO_WritePin(STIMB_SELn_GPIO_Port, STIMB_SELn_Pin, (sel.stimB)?GPIO_PIN_RESET:GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_GPIO_WritePin(STIM_SEL_CH1n_GPIO_Port, STIM_SEL_CH1n_Pin, (sel.sel_ch.ch1)?GPIO_PIN_RESET:GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_GPIO_WritePin(STIM_SEL_CH2n_GPIO_Port, STIM_SEL_CH2n_Pin, (sel.sel_ch.ch2)?GPIO_PIN_RESET:GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_GPIO_WritePin(STIM_SEL_CH3n_GPIO_Port, STIM_SEL_CH3n_Pin, (sel.sel_ch.ch3)?GPIO_PIN_RESET:GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_GPIO_WritePin(STIM_SEL_CH4n_GPIO_Port, STIM_SEL_CH4n_Pin, (sel.sel_ch.ch4)?GPIO_PIN_RESET:GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_GPIO_WritePin(STIM_SEL_ENCLn_GPIO_Port, STIM_SEL_ENCLn_Pin, (sel.sel_ch.encl)?GPIO_PIN_RESET:GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */

	stimSel = sel;
}

/**
 * @brief Set current source configuration
 *
 * @param current_sources The current source configuration
 */
void app_func_stim_curr_src_set(Current_Sources_t current_sources) {
	srcSnk = current_sources;
	if (!pulseWave1.is_running && !pulseWave2.is_running && !sineWave.is_running) {
		HAL_GPIO_WritePin(SRC1_GPIO_Port, 	SRC1_Pin, 	(current_sources.src1)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_GPIO_WritePin(SRC2_GPIO_Port, 	SRC2_Pin, 	(current_sources.src2)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_GPIO_WritePin(SNK1_GPIO_Port, 	SNK1_Pin, 	(current_sources.snk1)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_GPIO_WritePin(SNK2_GPIO_Port, 	SNK2_Pin, 	(current_sources.snk2)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_GPIO_WritePin(SNK3_GPIO_Port, 	SNK3_Pin, 	(current_sources.snk3)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_GPIO_WritePin(SNK4_GPIO_Port, 	SNK4_Pin, 	(current_sources.snk4)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_GPIO_WritePin(SNK5_GPIO_Port, 	SNK5_Pin, 	(current_sources.snk5)?GPIO_PIN_SET:GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}
}

/**
 * @brief Set the waveform settings of stim1
 * 
 * @param stimulus_waveform The waveform settings
 */
void app_func_stim_circuit_para1_set(Stimulus_Waveform_t stimulus_waveform) {
	pulseWave1.pwm_pulse_width_us = stimulus_waveform.pulseWidth_us;
	pulseWave1.pwm_period_us = stimulus_waveform.pulsePeriod_us / 2;

	pulseWave1.train_period_us = (stimulus_waveform.trainOnDuration_ms + stimulus_waveform.trainOffDuration_ms) * 1000;
	pulseWave1.train_on_duration_us = stimulus_waveform.trainOnDuration_ms * 1000;

	if (pulseWave1.is_running) {
		__HAL_TIM_SET_AUTORELOAD(&HANDLE_PULSE1_TIM, pulseWave1.pwm_period_us - 1);
		__HAL_TIM_SET_COMPARE(&HANDLE_PULSE1_TIM, TIM_CHANNEL_1, pulseWave1.pwm_pulse_width_us);
		app_func_stim_sync();
	}
}

/**
 * @brief Set the waveform settings of stim2
 *
 * @param stimulus_waveform The waveform settings
 */
void app_func_stim_circuit_para2_set(Stimulus_Waveform_t stimulus_waveform) {
	pulseWave2.pwm_pulse_width_us = stimulus_waveform.pulseWidth_us;
	pulseWave2.pwm_period_us = stimulus_waveform.pulsePeriod_us / 2;

	pulseWave2.train_period_us = (stimulus_waveform.trainOnDuration_ms + stimulus_waveform.trainOffDuration_ms) * 1000;
	pulseWave2.train_on_duration_us = stimulus_waveform.trainOnDuration_ms * 1000;

	if (pulseWave2.is_running) {
		__HAL_TIM_SET_AUTORELOAD(&HANDLE_PULSE2_TIM, pulseWave2.pwm_period_us - 1);
		__HAL_TIM_SET_COMPARE(&HANDLE_PULSE2_TIM, TIM_CHANNEL_1, pulseWave2.pwm_pulse_width_us);
		app_func_stim_sync();
	}
}

/**
 * @brief Generate stim1 waveforms based on waveform settings and current source settings
 *
 * @param imc_en The enabled status of IMC
 */
void app_func_stim_stim1_start(bool imc_en) {
	if (pulseWave1.is_running) {
		return;
	}

	pulseWave1.train_timer_us 	= 0;
	pulseWave1.ramp.timer_us 	= 0;
	pulseWave1.is_positive 		= true;
	pulseWave1.pause_output 	= false;
	pulseWave1.imc_is_enabled	= imc_en;
	if (pulseWave1.imc_is_enabled) {
		pulseWave1.train_period_us = pulseWave1.train_on_duration_us;
	}

	pulseWave1.sel_positive 	= stimSel.sel_ch;

	pulseWave1.sel_negative.ch1 = !pulseWave1.sel_positive.ch1;
	pulseWave1.sel_negative.ch2 = !pulseWave1.sel_positive.ch2;
	pulseWave1.sel_negative.ch3 = !pulseWave1.sel_positive.ch3;
	pulseWave1.sel_negative.ch4 = !pulseWave1.sel_positive.ch4;
	pulseWave1.sel_negative.encl = !pulseWave1.sel_positive.encl;

	pulseWave1.sel_discharge.ch1 = STIM_SEL_CH1_SINK_CH1;
	pulseWave1.sel_discharge.ch2 = STIM_SEL_CH2_SINK_CH2;
	pulseWave1.sel_discharge.ch3 = STIM_SEL_CH3_SINK_CH3;
	pulseWave1.sel_discharge.ch4 = STIM_SEL_CH4_SINK_CH4;
	pulseWave1.sel_discharge.encl = STIM_SEL_ENCL_SINK_ENCL;

	memset(&pulseWave1.sel_enabled, 0, sizeof(Stim_Sel_Ch_t));
	if (srcSnk.src1 == true) {
		if (stimSel.stimA == STIMA_SEL_STIM1) {
			pulseWave1.sel_enabled.ch1 = srcSnk.snk1;
			pulseWave1.sel_enabled.ch2 = srcSnk.snk2;
			pulseWave1.sel_enabled.encl = srcSnk.snk5;
		}
		if (stimSel.stimB == STIMB_SEL_STIM1) {
			pulseWave1.sel_enabled.ch3 = srcSnk.snk3;
			pulseWave1.sel_enabled.ch4 = srcSnk.snk4;
		}
	}

	uint32_t switching_time = 0;
	if (pulseWave1.pwm_period_us >= BER_HI_TIME_US) {
		switching_time = pulseWave1.pwm_period_us - BER_HI_TIME_US;
		if (switching_time < pulseWave1.pwm_pulse_width_us) {
			switching_time = pulseWave1.pwm_pulse_width_us;
		}
	}

	__HAL_TIM_SET_AUTORELOAD(&HANDLE_PULSE1_TIM, pulseWave1.pwm_period_us - 1);
	HAL_ERROR_CHECK(HAL_TIM_Base_Start_IT(&HANDLE_PULSE1_TIM));

	__HAL_TIM_SET_COMPARE(&HANDLE_PULSE1_TIM, TIM_CH_PULSE1_TO_LOW, pulseWave1.pwm_pulse_width_us);
	HAL_ERROR_CHECK(HAL_TIM_PWM_Start_IT(&HANDLE_PULSE1_TIM, TIM_CH_PULSE1_TO_LOW));

	__HAL_TIM_SET_COMPARE(&HANDLE_PULSE1_TIM, TIM_CH_PULSE1_BEF_HI, switching_time);
	HAL_ERROR_CHECK(HAL_TIM_OC_Start_IT(&HANDLE_PULSE1_TIM, TIM_CH_PULSE1_BEF_HI));

	__HAL_TIM_SET_COUNTER(&HANDLE_PULSE1_TIM, 0);
	pulseWave1.is_running = true;
}

/**
 * @brief Generate stim2 waveforms based on waveform settings and current source settings
 *
 */
void app_func_stim_stim2_start(void) {
	if (pulseWave2.is_running) {
		return;
	}

	pulseWave2.train_timer_us 	= 0;
	pulseWave2.ramp.timer_us 	= 0;
	pulseWave2.is_positive 		= true;
	pulseWave2.pause_output 	= false;
	pulseWave2.imc_is_enabled	= false;

	pulseWave2.sel_positive 	= stimSel.sel_ch;

	pulseWave2.sel_negative.ch1 = !pulseWave2.sel_positive.ch1;
	pulseWave2.sel_negative.ch2 = !pulseWave2.sel_positive.ch2;
	pulseWave2.sel_negative.ch3 = !pulseWave2.sel_positive.ch3;
	pulseWave2.sel_negative.ch4 = !pulseWave2.sel_positive.ch4;
	pulseWave2.sel_negative.encl = !pulseWave2.sel_positive.encl;

	pulseWave2.sel_discharge.ch1 = STIM_SEL_CH1_SINK_CH1;
	pulseWave2.sel_discharge.ch2 = STIM_SEL_CH2_SINK_CH2;
	pulseWave2.sel_discharge.ch3 = STIM_SEL_CH3_SINK_CH3;
	pulseWave2.sel_discharge.ch4 = STIM_SEL_CH4_SINK_CH4;
	pulseWave2.sel_discharge.encl = STIM_SEL_ENCL_SINK_ENCL;

	memset(&pulseWave2.sel_enabled, 0, sizeof(Stim_Sel_Ch_t));
	if (srcSnk.src2 == true) {
		if (stimSel.stimA == STIMA_SEL_STIM2) {
			pulseWave2.sel_enabled.ch1 = srcSnk.snk1;
			pulseWave2.sel_enabled.ch2 = srcSnk.snk2;
			pulseWave2.sel_enabled.encl = srcSnk.snk5;
		}
		if (stimSel.stimB == STIMB_SEL_STIM2) {
			pulseWave2.sel_enabled.ch3 = srcSnk.snk3;
			pulseWave2.sel_enabled.ch4 = srcSnk.snk4;
		}
	}

	uint32_t switching_time = 0;
	if (pulseWave2.pwm_period_us >= BER_HI_TIME_US) {
		switching_time = pulseWave2.pwm_period_us - BER_HI_TIME_US;
		if (switching_time < pulseWave2.pwm_pulse_width_us) {
			switching_time = pulseWave2.pwm_pulse_width_us;
		}
	}

	__HAL_TIM_SET_AUTORELOAD(&HANDLE_PULSE2_TIM, pulseWave2.pwm_period_us - 1);
	HAL_ERROR_CHECK(HAL_TIM_Base_Start_IT(&HANDLE_PULSE2_TIM));

	__HAL_TIM_SET_COMPARE(&HANDLE_PULSE2_TIM, TIM_CH_PULSE2_TO_LOW, pulseWave2.pwm_pulse_width_us);
	HAL_ERROR_CHECK(HAL_TIM_PWM_Start_IT(&HANDLE_PULSE2_TIM, TIM_CH_PULSE2_TO_LOW));

	__HAL_TIM_SET_COMPARE(&HANDLE_PULSE2_TIM, TIM_CH_PULSE2_BEF_HI, switching_time);
	HAL_ERROR_CHECK(HAL_TIM_OC_Start_IT(&HANDLE_PULSE2_TIM, TIM_CH_PULSE2_BEF_HI));

	__HAL_TIM_SET_COUNTER(&HANDLE_PULSE2_TIM, 0);
	pulseWave2.is_running = true;
}

/**
 * @brief Stop stim1 waveform
 * 
 */
void app_func_stim_stim1_stop(void) {
	if (pulseWave1.is_running) {
		HAL_ERROR_CHECK(HAL_TIM_Base_Stop_IT(&HANDLE_PULSE1_TIM));
		HAL_ERROR_CHECK(HAL_TIM_PWM_Stop_IT(&HANDLE_PULSE1_TIM, TIM_CH_PULSE1_TO_LOW));
		HAL_ERROR_CHECK(HAL_TIM_OC_Stop_IT(&HANDLE_PULSE1_TIM, TIM_CH_PULSE1_BEF_HI));
		(void)memset(&pulseWave1, 0, sizeof(pulseWave1));
	}
}

/**
 * @brief Stop stim2 waveform
 *
 */
void app_func_stim_stim2_stop(void) {
	if (pulseWave2.is_running) {
		HAL_ERROR_CHECK(HAL_TIM_Base_Stop_IT(&HANDLE_PULSE2_TIM));
		HAL_ERROR_CHECK(HAL_TIM_PWM_Stop_IT(&HANDLE_PULSE2_TIM, TIM_CH_PULSE2_TO_LOW));
		HAL_ERROR_CHECK(HAL_TIM_OC_Stop_IT(&HANDLE_PULSE2_TIM, TIM_CH_PULSE2_BEF_HI));
		(void)memset(&pulseWave2, 0, sizeof(pulseWave2));
	}
}

/**
 * @brief Timer callback of stim1 waveform
 *
 * @param state Callback state
 */
void app_func_stim_stim1_cb(PWM_InterruptState state) {
	if (state == BEFORE_HIGH) {
		uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&HANDLE_PULSE1_TIM) + 1;
		uint32_t cnt = __HAL_TIM_GET_COUNTER(&HANDLE_PULSE1_TIM);
		uint32_t curr_timer = (pulseWave1.train_timer_us + cnt) % pulseWave1.train_period_us;
		pulseWave1.train_timer_us = (pulseWave1.train_timer_us + arr) % pulseWave1.train_period_us;

		if (curr_timer < pulseWave1.train_on_duration_us && !pulseWave1.pause_output) {
			if (pulseWave1.is_positive) {
				sel_ch_srcsnk_set(pulseWave1.sel_positive, pulseWave1.sel_enabled);
				if (pulseWave1.imc_is_enabled) {
					imp_sel_set(pulseWave1.sel_positive, pulseWave1.sel_enabled);
				}
			}
			else {
				sel_ch_srcsnk_set(pulseWave1.sel_negative, pulseWave1.sel_enabled);
				if (pulseWave1.imc_is_enabled) {
					imp_sel_set(pulseWave1.sel_negative, pulseWave1.sel_enabled);
				}
			}
		}
		else {
			sel_ch_srcsnk_set(pulseWave1.sel_discharge, pulseWave1.sel_enabled);
		}
	}
	else if (state == TO_HIGH) {

	}
	else if (state == TO_LOW) {
		if (!pulseWave1.imc_is_enabled) {
			sel_ch_srcsnk_set(pulseWave1.sel_discharge, pulseWave1.sel_enabled);
		}
		pulseWave1.is_positive = !pulseWave1.is_positive;

		if (memcmp(&pulseWave1.ramp, &(Ramp_t){0}, sizeof(Ramp_t)) != 0) {
			uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&HANDLE_PULSE1_TIM) + 1;
			pulseWave1.ramp.timer_us = (pulseWave1.ramp.timer_us + arr) % pulseWave1.ramp.period_us;

			if (pulseWave1.ramp.timer_us >= pulseWave1.ramp.rampUpStart_us && pulseWave1.ramp.timer_us < pulseWave1.ramp.rampUpEnd_us) {
				uint32_t sine_timer = pulseWave1.ramp.timer_us;
				ramp_amplitude = (uint16_t)generate_sine_wave(pulseWave1.ramp.rampUpDuration_us * 4, sine_timer, (_Float64)pulseWave1.ramp.max_amplitude_mV);
			}
			else if (pulseWave1.ramp.timer_us >= pulseWave1.ramp.rampUpEnd_us && pulseWave1.ramp.timer_us < pulseWave1.ramp.rampDownStart_us) {
				ramp_amplitude = pulseWave1.ramp.max_amplitude_mV;
			}
			else if (pulseWave1.ramp.timer_us >= pulseWave1.ramp.rampDownStart_us && pulseWave1.ramp.timer_us < pulseWave1.ramp.rampDownEnd_us) {
				uint32_t sine_timer = pulseWave1.ramp.timer_us - pulseWave1.ramp.rampDownStart_us + pulseWave1.ramp.rampDownDuration_us;
				ramp_amplitude = (uint16_t)generate_sine_wave(pulseWave1.ramp.rampDownDuration_us * 4, sine_timer, (_Float64)pulseWave1.ramp.max_amplitude_mV);
			}
			else {
				ramp_amplitude = 0;
			}

			if (ramp_amplitude != pulseWave1.ramp.curr_amplitude_mV) {
				pulseWave1.ramp.curr_amplitude_mV = ramp_amplitude;
				uint16_t data = DAC8050x_dac_vout_to_data(ramp_amplitude, DAC8050x_VREF_INT_MV, DAC8050x_VREF_DIV_2, DAC8050x_GAIN_2);
				dac_write = DAC8050x_format_get(DAC8050x_REG_DAC1, data);
				bsp_sp_DAC80502_write_IT(dac_write.Register, &dac_write.Data_MSB);
			}
		}
	}
}

/**
 * @brief Timer callback of stim2 waveform
 *
 * @param state Callback state
 */
void app_func_stim_stim2_cb(PWM_InterruptState state) {
	if (state == BEFORE_HIGH) {
		uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&HANDLE_PULSE2_TIM) + 1;
		uint32_t cnt = __HAL_TIM_GET_COUNTER(&HANDLE_PULSE2_TIM);
		uint32_t curr_timer = (pulseWave2.train_timer_us + cnt) % pulseWave2.train_period_us;
		pulseWave2.train_timer_us = (pulseWave2.train_timer_us + arr) % pulseWave2.train_period_us;

		if (curr_timer < pulseWave2.train_on_duration_us && !pulseWave2.pause_output) {
			if (pulseWave2.is_positive) {
				sel_ch_srcsnk_set(pulseWave2.sel_positive, pulseWave2.sel_enabled);
			}
			else {
				sel_ch_srcsnk_set(pulseWave2.sel_negative, pulseWave2.sel_enabled);
			}
		}
		else {
			sel_ch_srcsnk_set(pulseWave2.sel_discharge, pulseWave2.sel_enabled);
		}
	}
	else if (state == TO_HIGH) {
		HAL_GPIO_WritePin(VNSb_EN_GPIO_Port, VNSb_EN_Pin, GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	}
	else if (state == TO_LOW) {
		HAL_GPIO_WritePin(VNSb_EN_GPIO_Port, VNSb_EN_Pin, GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		sel_ch_srcsnk_set(pulseWave2.sel_discharge, pulseWave2.sel_enabled);
		pulseWave2.is_positive = !pulseWave2.is_positive;
	}
}

/**
 * @brief Set the waveform settings of sine
 *
 * @param nerveBlock_waveform The waveform settings
 */
void app_func_stim_sine_para_set(NerveBlock_Waveform_t nerveBlock_waveform) {
	sineWave.period_us 			= nerveBlock_waveform.sinePeriod_us;
	sineWave.update_interval_us = nerveBlock_waveform.sinePeriod_us / SINE_PERIOD_POINTS;
	sineWave.phaseShift_us		= nerveBlock_waveform.sinePhaseShift_us % nerveBlock_waveform.sinePeriod_us;
	sineWave.amplitude_mV 		= nerveBlock_waveform.amplitude_mV;

	sineWave.train_period_us = (nerveBlock_waveform.trainOnDuration_ms + nerveBlock_waveform.trainOffDuration_ms) * 1000;
	sineWave.train_on_duration_us = nerveBlock_waveform.trainOnDuration_ms * 1000;

	sineWave.sine_point_idx = (sineWave.phaseShift_us % sineWave.period_us) / sineWave.update_interval_us + 1U;
	sineWave.sine_point_idx = sineWave.sine_point_idx % SINE_PERIOD_POINTS;
	for (int i = 0; i < SINE_PERIOD_POINTS; i++) {
		sineWave.sine_points[i].tim_cnt = sineWave.update_interval_us * i;
		_Float64 vout = generate_sine_wave(sineWave.period_us, sineWave.sine_points[i].tim_cnt, (_Float64)sineWave.amplitude_mV);
		sineWave.sine_points[i].dac_cnt = DAC8050x_dac_vout_to_data((uint16_t)fabs(vout), DAC8050x_VREF_INT_MV, DAC8050x_VREF_DIV_2, DAC8050x_GAIN_2);
	}

	if (sineWave.is_running) {
		__HAL_TIM_SET_COMPARE(&HANDLE_SINE_TIM, TIM_CH_SINE_AMP, sineWave.sine_points[sineWave.sine_point_idx].tim_cnt);
		app_func_stim_sync();
	}
}

/**
 * @brief Generate sine waveform based on the waveform settings
 *
 */
void app_func_stim_sine_start(void) {
	sineWave.train_timer_us 	= 0;
	sineWave.pause_output 		= false;

	sineWave.sel_positive 		= stimSel.sel_ch;

	sineWave.sel_negative.ch1 = !sineWave.sel_positive.ch1;
	sineWave.sel_negative.ch2 = !sineWave.sel_positive.ch2;
	sineWave.sel_negative.ch3 = !sineWave.sel_positive.ch3;
	sineWave.sel_negative.ch4 = !sineWave.sel_positive.ch4;
	sineWave.sel_negative.encl = !sineWave.sel_positive.encl;

	sineWave.sel_discharge.ch1 = STIM_SEL_CH1_SINK_CH1;
	sineWave.sel_discharge.ch2 = STIM_SEL_CH2_SINK_CH2;
	sineWave.sel_discharge.ch3 = STIM_SEL_CH3_SINK_CH3;
	sineWave.sel_discharge.ch4 = STIM_SEL_CH4_SINK_CH4;
	sineWave.sel_discharge.encl = STIM_SEL_ENCL_SINK_ENCL;

	memset(&sineWave.sel_enabled, 0, sizeof(Stim_Sel_Ch_t));
	if (srcSnk.src2 == true) {
		if (stimSel.stimA == STIMA_SEL_STIM2) {
			sineWave.sel_enabled.ch1 = srcSnk.snk1;
			sineWave.sel_enabled.ch2 = srcSnk.snk2;
			sineWave.sel_enabled.encl = srcSnk.snk5;
		}
		if (stimSel.stimB == STIMB_SEL_STIM2) {
			sineWave.sel_enabled.ch3 = srcSnk.snk3;
			sineWave.sel_enabled.ch4 = srcSnk.snk4;
		}
	}

	HAL_GPIO_WritePin(VNSb_EN_GPIO_Port, VNSb_EN_Pin, GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */

	uint16_t data = DAC8050x_dac_vout_to_data(0, DAC8050x_VREF_INT_MV, DAC8050x_VREF_DIV_2, DAC8050x_GAIN_2);
	dac_write = DAC8050x_format_get(DAC8050x_REG_DAC2, data);
	bsp_sp_DAC80502_write(dac_write.Register, &dac_write.Data_MSB);

	if (sineWave.phaseShift_us < (sineWave.period_us / 2)) {
		sel_ch_srcsnk_set(sineWave.sel_positive, sineWave.sel_enabled);
	}
	else {
		sel_ch_srcsnk_set(sineWave.sel_negative, sineWave.sel_enabled);
	}

	__HAL_TIM_SET_AUTORELOAD(&HANDLE_SINE_TIM, sineWave.period_us - 1);
	HAL_ERROR_CHECK(HAL_TIM_Base_Start_IT(&HANDLE_SINE_TIM));

	__HAL_TIM_SET_COMPARE(&HANDLE_SINE_TIM, TIM_CH_SINE_POLR, sineWave.period_us / 2);
	HAL_ERROR_CHECK(HAL_TIM_PWM_Start_IT(&HANDLE_SINE_TIM, TIM_CH_SINE_POLR));

	__HAL_TIM_SET_COMPARE(&HANDLE_SINE_TIM, TIM_CH_SINE_AMP, sineWave.sine_points[sineWave.sine_point_idx].tim_cnt);
	HAL_ERROR_CHECK(HAL_TIM_OC_Start_IT(&HANDLE_SINE_TIM, TIM_CH_SINE_AMP));

	__HAL_TIM_SET_COUNTER(&HANDLE_SINE_TIM, sineWave.phaseShift_us);
	sineWave.is_running = true;
}

/**
 * @brief Stop sine wave
 *
 */
void app_func_stim_sine_stop(void) {
	if (sineWave.is_running) {
		HAL_ERROR_CHECK(HAL_TIM_Base_Stop_IT(&HANDLE_SINE_TIM));
		HAL_ERROR_CHECK(HAL_TIM_PWM_Stop_IT(&HANDLE_SINE_TIM, TIM_CH_SINE_POLR));
		HAL_ERROR_CHECK(HAL_TIM_OC_Stop_IT(&HANDLE_SINE_TIM, TIM_CH_SINE_AMP));
		(void)memset(&sineWave, 0, sizeof(sineWave));
	}
}

/**
 * @brief Timer callback of the sine wave
 *
 * @param state Callback state
 */
void app_func_stim_sine_cb(SINE_InterruptState state) {
	if (state == AMP) {
		uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&HANDLE_SINE_TIM) + 1;
		uint32_t cnt = __HAL_TIM_GET_COUNTER(&HANDLE_SINE_TIM);
		sineWave.sine_point_idx = (sineWave.sine_point_idx + 1U) % SINE_PERIOD_POINTS;
		__HAL_TIM_SET_COMPARE(&HANDLE_SINE_TIM, TIM_CH_SINE_AMP, sineWave.sine_points[sineWave.sine_point_idx].tim_cnt);

		uint32_t curr_timer = (sineWave.train_timer_us + cnt) % sineWave.train_period_us;
		if (sineWave.sine_points[sineWave.sine_point_idx].tim_cnt == 0) {
			sineWave.train_timer_us = (sineWave.train_timer_us + arr) % sineWave.train_period_us;
		}

		if (curr_timer < sineWave.train_on_duration_us) {
			sineWave.pause_output = false;
			dac_write = DAC8050x_format_get(DAC8050x_REG_DAC2, sineWave.sine_points[sineWave.sine_point_idx].dac_cnt);
		}
		else {
			sineWave.pause_output = true;
			dac_write = DAC8050x_format_get(DAC8050x_REG_DAC2, 0U);
		}
		bsp_sp_DAC80502_write_IT(dac_write.Register, &dac_write.Data_MSB);
	}
	else if (state == POLR_POS) {
		if (sineWave.pause_output) {
			sel_ch_srcsnk_set(sineWave.sel_discharge, sineWave.sel_enabled);
		}
		else {
			sel_ch_srcsnk_set(sineWave.sel_positive, sineWave.sel_enabled);
		}
	}
	else if (state == POLR_NEG) {
		if (sineWave.pause_output) {
			sel_ch_srcsnk_set(sineWave.sel_discharge, sineWave.sel_enabled);
		}
		else {
			sel_ch_srcsnk_set(sineWave.sel_negative, sineWave.sel_enabled);
		}
	}
}

/**
 * @brief Synchronizes the timers of all waveforms.
 *
 */
void app_func_stim_sync(void) {
	pulseWave1.train_timer_us 	= 0;
	pulseWave1.ramp.timer_us 	= 0;
	pulseWave1.is_positive 		= true;
	pulseWave2.train_timer_us 	= 0;
	pulseWave2.ramp.timer_us 	= 0;
	pulseWave2.is_positive 		= true;
	sineWave.train_timer_us 	= 0;

	__HAL_TIM_SET_COUNTER(&HANDLE_PULSE1_TIM, 0);
	__HAL_TIM_SET_COUNTER(&HANDLE_PULSE2_TIM, 0);
	__HAL_TIM_SET_COUNTER(&HANDLE_SINE_TIM, sineWave.phaseShift_us);
}

/**
 * @brief Turn off all peripheral circuits of stimulation
 * 
 */
void app_func_stim_off(void) {
	app_func_stim_stim1_stop();
	app_func_stim_stim2_stop();
	app_func_stim_sine_stop();

	app_func_stim_stimulus_enable(false);
	app_func_stim_mux_enable(false);
	app_func_stim_vnsb_enable(false);

	app_func_stim_vdds_sup_enable(false);
	app_func_stim_hv_supply_set(false, false);

	Current_Sources_t off = {false, false, false, false, false, false, false};
	app_func_stim_curr_src_set(off);
}

