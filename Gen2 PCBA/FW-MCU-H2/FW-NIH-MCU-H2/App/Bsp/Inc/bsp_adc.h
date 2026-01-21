/**
 * @file bsp_adc.h
 * @brief This file contains all the function prototypes for the bsp_adc.c file
 * @copyright Copyright (c) 2024
 */
#ifndef BSP_INC_BSP_ADC_H_
#define BSP_INC_BSP_ADC_H_
#include <stdint.h>
#include <stdbool.h>

#define ADC1_CHANNEL_BATT_MON1			ADC_CHANNEL_5	//PA0
#define ADC1_CHANNEL_BATT_MON2			ADC_CHANNEL_6	//PA1
#define ADC1_CHANNEL_ECG_HR_OUT			ADC_CHANNEL_15	//PB0
#define ADC1_CHANNEL_ECG_RR_OUT			ADC_CHANNEL_16	//PB1
#define ADC1_CHANNEL_ENG1_OUT			ADC_CHANNEL_17	//PB2
#define ADC1_CHANNEL_DVDD 				ADC_CHANNEL_VBAT//VBAT

#define ADC4_CHANNEL_THERM_OFST			ADC_CHANNEL_1	//PC0
#define ADC4_CHANNEL_THERM_OUT			ADC_CHANNEL_2	//PC1
#define ADC4_CHANNEL_THERM_REF			ADC_CHANNEL_3	//PC2
#define ADC4_CHANNEL_IMP_INA			ADC_CHANNEL_5	//PF14
#define ADC4_CHANNEL_IMP_INB			ADC_CHANNEL_6	//PF15
#define ADC4_CHANNEL_ENG2_OUT			ADC_CHANNEL_7	//PG0
#define ADC4_CHANNEL_VRECT_MON			ADC_CHANNEL_22	//PC4

#define HANDLE_ID_ADC1					0
#define HANDLE_ID_ADC4					1

#define ADC_MAX_SAMPLE_POINTS			2000

/**
 * @brief Initialization of the ADC
 *
 */
void bsp_adc_init(void);

/**
 * @brief Start the ADC single end sampling
 *
 * @param hadcID ADC handle ID
 * @param channel The channel to sample
 * @param voltageBuffer Buffer to store sample voltage
 * @param samplingPoints Number of the sampling points
 * @param samplingFrequency_hz Sampling frequency of the ADC
 */
void bsp_adc_single_sampling(uint8_t hadcID, uint32_t channel, uint16_t voltageBuffer[], uint16_t samplingPoints, uint16_t samplingFrequency_hz);

/**
 * @brief Continue the ADC sampling
 *
 * @param cycles Number of the sampling cycles
 */
void bsp_adc_sampling_continue_IT(uint8_t cycles);

/**
 * @brief Convert the ADC sample value
 *
 * @param multiple The multiple of the number of sampling points to the number of output points
 */
void bsp_adc_sampling_conversion(uint8_t multiple);

/**
 * @brief Confirm that ADC sampling is complete
 *
 * @return true ADC sampling is completed
 * @return false ADC sampling is not completed
 */
bool bsp_adc_sampling_is_completed(void);

#endif /* BSP_INC_BSP_ADC_H_ */
