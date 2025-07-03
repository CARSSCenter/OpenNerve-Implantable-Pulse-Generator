/**
 * @file app_func_stimulation.h
 * @brief This file contains all the function prototypes for the app_func_stimulation.c file
 * @copyright Copyright (c) 2024
 */
#ifndef FUNCTIONS_INC_APP_FUNC_STIMULATION_H_
#define FUNCTIONS_INC_APP_FUNC_STIMULATION_H_
#include <stdint.h>
#include <stdbool.h>

#define HV_SUPPLY_MV		11633U			/*!< The voltage of HV supply, unit: mV */
#define RAMP_STEPS_NUM		10U				/*!< The number of steps on the ramp */

#define DACTYPE_SINE		0b00000001U		/*!< The type of DAC is DAC80501 for sine waveform */
#define DACTYPE_STIM		0b00000010U		/*!< The type of DAC is DAC80502 for stimulus waveform */

typedef struct {
	uint32_t 	pulseWidth_us;				/*!< The pulse width of the stimulus waveform, unit: us */
	uint32_t 	pulsePeriod_us;				/*!< The pulse period of the stimulus waveform, unit: us */
	uint32_t 	trainOnDuration_ms;			/*!< The train on duration of the stimulus waveform, unit: ms */
	uint32_t 	trainOffDuration_ms;		/*!< The train off duration of the stimulus waveform, unit: ms */
} Stimulus_Waveform_t;

typedef struct {
	uint32_t	sinePeriod_us;
	uint32_t	sinePhaseShift_us;
	uint32_t	dacUpdateRate_Hz;
	uint16_t	amplitude_mV;
} NerveBlock_Waveform_t;

typedef struct {
	uint32_t 	pulseWidth_us;				/*!< The pulse width of the VNSb waveform, unit: us */
	uint32_t 	pulseFrequency_Hz;
	uint32_t	sineFrequency_Hz;
	uint32_t	sineAmplitude_mV;
	uint32_t 	trainOnDuration_ms;			/*!< The train on duration of the VNSb waveform, unit: ms */
	uint32_t 	trainOffDuration_ms;		/*!< The train off duration of the VNSb waveform, unit: ms */
} VNSb_Waveform_t;

typedef struct {
	bool src;								/*!< The default state of the source pin */
	bool sink;								/*!< The default state of the sink pin */
} Src_Sink_t;

typedef struct {
	Src_Sink_t s1;							/*!< Default state of source and sink pin group 1 */
	Src_Sink_t s2;							/*!< Default state of source and sink pin group 2 */
	Src_Sink_t s3;							/*!< Default state of source and sink pin group 3 */
	Src_Sink_t s4;							/*!< Default state of source and sink pin group 4 */
	Src_Sink_t s5;							/*!< Default state of source and sink pin group 5 */
	Src_Sink_t s6;							/*!< Default state of source and sink pin group 6 */
	Src_Sink_t s7;							/*!< Default state of source and sink pin group 7 */
	Src_Sink_t s8;							/*!< Default state of source and sink pin group 8 */
	Src_Sink_t s17;							/*!< Default state of source and sink pin group 17 */
} Current_Sources_t;

typedef struct {
	uint32_t 	rampUpDuration_us;				/*!< The duration of the ramp up, unit: us */
	uint32_t 	rampDownDuration_us;			/*!< The duration of the ramp down, unit: us */
	uint32_t 	rampUpStart_us;					/*!< Time to start ramp up, unit: us */
	uint32_t 	rampUpEnd_us;					/*!< Time to end ramp up, unit: us */
	uint32_t 	rampDownStart_us;				/*!< Time to start ramp down, unit: us */
	uint32_t 	rampDownEnd_us;					/*!< Time to end ramp down, unit: us */

	uint32_t 	period_us;						/*!< The period of the ramp, unit: us */
	uint32_t 	timer_us;						/*!< The timer of the ramp, unit: us */
	uint16_t	amplitude_mV;					/*!< The max amplitude of the ramp, unit: mV */
} Ramp_t;

typedef struct {
	uint32_t 	pwm_pulse_width_us;				/*!< The pulse width of the PWM, unit: us */
	uint32_t 	pwm_period_us;					/*!< The period of the PWM, unit: us */

	uint32_t 	train_period_us;				/*!< The period of the train signal, unit: us */
	uint32_t 	train_on_duration_us;			/*!< Duration of train-on time, unit: us */
	uint32_t 	train_timer_us;					/*!< The timer of the train signal, unit: us */

	bool		is_positive;					/*!< The direction of the waveform pulse is positive */
	bool		is_running;						/*!< The waveform is running */

	Current_Sources_t src_sink_positive;		/*!< Pin state for positive pulse */
	Current_Sources_t src_sink_negative;		/*!< Pin state for negative pulse */
	Current_Sources_t src_sink_discharge;		/*!< Pin state for discharge */

	bool		one_pulse_cycle;				/*!< Enables the one pulse cycle function of the stimulus waveform */
	bool		pause_output;					/*!< Pause the signal output. */

	Ramp_t 		ramp;							/*!< Ramp up and down settings. */
} PulseWave_t;

typedef struct {
	uint32_t 	period_us;						/*!< The period of the sine wave, unit: us */
	uint32_t 	update_interval_us;				/*!< The interval between DAC updates, unit: us */
	uint32_t 	timer_us;						/*!< The timer of the sine wave, unit: us */
	uint16_t	amplitude_mV;					/*!< The max amplitude of the sine wave, unit: mV */

	bool		is_running;						/*!< The waveform is running */

	Current_Sources_t src_sink_positive;		/*!< Pin state for positive pulse */
	Current_Sources_t src_sink_negative;		/*!< Pin state for negative pulse */
	Current_Sources_t src_sink_discharge;		/*!< Pin state for discharge */

	bool		pause_output;					/*!< Pause the signal output. */
} SineWave_t;

