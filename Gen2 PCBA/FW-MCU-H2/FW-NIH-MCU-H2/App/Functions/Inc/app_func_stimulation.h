/**
 * @file app_func_stimulation.h
 * @brief This file contains all the function prototypes for the app_func_stimulation.c file
 * @copyright Copyright (c) 2024
 */
#ifndef FUNCTIONS_INC_APP_FUNC_STIMULATION_H_
#define FUNCTIONS_INC_APP_FUNC_STIMULATION_H_
#include <stdint.h>
#include <stdbool.h>

#define HV_SUPPLY_MV			11633U			/*!< The voltage of HV supply, unit: mV */
#define RAMP_STEPS_NUM			10U				/*!< The number of steps on the ramp */
#define	SINE_PERIOD_POINTS		100U			/*!< The number of points on the sine period */

#define	STIMA_SEL_STIM1			false
#define	STIMA_SEL_STIM2			true

#define	STIMB_SEL_STIM1			true
#define	STIMB_SEL_STIM2			false

#define	STIM_SEL_CH1_STIMA		true
#define	STIM_SEL_CH1_SINK_CH1	false

#define	STIM_SEL_CH2_STIMA		true
#define	STIM_SEL_CH2_SINK_CH2	false

#define	STIM_SEL_CH3_STIMB		true
#define	STIM_SEL_CH3_SINK_CH3	false

#define	STIM_SEL_CH4_STIMB		true
#define	STIM_SEL_CH4_SINK_CH4	false

#define	STIM_SEL_ENCL_STIMA		true
#define	STIM_SEL_ENCL_SINK_ENCL	false

typedef enum
{
	BEFORE_HIGH = 0U,
	TO_HIGH,
	TO_LOW,
} PWM_InterruptState;

typedef enum
{
	AMP = 0U,
	POLR_POS,
	POLR_NEG,
} SINE_InterruptState;

typedef struct {
	uint32_t 	pulseWidth_us;				/*!< The pulse width of the stimulus waveform, unit: us */
	uint32_t 	pulsePeriod_us;				/*!< The pulse period of the stimulus waveform, unit: us */
	uint32_t 	trainOnDuration_ms;			/*!< The train on duration of the stimulus waveform, unit: ms */
	uint32_t 	trainOffDuration_ms;		/*!< The train off duration of the stimulus waveform, unit: ms */
} Stimulus_Waveform_t;

typedef struct {
	uint32_t	sinePeriod_us;				/*!< The period of the sine waveform, unit: us */
	uint32_t	sinePhaseShift_us;			/*!< The phase shift of the sine waveform, unit: us */
	uint16_t	amplitude_mV;				/*!< The amplitude of the sine wave, unit: mV */
	uint32_t 	trainOnDuration_ms;			/*!< The train on duration of the sine waveform, unit: ms */
	uint32_t 	trainOffDuration_ms;		/*!< The train off duration of the sine waveform, unit: ms */
} NerveBlock_Waveform_t;

typedef struct {
	uint32_t 	pulseWidth_us;				/*!< The pulse width of the VNSb waveform, unit: us */
	uint32_t 	pulseFrequency_Hz;			/*!< The pulse frequency of the VNSb waveform, unit: Hz */
	uint32_t	sineFrequency_Hz;			/*!< The sine frequency of the VNSb waveform, unit: Hz */
	uint16_t	sineAmplitude_mV;			/*!< The amplitude of the sine wave, unit: mV */
	uint32_t 	trainOnDuration_ms;			/*!< The train on duration of the VNSb waveform, unit: ms */
	uint32_t 	trainOffDuration_ms;		/*!< The train off duration of the VNSb waveform, unit: ms */
} VNSb_Waveform_t;

typedef struct {
	bool src1;								/*!< Default state of source pin 1 */
	bool src2;								/*!< Default state of source pin 2 */
	bool snk1;								/*!< Default state of sink pin 1 */
	bool snk2;								/*!< Default state of sink pin 2 */
	bool snk3;								/*!< Default state of sink pin 3 */
	bool snk4;								/*!< Default state of sink pin 4 */
	bool snk5;								/*!< Default state of sink pin 5 */
} Current_Sources_t;

