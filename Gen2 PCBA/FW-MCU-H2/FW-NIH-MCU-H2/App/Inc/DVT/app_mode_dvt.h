/**
 * @file app_mode_dvt.h
 * @brief This file contains all the function prototypes for the app_mode_dvt.c file
 * @copyright Copyright (c) 2024
 */
#ifndef INC_APP_APP_MODE_DVT_H_
#define INC_APP_APP_MODE_DVT_H_
#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint8_t hvSupplyEnable;
	uint8_t vddsSupplyEnable;
	uint8_t vddaSupplyEnable;
	uint8_t reserved[5];
} TestInformation_t;

typedef struct {
	uint16_t id;
} SampleId_t;

typedef struct {
	uint8_t mode;
} Mode_t;

typedef struct {
	uint16_t voltage_mv;
} HvSupplyVoltageValue_t;

typedef struct {
	uint16_t batteryAvoltage_mv;
	uint16_t batteryBvoltage_mv;
} BatteryVoltageMeasurement_t;

typedef struct {
	uint16_t pulseWidth1_us;
	uint8_t stimDuration1_ms;
	uint16_t pulsePeriod1_us;
	uint16_t pulseWidth2_us;
	uint8_t stimDuration2_ms;
	uint16_t pulsePeriod2_us;
	uint8_t stimFreqVNS_hz;
	uint16_t stimDurationVNS_ms;
} StimulusCircuitParameters_t;

typedef struct {
	uint16_t aDacOutputVoltage_mv;
	uint16_t bDacOutputVoltage_mv;
} DacAbOutputVoltage_t;

typedef struct {
	bool src1;
	bool src2;
	bool snk1;
	bool snk2;
	bool snk3;
	bool snk4;
	bool snk5;
} CurrentSourcesConfiguration_t;

typedef struct {
	uint8_t stima_sel;
	uint8_t stimb_sel;
	uint8_t stim_sel_encl;
	uint8_t stim_sel_ch1;
	uint8_t stim_sel_ch2;
	uint8_t stim_sel_ch3;
	uint8_t stim_sel_ch4;
} StimSelPositions_t;

typedef struct {
	uint8_t ecgHrLod;
} EcgHrLod_t;

typedef struct {
	uint8_t ecgRrLod;
} EcgRrLod_t;

typedef struct {
	uint8_t bufferSize;
	uint16_t samplingFrequency_hz;
} EcgHrAfeOutputReq_t;

typedef struct {
	uint8_t bufferSize;
	uint8_t buffer[238];
} EcgHrAfeOutputResp_t;

typedef struct {
	uint8_t bufferSize;
	uint16_t samplingFrequency_hz;
} EcgRrAfeOutputReq_t;

typedef struct {
	uint8_t bufferSize;
	uint8_t buffer[238];
} EcgRrAfeOutputResp_t;

typedef struct {
	uint8_t eng1Lod;
} Eng1Lod_t;

typedef struct {
	uint8_t eng2Lod;
} Eng2Lod_t;

typedef struct {
	uint8_t bufferSize;
	uint16_t samplingFrequency_hz;
} Eng1AfeOutputReq_t;

typedef struct {
	uint8_t bufferSize;
	uint8_t buffer[238];
} Eng1AfeOutputResp_t;

typedef struct {
	uint8_t bufferSize;
	uint16_t samplingFrequency_hz;
} Eng2AfeOutputReq_t;

typedef struct {
	uint8_t bufferSize;
	uint8_t buffer[238];
} Eng2AfeOutputResp_t;

typedef struct {
	uint8_t bufferSize;
	uint16_t samplingFrequency_hz;
} WptVrectOutputReq_t;

typedef struct {
	uint8_t bufferSize;
	uint8_t buffer[238];
} WptVrectOutputResp_t;

typedef struct {
	uint8_t det;
} VrectDet_t;

typedef struct {
	uint8_t ovp;
} VrectOvp_t;

typedef struct {
	uint8_t pGood;
} VchgRailSupplyPowerGood_t;

typedef struct {
	uint8_t chgRate1;
	uint8_t chgRate2;
} ChargeRateControl_t;

typedef struct {
	uint8_t status;
} Chg1Status_t;

typedef struct {
	uint8_t ovpErr;
} Chg1OvpErr_t;