typedef struct {
	uint32_t 	train_period_us;				/*!< The period of the train signal, unit: us */
	uint32_t 	train_on_duration_us;			/*!< Duration of train-on time, unit: us */
	uint32_t 	train_timer_us;					/*!< The timer of the train signal, unit: us */
} VNSbWave_t;

/**
 * @brief Set the state of GPIOs of HV supply
 * 
 * @param turnon Set the state of pin "HVSW_EN"
 * @param enable Set the state of pins "VPPSW_EN" and "HV_EN"
 */
void app_func_stim_hv_supply_set(bool turnon, bool enable);

/**
 * @brief Set the voltage of HV supply
 * 
 * @param voltage_mv The voltage of HV supply, unit: mV
 * @return uint8_t HAL status
 */
uint8_t app_func_stim_hv_sup_volt_set(uint16_t voltage_mv);

/**
 * @brief Enable / Disable VDDS supply
 * 
 * @param enable Enable / Disable
 */
void app_func_stim_vdds_sup_enable(bool enable);

/**
 * @brief Enable / Disable VDDA supply
 * 
 * @param enable Enable / Disable
 */
void app_func_stim_vdda_sup_enable(bool enable);

/**
 * @brief Initializes the DAC settings for stimulus generation.
 *
 * @param type DAC80501 or DAC80502 or all types.
 * @return uint8_t HAL status
 */
uint8_t app_func_stim_dac_init(uint8_t type);

/**
 * @brief Set the voltage of DAC
 * 
 * @param voltageA_mv The voltage of VOUTA, unit: mV
 * @param voltageB_mv The voltage of VOUTB, unit: mV
 * @return uint8_t HAL status
 */
uint8_t app_func_stim_dac_volt_set(uint16_t voltageA_mv, uint16_t voltageB_mv);

/**
 * @brief Set the ramp voltage of DAC
 *
 * @param ramp_up_duration_ms The duration of the ramp up, unit: ms
 * @param ramp_down_duration_ms The duration of the ramp down, unit: ms
 * @param voltage_mv The max voltage of VOUTA & VOUTB, unit: mV
 * @return uint8_t HAL status
 */
uint8_t app_func_stim_dac_ramp_volt_set(uint32_t ramp_up_duration_ms, uint32_t ramp_down_duration_ms, uint16_t voltage_mv);

/**
 * @brief Enable / disable stimulus output
 * 
 * @param enable Enable / disable
 */
void app_func_stim_stimulus_enable(bool enable);

/**
 * @brief Enable and set the multiplexer of the stimulus output
 *
 */
void app_func_stim_stimulus_mux_enable(void);

/**
 * @brief Set the waveform settings
 *
 * @param stimulus_waveform The waveform settings
 */
void app_func_stim_circuit_para_set(Stimulus_Waveform_t stimulus_waveform);

/**
 * @brief Set current source configuration
 * 
 * @param current_sources The current source configuration
 */
void app_func_stim_curr_src_set(Current_Sources_t current_sources);

/**
 * @brief Generate stimulation waveforms based on waveform settings and current source settings
 *
 * @param ph1_enable Enables the function of pin "PH1"
 * @param ph2_enable Enables the function of pin "PH2"
 * @param one_pulse Enables the one pulse cycle function of the stimulus waveform
 */
void app_func_stim_stimulus_start(bool ph1_enable, bool ph2_enable, bool one_pulse);

/**
 * @brief Stop stimulation waveform
 * 
 */
void app_func_stim_stimulus_stop(void);

/**
 * @brief Timer callback of stimulus waveform
 *
 * @param on_pulse_start Callback at the start or end of the pulse
 */
void app_func_stim_stimulus_cb(bool on_pulse_start);

/**
 * @brief Generate sine waveform based on the waveform settings
 *
 * @param nerveBlock_waveform The waveform settings
 */
void app_func_stim_sine_start(NerveBlock_Waveform_t nerveBlock_waveform);

/**
 * @brief Stop sine wave
 *
 */
void app_func_stim_sine_stop(void);

/**
 * @brief Timer callback of the sine wave
 *
 */
void app_func_stim_sine_cb(void);

/**
 * @brief Forces the outputs PH1_OUT (high) and PH2_OUT (low).
 *
 */
void app_func_stim_ph_out_force(void);

/**
 * @brief Stop the outputs PH1_OUT and PH2_OUT.
 *
 */
void app_func_stim_ph_out_stop(void);

/**
 * @brief Timer callback to force output.
 *
 */
void app_func_stim_ph_out_force_cb(void);

/**
 * @brief Generate VNSb waveform based on the waveform settings
 *
 * @param VNSb_waveform The waveform settings
 */
void app_func_stim_VNSb_start(VNSb_Waveform_t VNSb_waveform);

/**
 * @brief Stop VNSb waveform
 *
 */
void app_func_stim_VNSb_stop(void);

/**
 * @brief Timer callback of the VNSb waveform
 *
 */
void app_func_stim_VNSb_cb(void);

/**
 * @brief Turn off all peripheral circuits of stimulation
 * 
 */
void app_func_stim_off(void);

#endif /* FUNCTIONS_INC_APP_FUNC_STIMULATION_H_ */
