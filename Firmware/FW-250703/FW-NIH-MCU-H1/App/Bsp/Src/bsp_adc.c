/**
 * @file bsp_adc.c
 * @brief This file provides the functionality of the ADC
 * @copyright Copyright (c) 2024
 */
#include "bsp_adc.h"
#include "bsp_config.h"

static uint32_t RankADC1[] = {
		ADC_REGULAR_RANK_1,
		ADC_REGULAR_RANK_2,
		ADC_REGULAR_RANK_3,
		ADC_REGULAR_RANK_4,
		ADC_REGULAR_RANK_5,
		ADC_REGULAR_RANK_6,
		ADC_REGULAR_RANK_7,
		ADC_REGULAR_RANK_8,
		ADC_REGULAR_RANK_9,
		ADC_REGULAR_RANK_10,
		ADC_REGULAR_RANK_11,
		ADC_REGULAR_RANK_12,
		ADC_REGULAR_RANK_13,
		ADC_REGULAR_RANK_14,
		ADC_REGULAR_RANK_15,
		ADC_REGULAR_RANK_16,
};

static uint32_t samplingBuffer[ADC_MAX_SAMPLE_POINTS];
static uint32_t vchA[ADC_MAX_SAMPLE_POINTS / 2];
static uint32_t vchB[ADC_MAX_SAMPLE_POINTS / 2];

static ADC_HandleTypeDef* p_hadc[] = {
		&hadc1,
		&hadc4,
};

typedef struct
{
	ADC_HandleTypeDef* 		hadc;
	TIM_HandleTypeDef*		htim;
	uint32_t* 				samplingBuffer;
	uint16_t 				samplingPoints;
	uint16_t 				samplingFrequency_hz;
	uint8_t					samplingCycles;
	bool					isCompleted;
	uint16_t* 				voltageBuffer;
	uint32_t				vrefanalog_mv;
} ADC_Sampling_t;
ADC_Sampling_t	sampling;

/**
 * @brief Configure the sample rate of the ADC
 *
 * @param p_tim 	The trigger timer of the ADC
 * @param samplingFrequency_hz 	The sampling frequency of the ADC
 */
static void bsp_adc_sample_rate_config(TIM_HandleTypeDef* p_tim, uint16_t samplingFrequency_hz)
{
	uint32_t cloksrc = HAL_RCC_GetSysClockFreq();
	uint32_t psc = p_tim->Instance->PSC + 1;
	uint32_t Period = cloksrc / psc / samplingFrequency_hz;
	__HAL_TIM_SET_AUTORELOAD(p_tim, Period-1);
}

/**
 * @brief Deinitialize the ADC
 *
 * @param hadc ADC handle
 */
static void bsp_adc_deinit(ADC_HandleTypeDef *hadc)
{
	HAL_ADC_Stop_DMA(hadc);
	HAL_ADC_DeInit(hadc);
	hadc->Init.NbrOfConversion = 0;
}

/**
 * @brief Add a channel to ADC
 *
 * @param hadc ADC handle
 * @param channel Specify the channel to configure into ADC regular group
 */
static ADC_ChannelConfTypeDef bsp_adc_channel_add(ADC_HandleTypeDef *hadc, uint32_t channel)
{
	ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Channel = channel;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset = 0;

	if (hadc == &hadc1)
	{
		sConfig.SamplingTime = ADC_SAMPLETIME_5CYCLES;
		sConfig.SingleDiff = ADC_SINGLE_ENDED;
		sConfig.Rank = RankADC1[hadc1.Init.NbrOfConversion];
	}
	else if (hadc == &hadc4)
	{
		sConfig.Rank = ADC4_RANK_CHANNEL_NUMBER;
		sConfig.SamplingTime = ADC4_SAMPLINGTIME_COMMON_1;
	}
	hadc->Init.NbrOfConversion++;
	return sConfig;
}

/**
 * @brief Reinitialize the ADC
 *
 * @param hadc ADC handle
 * @param config Configure the channels to the ADC group regular
 * @param channels Number of the channels
 */