typedef struct {
	uint8_t status;
} Chg2Status_t;

typedef struct {
	uint8_t ovpErr;
} Chg2OvpErr_t;

typedef struct {
	uint8_t bufferSize;
	uint16_t samplingFrequency_hz;
} ThermRefReq_t;

typedef struct {
	uint8_t bufferSize;
	uint8_t buffer[238];
} ThermRefResp_t;

typedef struct {
	uint8_t bufferSize;
	uint16_t samplingFrequency_hz;
} ThermOutReq_t;

typedef struct {
	uint8_t bufferSize;
	uint8_t buffer[238];
} ThermOutResp_t;

typedef struct {
	uint8_t bufferSize;
	uint16_t samplingFrequency_hz;
} ThermOffstReq_t;

typedef struct {
	uint8_t bufferSize;
	uint8_t buffer[238];
} ThermOffstResp_t;

typedef struct {
	bool status;
} MagStatus_t;

typedef struct {
	bool imp_in_n_sel0;
	bool imp_in_n_sel1;
	bool imp_in_n_sel2;
	bool imp_in_p_sel0;
} ImpSelPositions_t;

typedef struct {
	uint16_t imc_measure;
} ImcMeasure_t;

typedef struct {
	uint8_t FreqSampling;
	uint8_t ModeDeviceID;
} FreqModeDeviceID_t;

typedef struct {
	uint16_t X;
	uint16_t Y;
	uint16_t Z;
} Axis_t;

typedef struct {
	uint8_t DeviceID;
	Axis_t	XYZ;
} AccMeasure_t;

typedef struct {
	uint8_t DeviceID;
} DeviceID_t;

typedef struct {
	uint8_t DeviceID;
	uint32_t Timestamp;
	uint16_t SampleIndex;
	Axis_t XYZ[32];
} InfoData_t;

/**
 * @brief Handler for DVT mode
 *
 */
void app_mode_dvt_handler(void);

/**
 * @brief Start the MIS2DHTR sensor and begin retrieving XYZ acceleration data.
 *
 * @param freqModedeviceID 	The sampling frequency, response mode, and I2C device address of the MIS2DHTR sensor.
 *
 * @param dataBuffer  		Pointer to the buffer where the retrieved XYZ acceleration
 *                    		data will be stored.
 *
 * @param dataSize    		Pointer to a variable that will receive the actual number
 *                    		of bytes written into @p dataBuffer.
 *
 * @return uint8_t    		Status code indicating the result of the operation.
 *                    		Typically returns 0 on success; non-zero values indicate errors.
 */
uint8_t app_mode_dvt_acc_start(FreqModeDeviceID_t freqModedeviceID, uint8_t* dataBuffer, uint8_t* dataSize);

/**
 * @brief Get XYZ acceleration data from the MIS2DHTR sensor.
 *
 * @param deviceID		The device ID of the MIS2DHTR sensor.
 *
 * @param dataBuffer	Pointer to the buffer where the retrieved XYZ acceleration
 *                    	data will be stored.
 *
 * @param dataSize    	Pointer to a variable that will receive the actual number
 *                    	of bytes written into @p dataBuffer.
 *
 * @return uint8_t    	Status code indicating the result of the operation.
 *                    	Typically returns 0 on success; non-zero values indicate errors.
 */
uint8_t app_mode_dvt_acc_data_get(DeviceID_t deviceID, uint8_t* dataBuffer, uint8_t* dataSize);

/**
 * @brief Stop the MIS2DHTR sensor.
 *
 * @param deviceID		The device ID of the MIS2DHTR sensor.
 *
 * @param dataBuffer	Pointer to the buffer where the retrieved XYZ acceleration
 *                    	data will be stored.
 *
 * @param dataSize    	Pointer to a variable that will receive the actual number
 *                    	of bytes written into @p dataBuffer.
 *
 * @return uint8_t    	Status code indicating the result of the operation.
 *                    	Typically returns 0 on success; non-zero values indicate errors.
 */
uint8_t app_mode_dvt_acc_stop(DeviceID_t deviceID, uint8_t* dataBuffer, uint8_t* dataSize);

/**
 * @brief Callback for the accelerometer timer, unit: ms
 *
 */
void app_mode_dvt_acc_timer_cb(void);

#endif /* INC_APP_APP_MODE_DVT_H_ */