typedef struct {
	bool encl;								/*!< Stimulus multiplexer channel "ENCL" settings. */
	bool ch1;								/*!< Stimulus multiplexer channel "CH1" settings. */
	bool ch2;								/*!< Stimulus multiplexer channel "CH2" settings. */
	bool ch3;								/*!< Stimulus multiplexer channel "CH3" settings. */
	bool ch4;								/*!< Stimulus multiplexer channel "CH4" settings. */
} Stim_Sel_Ch_t;

typedef struct {
	bool stimA;								/*!< Stimulus multiplexer channel "STIMA" settings. */
	bool stimB;								/*!< Stimulus multiplexer channel "STIMB" settings. */

	Stim_Sel_Ch_t	sel_ch;					/*!< Stimulus multiplexer channel "CH1~CH4, ENCL" settings. */
} Stim_Sel_t;

typedef struct {
	uint32_t 	rampUpDuration_us;				/*!< The duration of the ramp up, unit: us */
	uint32_t 	rampDownDuration_us;			/*!< The duration of the ramp down, unit: us */
	uint32_t 	rampUpStart_us;					/*!< Time to start ramp up, unit: us */
	uint32_t 	rampUpEnd_us;					/*!< Time to end ramp up, unit: us */
	uint32_t 	rampDownStart_us;				/*!< Time to start ramp down, unit: us */
	uint32_t 	rampDownEnd_us;					/*!< Time to end ramp down, unit: us */

	uint32_t 	period_us;						/*!< The period of the ramp, unit: us */
	uint32_t 	timer_us;						/*!< The timer of the ramp, unit: us */
	uint16_t	max_amplitude_mV;				/*!< The max amplitude of the ramp, unit: mV */
	uint16_t 	curr_amplitude_mV;				/*!< The current amplitude of the ramp, unit: mV */
} Ramp_t;

typedef struct {
	uint32_t 	pwm_pulse_width_us;				/*!< The pulse width of the PWM, unit: us */
	uint32_t 	pwm_period_us;					/*!< The period of the PWM, unit: us */

	uint32_t 	train_period_us;				/*!< The period of the train signal, unit: us */
	uint32_t 	train_on_duration_us;			/*!< Duration of train-on time, unit: us */
	uint32_t 	train_timer_us;					/*!< The timer of the train signal, unit: us */

	bool		is_positive;					/*!< The direction of the waveform pulse is positive */
	bool		is_running;						/*!< The waveform is running */

	Stim_Sel_Ch_t sel_positive;					/*!< The multiplexer positive settings */
	Stim_Sel_Ch_t sel_negative;					/*!< The multiplexer negative settings */
	Stim_Sel_Ch_t sel_discharge;				/*!< The multiplexer discharge settings */
	Stim_Sel_Ch_t sel_enabled;					/*!< The multiplexer enabled CH */

	bool		pause_output;					/*!< Pause the signal output. */

	bool		imc_is_enabled;					/*!< The IMC is enabled. */

	Ramp_t 		ramp;							/*!< Ramp up and down settings. */
} PulseWave_t;

typedef struct {
	uint32_t tim_cnt;							/*!< The timer counts of the point on the sine period */
	uint16_t dac_cnt;							/*!< DAC counts of the point on the sine period */
} Sine_Point_t;

typedef struct {
	uint32_t 	period_us;						/*!< The period of the sine wave, unit: us */
	uint32_t	phaseShift_us;					/*!< The phase shift of the sine waveform, unit: us */
	uint32_t 	update_interval_us;				/*!< The interval between DAC updates, unit: us */
	uint16_t	amplitude_mV;					/*!< The amplitude of the sine wave, unit: mV */

	uint32_t 	train_period_us;				/*!< The period of the train signal, unit: us */
	uint32_t 	train_on_duration_us;			/*!< Duration of train-on time, unit: us */
	uint32_t 	train_timer_us;					/*!< The timer of the train signal, unit: us */

	bool		is_running;						/*!< The waveform is running */

	Stim_Sel_Ch_t sel_positive;					/*!< The multiplexer positive settings */
	Stim_Sel_Ch_t sel_negative;					/*!< The multiplexer negative settings */
	Stim_Sel_Ch_t sel_discharge;				/*!< The multiplexer discharge settings */
	Stim_Sel_Ch_t sel_enabled;					/*!< The multiplexer enabled CH */

	bool		pause_output;					/*!< Pause the signal output. */

	Sine_Point_t sine_points[SINE_PERIOD_POINTS];	/*!< The points on the sine period */
	uint16_t sine_point_idx;						/*!< Current index of the point on the sine period */
} SineWave_t;

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
 * @brief Initializes the DAC settings for stimulus generation.
 *
 * @return uint8_t HAL status
 */