static void bsp_adc_reinit(ADC_HandleTypeDef *hadc, ADC_ChannelConfTypeDef config[], uint8_t channels)
{
	HAL_ADC_Init(hadc);
	for(uint8_t i=0;i<channels;i++) {
		HAL_ADC_ConfigChannel(hadc, &config[i]);
	}
}

/**
 * @brief Start the ADC sampling
 *
 * @param hadc ADC handle
 * @param samplingBuffer Data buffer of the sampling data
 * @param samplingPoints Number of the sampling points
 * @param samplingFrequency_hz Sampling frequency of the ADC
 */
static void bsp_adc_sampling(ADC_HandleTypeDef *hadc, uint32_t samplingBuffer[], uint16_t samplingPoints, uint16_t samplingFrequency_hz)
{
	sampling.hadc = hadc;
	sampling.samplingBuffer = (uint32_t*)samplingBuffer;
	sampling.samplingPoints = samplingPoints;
	sampling.samplingFrequency_hz = samplingFrequency_hz;
	sampling.samplingCycles = 1U;
	sampling.isCompleted = false;

	HAL_ADC_Start_DMA(sampling.hadc, sampling.samplingBuffer, sampling.samplingPoints * hadc->Init.NbrOfConversion);
	if (sampling.hadc == &hadc1) {
		sampling.htim = &HANDLE_ADC1_SAMPLE_TIM;
	}
	else if (sampling.hadc == &hadc4) {
		sampling.htim = &HANDLE_ADC4_SAMPLE_TIM;
	}
	bsp_adc_sample_rate_config(sampling.htim, sampling.samplingFrequency_hz);
	HAL_TIM_Base_Start_IT(sampling.htim);
	while(HAL_TIM_Base_GetState(sampling.htim) == HAL_TIM_STATE_BUSY);
	while(!sampling.isCompleted);
	sampling.isCompleted = false;
}

/**
 * @brief Continue the ADC sampling
 *
 * @param cycles Number of the sampling cycles
 */
void bsp_adc_sampling_continue_IT(uint8_t cycles)
{
	sampling.samplingCycles = cycles;
	sampling.isCompleted = false;

	HAL_ADC_Start_DMA(sampling.hadc, sampling.samplingBuffer, sampling.samplingPoints * sampling.samplingCycles * sampling.hadc->Init.NbrOfConversion);
	HAL_TIM_Base_Start_IT(sampling.htim);
}

/**
 * @brief Convert the ADC sample value
 *
 * @param multiple The multiple of the number of sampling points to the number of output points
 */
void bsp_adc_sampling_conversion(uint8_t multiple)
{
	for (uint8_t i = 0; i < sampling.samplingPoints; i++) {
		sampling.voltageBuffer[i] = __HAL_ADC_CALC_DATA_TO_VOLTAGE(sampling.hadc->Instance, sampling.vrefanalog_mv, sampling.samplingBuffer[i * multiple], ADC_RESOLUTION_12B);
	}
}

/**
 * @brief Confirm that ADC sampling is complete
 *
 * @return true ADC sampling is completed
 * @return false ADC sampling is not completed
 */
bool bsp_adc_sampling_is_completed(void)
{
	return sampling.isCompleted;
}

/**
 * @brief Initialization of the ADC
 *
 */
void bsp_adc_init(void)
{
	HAL_ERROR_CHECK(HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED));
	HAL_ERROR_CHECK(HAL_ADCEx_Calibration_Start(&hadc4, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED));
}

/**
 * @brief Get the ADC Vref in mV
 *
 * @param VoltageScaling specifies the output voltage to achieve
 * @return uint32_t Vref in mV
 */
uint32_t bsp_adc_vref_get(uint32_t VoltageScaling)
{
	switch (VoltageScaling) {
	case SYSCFG_VREFBUF_VOLTAGE_SCALE0:
		return 1500;

	case SYSCFG_VREFBUF_VOLTAGE_SCALE1:
		return 1800;

	case SYSCFG_VREFBUF_VOLTAGE_SCALE2:
		return 2048;

	case SYSCFG_VREFBUF_VOLTAGE_SCALE3:
		return 2500;
	}
	return 0;
}

