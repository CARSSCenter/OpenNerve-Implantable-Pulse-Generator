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
	uint16_t pulseWidth_us;
	uint8_t pulseNumber_ms;
	uint16_t pulsePeriod_us;
	uint8_t reserved[3];
} StimulusCircuitParameters_t;

typedef struct {
	uint16_t aDacOutputVoltage_mv;
	uint16_t bDacOutputVoltage_mv;
} DacAbOutputVoltage_t;

typedef struct {
	bool src1;
	bool sink1;
	bool src2;
	bool sink2;
	bool src3;
	bool sink3;
	bool src4;
	bool sink4;
	bool src5;
	bool sink5;
	bool src6;
	bool sink6;
	bool src7;
	bool sink7;
	bool src8;
	bool sink8;
	bool src17;
	bool sink17;
} CurrentSourcesConfiguration_t;

typedef struct {
	uint8_t sr_sel1;
	uint8_t sr_sel2;
	uint8_t ecg_sel;
} SrSelPositions_t;

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
	uint8_t emg1Lod;
} Emg1Lod_t;

typedef struct {
	uint8_t emg2Lod;
} Emg2Lod_t;

typedef struct {
	uint8_t bufferSize;
	uint16_t samplingFrequency_hz;
} Emg1AfeOutputReq_t;

typedef struct {
	uint8_t bufferSize;
	uint8_t buffer[238];
} Emg1AfeOutputResp_t;

typedef struct {
	uint8_t bufferSize;
	uint16_t samplingFrequency_hz;
} Emg2AfeOutputReq_t;

typedef struct {
	uint8_t bufferSize;
	uint8_t buffer[238];
} Emg2AfeOutputResp_t;

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
} ThermOfstReq_t;

typedef struct {
	uint8_t bufferSize;
	uint8_t buffer[238];
} ThermOfstResp_t;

typedef struct {
	bool status;
} MagStatus_t;

typedef struct {
	bool impa_sel0;
	bool impa_sel1;
	bool impa_sel2;
	bool impa_sel3;
	bool impb_sel0;
	bool impb_sel1;
	bool impb_sel2;
	bool impb_sel3;
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
 * @brief Callback for the timer in DVT mode, unit: ms
 *
 */
void app_mode_dvt_ms_timer_cb(void);

#endif /* INC_APP_APP_MODE_DVT_H_ */