uint8_t app_func_stim_dac_init(void);

/**
 * @brief Set the voltage of DAC
 * 
 * @param voltageA_mv The voltage of VOUTA, unit: mV
 * @param voltageB_mv The voltage of VOUTB, unit: mV
 * @return uint8_t HAL status
 */
uint8_t app_func_stim_dac_volt_set(uint16_t voltageA_mv, uint16_t voltageB_mv);

/**
 * @brief Set the ramp settings of DAC1
 *
 * @param ramp_up_duration_ms The duration of the ramp up, unit: ms
 * @param ramp_down_duration_ms The duration of the ramp down, unit: ms
 * @param voltage_mv The max voltage of VOUTA, unit: mV
 */
void app_func_stim_dac1_ramp_set(uint32_t ramp_up_duration_ms, uint32_t ramp_down_duration_ms, uint16_t voltage_mv);

/**
 * @brief   Calculate the required VDAC voltage to achieve a target output current
 *          in the current mirror circuit.
 *
 * @param iout_mA  Desired output current in mA
 * @return _Float64 The corresponding VDAC voltage in mV required to produce iout_mA
 */
_Float64 app_func_stim_iout_to_dac(_Float64 iout_mA);

/**
 * @brief Enable / disable VNSb stimulation output
 *
 * @param enable Enable / disable
 */
void app_func_stim_vnsb_enable(bool enable);

/**
 * @brief Enable / disable stimulus output
 * 
 * @param enable Enable / disable
 */
void app_func_stim_stimulus_enable(bool enable);

/**
 * @brief Enable and set the multiplexer of the stimulus output
 *
 * @param enable Enable / disable
 */
void app_func_stim_mux_enable(bool enable);

/**
 * @brief Select the STIM_SEL channels of the output multiplexer.
 *
 * @param sel U1500 & U1501 Select Settings
 */
void app_func_stim_sel_set(Stim_Sel_t sel);

/**
 * @brief Set current source configuration
 *
 * @param current_sources The current source configuration
 */
void app_func_stim_curr_src_set(Current_Sources_t current_sources);

/**
 * @brief Set the waveform settings of stim1
 *
 * @param stimulus_waveform The waveform settings
 */
void app_func_stim_circuit_para1_set(Stimulus_Waveform_t stimulus_waveform);

/**
 * @brief Set the waveform settings of stim2
 *
 * @param stimulus_waveform The waveform settings
 */
void app_func_stim_circuit_para2_set(Stimulus_Waveform_t stimulus_waveform);

/**
 * @brief Generate stim1 waveforms based on waveform settings and current source settings
 *
 * @param imc_en The enabled status of IMC
 */
void app_func_stim_stim1_start(bool imc_en);

/**
 * @brief Generate stim2 waveforms based on waveform settings and current source settings
 *
 */
void app_func_stim_stim2_start(void);

/**
 * @brief Stop stim1 waveform
 * 
 */
void app_func_stim_stim1_stop(void);

/**
 * @brief Stop stim2 waveform
 *
 */
void app_func_stim_stim2_stop(void);

/**
 * @brief Timer callback of stim1 waveform
 *
 * @param state Callback state
 */
void app_func_stim_stim1_cb(PWM_InterruptState state);

/**
 * @brief Timer callback of stim2 waveform
 *
 * @param state Callback state
 */
void app_func_stim_stim2_cb(PWM_InterruptState state);

/**
 * @brief Set the waveform settings of sine
 *
 * @param nerveBlock_waveform The waveform settings
 */
void app_func_stim_sine_para_set(NerveBlock_Waveform_t nerveBlock_waveform);

/**
 * @brief Generate sine waveform based on the waveform settings
 *
 */
void app_func_stim_sine_start(void);

/**
 * @brief Stop sine wave
 *
 */
void app_func_stim_sine_stop(void);

/**
 * @brief Timer callback of the sine wave
 *
 * @param state Callback state
 */
void app_func_stim_sine_cb(SINE_InterruptState state);

/**
 * @brief Synchronizes the timers of all waveforms.
 *
 */
void app_func_stim_sync(void);
/**
 * @brief Turn off all peripheral circuits of stimulation
 * 
 */
void app_func_stim_off(void);

#endif /* FUNCTIONS_INC_APP_FUNC_STIMULATION_H_ */