/**
 * @brief Start the ADC single end sampling
 *
 * @param hadcID ADC handle ID
 * @param channel The channel to sample
 * @param voltageBuffer Buffer to store sample voltage
 * @param samplingPoints Number of the sampling points
 * @param samplingFrequency_hz Sampling frequency of the ADC
 * @param vrefanalog_mv Vref of the ADC in mV
 */
__weak void bsp_adc_single_sampling(uint8_t hadcID, uint32_t channel, uint16_t voltageBuffer[], uint16_t samplingPoints, uint16_t samplingFrequency_hz, uint32_t vrefanalog_mv)
{
	ADC_HandleTypeDef* hadc = p_hadc[hadcID];
	bsp_adc_deinit(hadc);
	ADC_ChannelConfTypeDef sConfig = bsp_adc_channel_add(hadc, channel);
	bsp_adc_reinit(hadc, &sConfig, 1);
	bsp_adc_sampling(hadc, samplingBuffer, samplingPoints, samplingFrequency_hz);

	for (uint8_t i = 0; i < samplingPoints; i++) {
		voltageBuffer[i] = __HAL_ADC_CALC_DATA_TO_VOLTAGE(hadc->Instance, vrefanalog_mv, samplingBuffer[i], ADC_RESOLUTION_12B);
	}
	sampling.voltageBuffer = (uint16_t*)voltageBuffer;
	sampling.vrefanalog_mv = vrefanalog_mv;
}

/**
 * @brief Start the ADC differential sampling
 *
 * @param hadcID ADC handle ID
 * @param ch1 The first sample channel
 * @param ch2 The second sample channel
 * @param voltageBuffer Buffer to store sample voltage
 * @param samplingPoints Number of the sampling points
 * @param samplingFrequency_hz Sampling frequency of the ADC
 * @param vrefanalog_mv Vref of the ADC in mV
 */
__weak void bsp_adc_differential_sampling(uint8_t hadcID, uint32_t ch1, uint32_t ch2, uint16_t voltageBuffer[], uint16_t samplingPoints, uint16_t samplingFrequency_hz, uint32_t vrefanalog_mv)
{
	ADC_HandleTypeDef* hadc = p_hadc[hadcID];
	bsp_adc_deinit(hadc);
	ADC_ChannelConfTypeDef sConfig[2];
	sConfig[0] = bsp_adc_channel_add(hadc, ch1);
	sConfig[1] = bsp_adc_channel_add(hadc, ch2);
	bsp_adc_reinit(hadc, sConfig, 2);
	bsp_adc_sampling(hadc, samplingBuffer, samplingPoints*2, samplingFrequency_hz);

	for (uint16_t i = 0; i < samplingPoints; i++) {
		uint32_t s1 = samplingBuffer[i*2];
		uint32_t s2 = samplingBuffer[i*2+1];
		vchA[i] = __HAL_ADC_CALC_DATA_TO_VOLTAGE(hadc->Instance, vrefanalog_mv, s1, ADC_RESOLUTION_12B);
		vchB[i] = __HAL_ADC_CALC_DATA_TO_VOLTAGE(hadc->Instance, vrefanalog_mv, s2, ADC_RESOLUTION_12B);

		uint32_t diff = (s1 > s2) ? (s1 - s2) : (s2 - s1);
		voltageBuffer[i] = __HAL_ADC_CALC_DATA_TO_VOLTAGE(hadc->Instance, vrefanalog_mv, diff, ADC_RESOLUTION_12B);
	}
	sampling.voltageBuffer = (uint16_t*)voltageBuffer;
	sampling.vrefanalog_mv = vrefanalog_mv;
}

/**
  * @brief  Conversion complete callback in non-blocking mode.
  * @param hadc ADC handle
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	sampling.isCompleted = true;
	if (hadc == &hadc1) {
		HAL_TIM_Base_Stop(&HANDLE_ADC1_SAMPLE_TIM);
	}
	else if (hadc == &hadc4) {
		HAL_TIM_Base_Stop(&HANDLE_ADC4_SAMPLE_TIM);
	}
}
