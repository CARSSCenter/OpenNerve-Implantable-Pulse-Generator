/**
 * @file app_mode_dvt.c
 * @brief This file provides management of the DVT mode
 * @copyright Copyright (c) 2024
 */
#include "app_mode_dvt.h"
#include "app_config.h"
#ifdef DVT_TEST
#include "app_mode_dvt_test.h"
#endif

static uint8_t hvSupplyTurnOn = 0;
static bool shutdown = false;
static bool sw_reset = false;

typedef struct {
	bool src1;
	bool src2;
	bool vnsb_en;
	bool imp_en;
} Mocked_t;
static Mocked_t mocked = {
		.src1 = false,
		.src2 = false,
		.vnsb_en = false,
		.imp_en = false,
};

static TestInformation_t TestInformation = {
		.hvSupplyEnable = 0,
		.vddsSupplyEnable = 0,
		.vddaSupplyEnable = 0,
		.reserved = {0,0,0,0,0},
};

static DacAbOutputVoltage_t DacAbOutputVoltage = {
		.aDacOutputVoltage_mv = 0,
		.bDacOutputVoltage_mv = 0,
};

static StimulusCircuitParameters_t StimulusCircuitParameters = {
		.pulseWidth1_us = 100,
		.stimDuration1_ms = 200,
		.pulsePeriod1_us = 1000,
		.pulseWidth2_us = 100,
		.stimDuration2_ms = 200,
		.pulsePeriod2_us = 1000,
		.stimFreqVNS_hz = 10,
		.stimDurationVNS_ms = 200,
};

static CurrentSourcesConfiguration_t CurrentSourcesConfiguration = {
		.src1 = false,
		.src2 = false,
		.snk1 = false,
		.snk2 = false,
		.snk3 = false,
		.snk4 = false,
		.snk5 = false,
};

static StimSelPositions_t StimSelPositions = {
		.stima_sel = 0,
		.stimb_sel = 0,
		.stim_sel_encl = 0,
		.stim_sel_ch1 = 0,
		.stim_sel_ch2 = 0,
		.stim_sel_ch3 = 0,
		.stim_sel_ch4 = 0,
};

static ImpSelPositions_t ImpSelPositions = {
		.imp_in_n_sel0 = false,
		.imp_in_n_sel1 = false,
		.imp_in_n_sel2 = false,
		.imp_in_p_sel0 = false,
};

#define XYZ_BUFF_SIZE	(MIS2DHTR_FIFO_LEVEL * 5)

typedef enum {
  ON_DEMAND 		= 0x00U,
  BUFFER_STORAGE	= 0x01U,
} ResponseMode_t;

typedef struct {
	MIS2DHTR_t				Device;
	XYZ_t					SampleBuffer[XYZ_BUFF_SIZE];
	uint32_t 				Timestamp[XYZ_BUFF_SIZE];
	volatile uint32_t		LastFifoTimestamp;
	volatile uint16_t		WriteIndex;
	volatile uint16_t		ReadIndex;
	uint16_t				FifoIndex;
	volatile ResponseMode_t	Mode;
	bool					Startup;
	volatile bool			Overflow;
	volatile bool			WriteSuspend;
	volatile bool			ChannelsOpen;
} ACC_t;

ACC_t acc[2] = {
		{
				.Device.DeviceAddress = MIS2DHTR_DEVICE_ADDR_L,
				.LastFifoTimestamp = 0U,
				.WriteIndex = 0U,
				.ReadIndex = 0U,
				.FifoIndex = 0U,
				.Mode = ON_DEMAND,
				.Startup = false,
				.Overflow = false,
				.WriteSuspend = false,
				.ChannelsOpen = false,
		},
		{
				.Device.DeviceAddress = MIS2DHTR_DEVICE_ADDR_H,
				.LastFifoTimestamp = 0U,
				.WriteIndex = 0U,
				.ReadIndex = 0U,
				.FifoIndex = 0U,
				.Mode = ON_DEMAND,
				.Startup = false,
				.Overflow = false,
				.WriteSuspend = false,
				.ChannelsOpen = false,
		}
};

static uint8_t* copyStructFieldToPayload(uint8_t* p_payload, uint8_t* p_field, uint16_t field_size) {
	memcpy(p_payload, p_field, field_size);
	return (p_payload + field_size);
}

static uint8_t* copyPayloadToStructField(uint8_t* p_payload, uint8_t* p_field, uint16_t field_size) {
	memcpy(p_field, p_payload, field_size);
	return (p_payload + field_size);
}

static Cmd_Resp_t app_mode_dvt_command_req_parser(Cmd_Req_t req) {
	Cmd_Resp_t resp = {
			.Opcode 		= req.Opcode,
			.Status 		= STATUS_SUCCESS,
			.Payload 		= NULL,
			.PayloadLen 	= 0,
	};
	uint8_t len_payload;

	switch(req.Opcode) {
	case OP_SET_START_STATE:
	{
		len_payload = 2;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			uint16_t state = STATE_INVALID;
			(void)memcpy((void*)&state, (void*)req.Payload, sizeof(uint16_t));
			switch(state) {
			case STATE_SLEEP:
			case STATE_ACT:
			case STATE_ACT_MODE_BLE_ACT:
			case STATE_ACT_MODE_THERAPY_SESSION:
			case STATE_ACT_MODE_IMPED_TEST:
			case STATE_ACT_MODE_BATT_TEST:
			case STATE_ACT_MODE_DVT:
			{
				Sys_Config_t sc = {
						.DefaultState = DEFAULT_STATE,
						.StartState = state,
				};
				bsp_fram_write(ADDR_SYS_CONFIG, (uint8_t*)&sc, sizeof(sc), true);
				sw_reset = true;
			}	break;

			default:
			{
				resp.Status = STATUS_INVALID;
			}
				break;
			}
		}
	}
		break;

	case OP_PING:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			//TBD
		}
	}
		break;

	case OP_GET_TEST_INFORMATION:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			uint8_t resp_payload[sizeof(TestInformation)];
			uint8_t* payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&TestInformation.hvSupplyEnable, sizeof(TestInformation.hvSupplyEnable));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&TestInformation.vddsSupplyEnable, sizeof(TestInformation.vddsSupplyEnable));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&TestInformation.vddaSupplyEnable, sizeof(TestInformation.vddaSupplyEnable));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)TestInformation.reserved, sizeof(TestInformation.reserved));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_SET_SAMPLE_ID:
	{
		len_payload = 2;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			SampleId_t sampleid;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&sampleid.id, sizeof(sampleid.id));

			app_func_para_data_set((const uint8_t*)TPID_SAMPLE_ID, (uint8_t*)&sampleid.id);
		}
	}
		break;

	case OP_GET_SAMPLE_ID:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			SampleId_t sampleid;
			app_func_para_data_get((const uint8_t*)TPID_SAMPLE_ID, (uint8_t*)&sampleid.id, (uint8_t)sizeof(sampleid.id));
			uint8_t resp_payload[sizeof(sampleid)];
			uint8_t* payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&sampleid.id, sizeof(sampleid.id));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_SET_DVT_MODE:
	{
		len_payload = 1;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			//TBD
		}
	}
		break;

	case OP_TURN_ON_HV_SUPPLY:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			hvSupplyTurnOn = 1;
			app_func_stim_hv_supply_set(hvSupplyTurnOn, TestInformation.hvSupplyEnable);
		}
	}
		break;

	case OP_SET_HV_SUPPLY_VOLTAGE_VALUE:
	{
		len_payload = 2;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			HvSupplyVoltageValue_t hvsupplyvoltagevalue;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&hvsupplyvoltagevalue.voltage_mv, sizeof(hvsupplyvoltagevalue.voltage_mv));

			if (app_func_stim_hv_sup_volt_set(hvsupplyvoltagevalue.voltage_mv) != HAL_OK)
				resp.Status = STATUS_INVALID;
		}
	}
		break;

	case OP_ENABLE_HV_SUPPLY:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			TestInformation.hvSupplyEnable = 1;
			app_func_stim_hv_supply_set(hvSupplyTurnOn, TestInformation.hvSupplyEnable);
		}
	}
		break;

	case OP_IPG_SHUTDOWN:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			shutdown = true;
		}
	}
		break;

	case OP_ENABLE_VDDS_SUPPLY:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			TestInformation.vddsSupplyEnable = 1;
			app_func_stim_vdds_sup_enable(TestInformation.vddsSupplyEnable);
		}
	}
		break;

	case OP_ENABLE_VDDA_SUPPLY:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			TestInformation.vddaSupplyEnable = 1;
			app_func_meas_vdda_sup_enable(TestInformation.vddaSupplyEnable);
		}
	}
		break;

	case OP_ENABLE_BATTERY_MONITOR:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			app_func_meas_batt_mon_enable(true);
		}
	}
		break;

	case OP_GET_BATTERY_VOLTAGE_MEASUREMENT:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			BatteryVoltageMeasurement_t batteryvoltagemeasurement;
			app_func_meas_batt_mon_meas(&batteryvoltagemeasurement.batteryAvoltage_mv, &batteryvoltagemeasurement.batteryBvoltage_mv);
			app_func_meas_batt_mon_enable(false);

			uint8_t resp_payload[sizeof(batteryvoltagemeasurement)];
			uint8_t* payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&batteryvoltagemeasurement.batteryAvoltage_mv, sizeof(batteryvoltagemeasurement.batteryAvoltage_mv));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&batteryvoltagemeasurement.batteryBvoltage_mv, sizeof(batteryvoltagemeasurement.batteryBvoltage_mv));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_SET_STIMULUS_CIRCUIT_PARAMETERS:
	{
		len_payload = 13;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			StimulusCircuitParameters_t stimuluscircuitparameters;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&stimuluscircuitparameters.pulseWidth1_us, sizeof(stimuluscircuitparameters.pulseWidth1_us));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&stimuluscircuitparameters.stimDuration1_ms, sizeof(stimuluscircuitparameters.stimDuration1_ms));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&stimuluscircuitparameters.pulsePeriod1_us, sizeof(stimuluscircuitparameters.pulsePeriod1_us));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&stimuluscircuitparameters.pulseWidth2_us, sizeof(stimuluscircuitparameters.pulseWidth2_us));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&stimuluscircuitparameters.stimDuration2_ms, sizeof(stimuluscircuitparameters.stimDuration2_ms));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&stimuluscircuitparameters.pulsePeriod2_us, sizeof(stimuluscircuitparameters.pulsePeriod2_us));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&stimuluscircuitparameters.stimFreqVNS_hz, sizeof(stimuluscircuitparameters.stimFreqVNS_hz));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&stimuluscircuitparameters.stimDurationVNS_ms, sizeof(stimuluscircuitparameters.stimDurationVNS_ms));

			memcpy(&StimulusCircuitParameters, &stimuluscircuitparameters, sizeof(StimulusCircuitParameters_t));
			Stimulus_Waveform_t para1 = {
					.pulseWidth_us 			= StimulusCircuitParameters.pulseWidth1_us,
					.pulsePeriod_us 		= StimulusCircuitParameters.pulsePeriod1_us,
					.trainOnDuration_ms 	= StimulusCircuitParameters.stimDuration1_ms,
					.trainOffDuration_ms 	= StimulusCircuitParameters.stimDuration1_ms,
			};
			Stimulus_Waveform_t para2 = {
					.pulseWidth_us 			= StimulusCircuitParameters.pulseWidth2_us,
					.pulsePeriod_us 		= StimulusCircuitParameters.pulsePeriod2_us,
					.trainOnDuration_ms 	= StimulusCircuitParameters.stimDuration2_ms,
					.trainOffDuration_ms 	= StimulusCircuitParameters.stimDuration2_ms,
			};
			NerveBlock_Waveform_t sine_para = {
					.sinePeriod_us			= (uint32_t)(1000000.0 / (_Float64)StimulusCircuitParameters.stimFreqVNS_hz),
					.sinePhaseShift_us		= 0,
					.amplitude_mV 			= DacAbOutputVoltage.bDacOutputVoltage_mv,
					.trainOnDuration_ms 	= StimulusCircuitParameters.stimDurationVNS_ms,
					.trainOffDuration_ms 	= StimulusCircuitParameters.stimDurationVNS_ms,
			};
			app_func_stim_circuit_para1_set(para1);
			app_func_stim_circuit_para2_set(para2);
			app_func_stim_sine_para_set(sine_para);
		}
	}
		break;

	case OP_GET_STIMULUS_CIRCUIT_PARAMETERS:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			uint8_t resp_payload[sizeof(StimulusCircuitParameters)];
			uint8_t* payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&StimulusCircuitParameters.pulseWidth1_us, sizeof(StimulusCircuitParameters.pulseWidth1_us));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&StimulusCircuitParameters.stimDuration1_ms, sizeof(StimulusCircuitParameters.stimDuration1_ms));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&StimulusCircuitParameters.pulsePeriod1_us, sizeof(StimulusCircuitParameters.pulsePeriod1_us));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&StimulusCircuitParameters.pulseWidth2_us, sizeof(StimulusCircuitParameters.pulseWidth2_us));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&StimulusCircuitParameters.stimDuration2_ms, sizeof(StimulusCircuitParameters.stimDuration2_ms));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&StimulusCircuitParameters.pulsePeriod2_us, sizeof(StimulusCircuitParameters.pulsePeriod2_us));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&StimulusCircuitParameters.stimFreqVNS_hz, sizeof(StimulusCircuitParameters.stimFreqVNS_hz));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&StimulusCircuitParameters.stimDurationVNS_ms, sizeof(StimulusCircuitParameters.stimDurationVNS_ms));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_SET_DAC_AB_OUTPUT_VOLTAGE:
	{
		len_payload = 4;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			DacAbOutputVoltage_t dacaboutputvoltage;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&dacaboutputvoltage.aDacOutputVoltage_mv, sizeof(dacaboutputvoltage.aDacOutputVoltage_mv));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&dacaboutputvoltage.bDacOutputVoltage_mv, sizeof(dacaboutputvoltage.bDacOutputVoltage_mv));

			memcpy(&DacAbOutputVoltage, &dacaboutputvoltage, sizeof(DacAbOutputVoltage_t));
			if (app_func_stim_dac_init() != HAL_OK) {
				resp.Status = STATUS_INVALID;
			}
			if (app_func_stim_dac_volt_set(DacAbOutputVoltage.aDacOutputVoltage_mv, DacAbOutputVoltage.bDacOutputVoltage_mv) != HAL_OK) {
				resp.Status = STATUS_INVALID;
			}
		}
	}
		break;

	case OP_ENABLE_VNS_STIMULATION:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			app_func_stim_vnsb_enable(true);
			mocked.vnsb_en = true;
		}
	}
		break;

	case OP_DISABLE_VNS_STIMULATION:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			app_func_stim_vnsb_enable(false);
			mocked.vnsb_en = false;
		}
	}
		break;

	case OP_ENABLE_STIMULUS_CIRCUIT:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			app_func_stim_stimulus_enable(true);
		}
	}
		break;

	case OP_SET_CURRENT_SOURCES_CONFIGURATION:
	{
		len_payload = 7;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			CurrentSourcesConfiguration_t currentsourcesconfiguration;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&currentsourcesconfiguration.src1, sizeof(currentsourcesconfiguration.src1));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&currentsourcesconfiguration.src2, sizeof(currentsourcesconfiguration.src2));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&currentsourcesconfiguration.snk1, sizeof(currentsourcesconfiguration.snk1));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&currentsourcesconfiguration.snk2, sizeof(currentsourcesconfiguration.snk2));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&currentsourcesconfiguration.snk3, sizeof(currentsourcesconfiguration.snk3));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&currentsourcesconfiguration.snk4, sizeof(currentsourcesconfiguration.snk4));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&currentsourcesconfiguration.snk5, sizeof(currentsourcesconfiguration.snk5));

			memcpy(&CurrentSourcesConfiguration, &currentsourcesconfiguration, sizeof(CurrentSourcesConfiguration_t));
			Current_Sources_t config = {
					.src1 	= CurrentSourcesConfiguration.src1,
					.src2 	= CurrentSourcesConfiguration.src2,
					.snk1 	= CurrentSourcesConfiguration.snk1,
					.snk2 	= CurrentSourcesConfiguration.snk2,
					.snk3 	= CurrentSourcesConfiguration.snk3,
					.snk4 	= CurrentSourcesConfiguration.snk4,
					.snk5 	= CurrentSourcesConfiguration.snk5,
			};
			app_func_stim_curr_src_set(config);
			mocked.src1 = CurrentSourcesConfiguration.src1;
			mocked.src2 = CurrentSourcesConfiguration.src2;
		}
	}
		break;

	case OP_GET_CURRENT_SOURCES_CONFIGURATION:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			uint8_t resp_payload[sizeof(CurrentSourcesConfiguration)];
			uint8_t* payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&CurrentSourcesConfiguration.src1, sizeof(CurrentSourcesConfiguration.src1));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&CurrentSourcesConfiguration.src2, sizeof(CurrentSourcesConfiguration.src2));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&CurrentSourcesConfiguration.snk1, sizeof(CurrentSourcesConfiguration.snk1));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&CurrentSourcesConfiguration.snk2, sizeof(CurrentSourcesConfiguration.snk2));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&CurrentSourcesConfiguration.snk3, sizeof(CurrentSourcesConfiguration.snk3));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&CurrentSourcesConfiguration.snk4, sizeof(CurrentSourcesConfiguration.snk4));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&CurrentSourcesConfiguration.snk5, sizeof(CurrentSourcesConfiguration.snk5));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_GENERATE_MOCKED_STIMULUS:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			if (mocked.src1) {
				app_func_stim_stim1_start(mocked.imp_en);
			}
			if (mocked.src2) {
				if (mocked.vnsb_en) {
					app_func_stim_sine_start();
				}
				else {
					app_func_stim_stim2_start();
				}
			}
			app_func_stim_sync();
		}
	}
		break;

	case OP_ENABLE_CHANNEL_MUX:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			app_func_stim_mux_enable(true);
		}
	}
		break;

	case OP_SET_STIM_SEL_POSITIONS:
	{
		len_payload = 7;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			StimSelPositions_t stimselpositions;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&stimselpositions.stima_sel, sizeof(stimselpositions.stima_sel));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&stimselpositions.stimb_sel, sizeof(stimselpositions.stimb_sel));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&stimselpositions.stim_sel_encl, sizeof(stimselpositions.stim_sel_encl));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&stimselpositions.stim_sel_ch1, sizeof(stimselpositions.stim_sel_ch1));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&stimselpositions.stim_sel_ch2, sizeof(stimselpositions.stim_sel_ch2));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&stimselpositions.stim_sel_ch3, sizeof(stimselpositions.stim_sel_ch3));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&stimselpositions.stim_sel_ch4, sizeof(stimselpositions.stim_sel_ch4));

			memcpy(&StimSelPositions, &stimselpositions, sizeof(StimSelPositions_t));
			Stim_Sel_t sel = {
					.stimA 			= StimSelPositions.stima_sel,
					.stimB 			= StimSelPositions.stimb_sel,
					.sel_ch.encl 	= StimSelPositions.stim_sel_encl,
					.sel_ch.ch1 	= StimSelPositions.stim_sel_ch1,
					.sel_ch.ch2 	= StimSelPositions.stim_sel_ch2,
					.sel_ch.ch3 	= StimSelPositions.stim_sel_ch3,
					.sel_ch.ch4 	= StimSelPositions.stim_sel_ch4,
			};
			app_func_stim_sel_set(sel);
		}
	}
		break;

	case OP_GET_STIM_SEL_POSITIONS:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			uint8_t resp_payload[sizeof(StimSelPositions)];
			uint8_t* payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&StimSelPositions.stima_sel, sizeof(StimSelPositions.stima_sel));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&StimSelPositions.stimb_sel, sizeof(StimSelPositions.stimb_sel));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&StimSelPositions.stim_sel_encl, sizeof(StimSelPositions.stim_sel_encl));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&StimSelPositions.stim_sel_ch1, sizeof(StimSelPositions.stim_sel_ch1));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&StimSelPositions.stim_sel_ch2, sizeof(StimSelPositions.stim_sel_ch2));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&StimSelPositions.stim_sel_ch3, sizeof(StimSelPositions.stim_sel_ch3));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&StimSelPositions.stim_sel_ch4, sizeof(StimSelPositions.stim_sel_ch4));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_DISABLE_ECG_HR_AFE:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			app_func_meas_sensor_enable(SENSOR_ID_ECG_HR, false);
		}
	}
		break;

	case OP_DISABLE_ECG_RR_AFE:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			app_func_meas_sensor_enable(SENSOR_ID_ECG_RR, false);
		}
	}
		break;

	case OP_ENABLE_ECG_HR_AFE:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			app_func_meas_sensor_enable(SENSOR_ID_ECG_HR, true);
		}
	}
		break;

	case OP_ENABLE_ECG_RR_AFE:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			app_func_meas_sensor_enable(SENSOR_ID_ECG_RR, true);
		}
	}
		break;

	case OP_GET_ECG_HR_LOD:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			EcgHrLod_t ecghrlod = {
					.ecgHrLod = HAL_GPIO_ReadPin(ECG_HR_LOD_GPIO_Port, ECG_HR_LOD_Pin),
			};
			uint8_t resp_payload[sizeof(ecghrlod)];
			uint8_t* payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&ecghrlod.ecgHrLod, sizeof(ecghrlod.ecgHrLod));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_GET_ECG_RR_LOD:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			EcgRrLod_t ecgrrlod = {
					.ecgRrLod = HAL_GPIO_ReadPin(ECG_RR_LOD_GPIO_Port, ECG_RR_LOD_Pin),
			};
			uint8_t resp_payload[sizeof(ecgrrlod)];
			uint8_t* payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&ecgrrlod.ecgRrLod, sizeof(ecgrrlod.ecgRrLod));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_GET_ECG_HR_AFE_OUTPUT:
	{
		len_payload = 3;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			EcgHrAfeOutputReq_t ecghrafeoutputreq;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&ecghrafeoutputreq.bufferSize, sizeof(ecghrafeoutputreq.bufferSize));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&ecghrafeoutputreq.samplingFrequency_hz, sizeof(ecghrafeoutputreq.samplingFrequency_hz));

			EcgHrAfeOutputResp_t ecghrafeoutputresp;
			memset(&ecghrafeoutputresp, 0, sizeof(EcgHrAfeOutputResp_t));
			ecghrafeoutputresp.bufferSize = ecghrafeoutputreq.bufferSize;
			app_func_meas_sensor_meas(SENSOR_ID_ECG_HR, ecghrafeoutputresp.buffer, ecghrafeoutputresp.bufferSize, ecghrafeoutputreq.samplingFrequency_hz);

			uint8_t resp_payload[sizeof(ecghrafeoutputresp)];
			payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&ecghrafeoutputresp.bufferSize, sizeof(ecghrafeoutputresp.bufferSize));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)ecghrafeoutputresp.buffer, sizeof(ecghrafeoutputresp.buffer));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_GET_ECG_RR_AFE_OUTPUT:
	{
		len_payload = 3;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			EcgRrAfeOutputReq_t ecgrrafeoutputreq;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&ecgrrafeoutputreq.bufferSize, sizeof(ecgrrafeoutputreq.bufferSize));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&ecgrrafeoutputreq.samplingFrequency_hz, sizeof(ecgrrafeoutputreq.samplingFrequency_hz));

			EcgRrAfeOutputResp_t ecgrrafeoutputresp;
			memset(&ecgrrafeoutputresp, 0, sizeof(EcgRrAfeOutputResp_t));
			ecgrrafeoutputresp.bufferSize = ecgrrafeoutputreq.bufferSize;
			app_func_meas_sensor_meas(SENSOR_ID_ECG_RR, ecgrrafeoutputresp.buffer, ecgrrafeoutputresp.bufferSize, ecgrrafeoutputreq.samplingFrequency_hz);

			uint8_t resp_payload[sizeof(ecgrrafeoutputresp)];
			payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&ecgrrafeoutputresp.bufferSize, sizeof(ecgrrafeoutputresp.bufferSize));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)ecgrrafeoutputresp.buffer, sizeof(ecgrrafeoutputresp.buffer));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_ENABLE_WPT_VRECT_MON_CIRCUIT:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			app_func_meas_vrect_enable(true);
		}
	}
		break;

	case OP_DISABLE_WPT_VRECT_MON_CIRCUIT:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			app_func_meas_vrect_enable(false);
		}
	}
		break;

	case OP_GET_WPT_VRECT_MON_CIRCUIT_OUTPUT:
	{
		len_payload = 3;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			WptVrectOutputReq_t wptvrectoutputreq;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&wptvrectoutputreq.bufferSize, sizeof(wptvrectoutputreq.bufferSize));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&wptvrectoutputreq.samplingFrequency_hz, sizeof(wptvrectoutputreq.samplingFrequency_hz));

			WptVrectOutputResp_t wptvrectoutputresp;
			memset(&wptvrectoutputresp, 0, sizeof(WptVrectOutputResp_t));
			wptvrectoutputresp.bufferSize = wptvrectoutputreq.bufferSize;
			app_func_meas_vrect_mon_meas(wptvrectoutputresp.buffer, wptvrectoutputresp.bufferSize, wptvrectoutputreq.samplingFrequency_hz);

			uint8_t resp_payload[sizeof(wptvrectoutputresp)];
			payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&wptvrectoutputresp.bufferSize, sizeof(wptvrectoutputresp.bufferSize));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)wptvrectoutputresp.buffer, sizeof(wptvrectoutputresp.buffer));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_GET_VRECT_DET:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			VrectDet_t vrectdet = {
					.det = (!HAL_GPIO_ReadPin(VRECT_DETn_GPIO_Port, VRECT_DETn_Pin)),
			};
			uint8_t resp_payload[sizeof(vrectdet)];
			uint8_t* payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&vrectdet.det, sizeof(vrectdet.det));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_GET_VRECT_OVP:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			VrectOvp_t vrectovp = {
					.ovp = (!HAL_GPIO_ReadPin(VRECT_OVPn_GPIO_Port, VRECT_OVPn_Pin)),
			};
			uint8_t resp_payload[sizeof(vrectovp)];
			uint8_t* payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&vrectovp.ovp, sizeof(vrectovp.ovp));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_GET_VCHG_RAIL_SUPPLY_CIRCUIT_POWER_GOOD:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			VchgRailSupplyPowerGood_t vchgrailsupplypowergood = {
					.pGood = HAL_GPIO_ReadPin(VCHG_PGOOD_GPIO_Port, VCHG_PGOOD_Pin),
			};
			uint8_t resp_payload[sizeof(vchgrailsupplypowergood)];
			uint8_t* payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&vchgrailsupplypowergood.pGood, sizeof(vchgrailsupplypowergood.pGood));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_ENABLE_CHARGE_CONTROL_1_CIRCUIT:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			HAL_GPIO_WritePin(CHG1_EN_GPIO_Port, CHG1_EN_Pin, true);
		}
	}
		break;

	case OP_DISABLE_CHARGE_CONTROL_1_CIRCUIT:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			HAL_GPIO_WritePin(CHG1_EN_GPIO_Port, CHG1_EN_Pin, false);
		}
	}
		break;

	case OP_CHARGE_RATE_CONTROL:
	{
		len_payload = 2;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			ChargeRateControl_t chargeratecontrol;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&chargeratecontrol.chgRate1, sizeof(chargeratecontrol.chgRate1));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&chargeratecontrol.chgRate2, sizeof(chargeratecontrol.chgRate2));

			HAL_GPIO_WritePin(CHG_RATE1_GPIO_Port, CHG_RATE1_Pin, chargeratecontrol.chgRate1);
			HAL_GPIO_WritePin(CHG_RATE2_GPIO_Port, CHG_RATE2_Pin, chargeratecontrol.chgRate2);
		}
	}
		break;

	case OP_GET_CHG1_STATUS:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			Chg1Status_t chg1status = {
					.status = HAL_GPIO_ReadPin(CHG1_STATUS_GPIO_Port, CHG1_STATUS_Pin),
			};
			uint8_t resp_payload[sizeof(chg1status)];
			uint8_t* payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&chg1status.status, sizeof(chg1status.status));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_GET_CHG1_OVP_ERR:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			Chg1OvpErr_t chg1ovperr = {
					.ovpErr = (!HAL_GPIO_ReadPin(CHG1_OVP_ERRn_GPIO_Port, CHG1_OVP_ERRn_Pin)),
			};
			uint8_t resp_payload[sizeof(chg1ovperr)];
			uint8_t* payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&chg1ovperr.ovpErr, sizeof(chg1ovperr.ovpErr));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_ENABLE_CHARGE_CONTROL_2_CIRCUIT:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			HAL_GPIO_WritePin(CHG2_EN_GPIO_Port, CHG2_EN_Pin, true);
		}
	}
		break;

	case OP_DISABLE_CHARGE_CONTROL_2_CIRCUIT:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			HAL_GPIO_WritePin(CHG2_EN_GPIO_Port, CHG2_EN_Pin, false);
		}
	}
		break;

	case OP_GET_CHG2_STATUS:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			Chg2Status_t chg2status = {
					.status = HAL_GPIO_ReadPin(CHG2_STATUS_GPIO_Port, CHG2_STATUS_Pin),
			};
			uint8_t resp_payload[sizeof(chg2status)];
			uint8_t* payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&chg2status.status, sizeof(chg2status.status));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_GET_CHG2_OVP_ERR:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			Chg2OvpErr_t chg2ovperr = {
					.ovpErr = (!HAL_GPIO_ReadPin(CHG2_OVP_ERRn_GPIO_Port, CHG2_OVP_ERRn_Pin)),
			};
			uint8_t resp_payload[sizeof(chg2ovperr)];
			uint8_t* payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&chg2ovperr.ovpErr, sizeof(chg2ovperr.ovpErr));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_ENABLE_THERMISTOR_INTERFACE_CIRCUIT:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			app_func_meas_therm_enable(true);
		}
	}
		break;

	case OP_DISABLE_THERMISTOR_INTERFACE_CIRCUIT:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			app_func_meas_therm_enable(false);
		}
	}
		break;

	case OP_GET_THERM_REF:
	{
		len_payload = 3;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			ThermRefReq_t thermrefreq;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&thermrefreq.bufferSize, sizeof(thermrefreq.bufferSize));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&thermrefreq.samplingFrequency_hz, sizeof(thermrefreq.samplingFrequency_hz));

			ThermRefResp_t thermrefresp;
			memset(&thermrefresp, 0, sizeof(ThermRefResp_t));
			thermrefresp.bufferSize = thermrefreq.bufferSize;
			app_func_meas_therm_meas(THERM_ID_REF, thermrefresp.buffer, thermrefresp.bufferSize, thermrefreq.samplingFrequency_hz);

			uint8_t resp_payload[sizeof(thermrefresp)];
			payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&thermrefresp.bufferSize, sizeof(thermrefresp.bufferSize));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)thermrefresp.buffer, sizeof(thermrefresp.buffer));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_GET_THERM_OUT:
	{
		len_payload = 3;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			ThermOutReq_t thermoutreq;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&thermoutreq.bufferSize, sizeof(thermoutreq.bufferSize));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&thermoutreq.samplingFrequency_hz, sizeof(thermoutreq.samplingFrequency_hz));

			ThermOutResp_t thermoutresp;
			memset(&thermoutresp, 0, sizeof(ThermOutResp_t));
			thermoutresp.bufferSize = thermoutreq.bufferSize;
			app_func_meas_therm_meas(THERM_ID_OUT, thermoutresp.buffer, thermoutresp.bufferSize, thermoutreq.samplingFrequency_hz);

			uint8_t resp_payload[sizeof(thermoutresp)];
			payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&thermoutresp.bufferSize, sizeof(thermoutresp.bufferSize));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)thermoutresp.buffer, sizeof(thermoutresp.buffer));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_GET_THERM_OFST:
	{
		len_payload = 3;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			ThermOffstReq_t thermofstreq;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&thermofstreq.bufferSize, sizeof(thermofstreq.bufferSize));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&thermofstreq.samplingFrequency_hz, sizeof(thermofstreq.samplingFrequency_hz));

			ThermOffstResp_t thermofstresp;
			memset(&thermofstresp, 0, sizeof(ThermOffstResp_t));
			thermofstresp.bufferSize = thermofstreq.bufferSize;
			app_func_meas_therm_meas(THERM_ID_OFST, thermofstresp.buffer, thermofstresp.bufferSize, thermofstreq.samplingFrequency_hz);

			uint8_t resp_payload[sizeof(thermofstresp)];
			payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&thermofstresp.bufferSize, sizeof(thermofstresp.bufferSize));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)thermofstresp.buffer, sizeof(thermofstresp.buffer));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_DISABLE_ENG1_AFE:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			app_func_meas_sensor_enable(SENSOR_ID_ENG1, false);
		}
	}
		break;

	case OP_DISABLE_ENG2_AFE:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			app_func_meas_sensor_enable(SENSOR_ID_ENG2, false);
		}
	}
		break;

	case OP_ENABLE_ENG1_AFE:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			app_func_meas_sensor_enable(SENSOR_ID_ENG1, true);
		}
	}
		break;

	case OP_ENABLE_ENG2_AFE:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			app_func_meas_sensor_enable(SENSOR_ID_ENG2, true);
		}
	}
		break;

	case OP_GET_ENG1_LOD:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			Eng1Lod_t eng1lod = {
					.eng1Lod = HAL_GPIO_ReadPin(ENG1_LOD_GPIO_Port, ENG1_LOD_Pin),
			};
			uint8_t resp_payload[sizeof(eng1lod)];
			uint8_t* payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&eng1lod.eng1Lod, sizeof(eng1lod.eng1Lod));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_GET_ENG2_LOD:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			Eng2Lod_t eng2lod = {
					.eng2Lod = HAL_GPIO_ReadPin(ENG2_LOD_GPIO_Port, ENG2_LOD_Pin),
			};
			uint8_t resp_payload[sizeof(eng2lod)];
			uint8_t* payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&eng2lod.eng2Lod, sizeof(eng2lod.eng2Lod));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_GET_ENG1_AFE_OUTPUT:
	{
		len_payload = 3;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			Eng1AfeOutputReq_t eng1afeoutputreq;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&eng1afeoutputreq.bufferSize, sizeof(eng1afeoutputreq.bufferSize));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&eng1afeoutputreq.samplingFrequency_hz, sizeof(eng1afeoutputreq.samplingFrequency_hz));

			Eng1AfeOutputResp_t eng1afeoutputresp;
			memset(&eng1afeoutputresp, 0, sizeof(Eng1AfeOutputResp_t));
			eng1afeoutputresp.bufferSize = eng1afeoutputreq.bufferSize;
			app_func_meas_sensor_meas(SENSOR_ID_ENG1, eng1afeoutputresp.buffer, eng1afeoutputresp.bufferSize, eng1afeoutputreq.samplingFrequency_hz);

			uint8_t resp_payload[sizeof(eng1afeoutputresp)];
			payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&eng1afeoutputresp.bufferSize, sizeof(eng1afeoutputresp.bufferSize));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)eng1afeoutputresp.buffer, sizeof(eng1afeoutputresp.buffer));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_GET_ENG2_AFE_OUTPUT:
	{
		len_payload = 3;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			Eng2AfeOutputReq_t eng2afeoutputreq;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&eng2afeoutputreq.bufferSize, sizeof(eng2afeoutputreq.bufferSize));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&eng2afeoutputreq.samplingFrequency_hz, sizeof(eng2afeoutputreq.samplingFrequency_hz));

			Eng2AfeOutputResp_t eng2afeoutputresp;
			memset(&eng2afeoutputresp, 0, sizeof(Eng2AfeOutputResp_t));
			eng2afeoutputresp.bufferSize = eng2afeoutputreq.bufferSize;
			app_func_meas_sensor_meas(SENSOR_ID_ENG2, eng2afeoutputresp.buffer, eng2afeoutputresp.bufferSize, eng2afeoutputreq.samplingFrequency_hz);

			uint8_t resp_payload[sizeof(eng2afeoutputresp)];
			payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&eng2afeoutputresp.bufferSize, sizeof(eng2afeoutputresp.bufferSize));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)eng2afeoutputresp.buffer, sizeof(eng2afeoutputresp.buffer));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_GET_MAG_STATUS:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			MagStatus_t magstatus = {
					.status = HAL_GPIO_ReadPin(MAG_DET_GPIO_Port, MAG_DET_Pin),
			};
			uint8_t resp_payload[sizeof(magstatus)];
			uint8_t* payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&magstatus.status, sizeof(magstatus.status));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_ENABLE_IMC:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			app_func_meas_imp_enable(true);
			mocked.imp_en = true;
		}
	}
		break;

	case OP_DISABLE_IMC:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			app_func_meas_imp_enable(false);
			mocked.imp_en = false;
		}
	}
		break;

	case OP_SET_IMP_SEL_POSITIONS:
	{
		len_payload = 4;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			ImpSelPositions_t impselpositions;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&impselpositions.imp_in_n_sel0, sizeof(impselpositions.imp_in_n_sel0));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&impselpositions.imp_in_n_sel1, sizeof(impselpositions.imp_in_n_sel1));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&impselpositions.imp_in_n_sel2, sizeof(impselpositions.imp_in_n_sel2));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&impselpositions.imp_in_p_sel0, sizeof(impselpositions.imp_in_p_sel0));

			memcpy(&ImpSelPositions, &impselpositions, sizeof(ImpSelPositions_t));
			app_func_meas_imp_sel_set(	ImpSelPositions.imp_in_n_sel0,
										ImpSelPositions.imp_in_n_sel1,
										ImpSelPositions.imp_in_n_sel2,
										ImpSelPositions.imp_in_p_sel0	);
		}
	}
		break;

	case OP_GET_IMP_SEL_POSITIONS:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			uint8_t resp_payload[sizeof(ImpSelPositions)];
			uint8_t* payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&ImpSelPositions.imp_in_n_sel0, sizeof(ImpSelPositions.imp_in_n_sel0));
			payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&ImpSelPositions.imp_in_n_sel1, sizeof(ImpSelPositions.imp_in_n_sel1));
			payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&ImpSelPositions.imp_in_n_sel2, sizeof(ImpSelPositions.imp_in_n_sel2));
			payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&ImpSelPositions.imp_in_p_sel0, sizeof(ImpSelPositions.imp_in_p_sel0));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_GET_IMC_MEASURE:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			const uint16_t samplingFrequency_hz = 50000;
			static uint16_t impVoltageBufferA[ADC_MAX_SAMPLE_POINTS];
			static uint16_t impVoltageBufferB[ADC_MAX_SAMPLE_POINTS];
			uint16_t periodPoints = app_func_meas_imp_sampPoints_get(samplingFrequency_hz, StimulusCircuitParameters.pulsePeriod1_us);
			uint16_t pulsePoints = app_func_meas_imp_sampPoints_get(samplingFrequency_hz, StimulusCircuitParameters.pulseWidth1_us);

			HAL_Delay(StimulusCircuitParameters.stimDuration1_ms);
			app_func_stim_sync();
			app_func_meas_imp_volt_meas(IMPIN_CH_P, impVoltageBufferA, periodPoints, samplingFrequency_hz);
			app_func_stim_sync();
			app_func_meas_imp_volt_meas(IMPIN_CH_N, impVoltageBufferB, periodPoints, samplingFrequency_hz);

			_Float64 impVoltageA = app_func_meas_imp_volt_calc(impVoltageBufferA, periodPoints, pulsePoints);
			_Float64 impVoltageB = app_func_meas_imp_volt_calc(impVoltageBufferB, periodPoints, pulsePoints);
			_Float64 impVoltage = impVoltageA + impVoltageB;

			_Float64 dacStimA_mV = ((StimSelPositions.stima_sel == STIMA_SEL_STIM1)
					?(_Float64)DacAbOutputVoltage.aDacOutputVoltage_mv:(_Float64)DacAbOutputVoltage.bDacOutputVoltage_mv);

			_Float64 dacStimB_mV = ((StimSelPositions.stimb_sel == STIMB_SEL_STIM1)
					?(_Float64)DacAbOutputVoltage.aDacOutputVoltage_mv:(_Float64)DacAbOutputVoltage.bDacOutputVoltage_mv);

			_Float64 dacStim_mV = ((ImpSelPositions.imp_in_p_sel0 == IMPIN_P_STIMA)?dacStimA_mV:dacStimB_mV);
			_Float64 impedance = app_func_meas_imp_calc(dacStim_mV, impVoltage);

			ImcMeasure_t ImcMeasure  = {
					.imc_measure = (uint16_t)impedance
			};
			uint8_t resp_payload[sizeof(ImcMeasure)];
			uint8_t* payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&ImcMeasure.imc_measure, sizeof(ImcMeasure.imc_measure));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_DISABLE_HV_SUPPLY:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			TestInformation.hvSupplyEnable = 0;
			app_func_stim_hv_supply_set(hvSupplyTurnOn, TestInformation.hvSupplyEnable);
		}
	}
		break;

	case OP_DISABLE_VDDS_SUPPLY:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			TestInformation.vddsSupplyEnable = 0;
			app_func_stim_vdds_sup_enable(TestInformation.vddsSupplyEnable);
		}
	}
		break;

	case OP_DISABLE_VDDA_SUPPLY:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			TestInformation.vddaSupplyEnable = 0;
			app_func_meas_vdda_sup_enable(TestInformation.vddaSupplyEnable);
		}
	}
		break;

	case OP_DISABLE_STIMULUS_CIRCUIT:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			app_func_stim_stimulus_enable(false);
		}
	}
		break;

	case OP_START_ACC:
	{
		len_payload = 2;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			FreqModeDeviceID_t freqModedeviceID;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&freqModedeviceID.FreqSampling, sizeof(freqModedeviceID.FreqSampling));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&freqModedeviceID.ModeDeviceID, sizeof(freqModedeviceID.ModeDeviceID));

			uint8_t resp_payload[sizeof(AccMeasure_t)];
			resp.Status = app_mode_dvt_acc_start(freqModedeviceID, (uint8_t*)resp_payload, &resp.PayloadLen);
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_GET_DATA_ACC:
	{
		len_payload = 1;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			DeviceID_t deviceID;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&deviceID.DeviceID, sizeof(deviceID.DeviceID));

			uint8_t resp_payload[sizeof(InfoData_t)];
			resp.Status = app_mode_dvt_acc_data_get(deviceID, (uint8_t*)resp_payload, &resp.PayloadLen);
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_STOP_ACC:
	{
		len_payload = 1;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			DeviceID_t deviceID;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&deviceID.DeviceID, sizeof(deviceID.DeviceID));

			uint8_t resp_payload[sizeof(DeviceID_t)];
			resp.Status = app_mode_dvt_acc_stop(deviceID, (uint8_t*)resp_payload, &resp.PayloadLen);
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_ENABLE_VCHG_RAIL_SUPPLY:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			HAL_GPIO_WritePin(VCHG_DISABLE_GPIO_Port, VCHG_DISABLE_Pin, false);
		}
	}
		break;

	case OP_DISABLE_VCHG_RAIL_SUPPLY:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			HAL_GPIO_WritePin(VCHG_DISABLE_GPIO_Port, VCHG_DISABLE_Pin, true);
		}
	}
		break;

	default:
	{
		resp.Status = STATUS_OPCODE_ERR;
	}
		break;
	}

	return resp;
}

static BLE_ADV_Setting_t setting = {
		.advance = {
			.passkey = "000000",
			.whitelist_enable = 0,
		},
		.adv_timeout = {0x01,0x00,0x00,0x00},	//1sec
		.passkey_timeout = {0,0,0,0},
		.companyid = BLE_COMPANY_ID_TEST,
		.msd = {
				0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,
				HW_VERSION,
		},
};

static void app_mode_dvt_init(void) {
	hvSupplyTurnOn 						= HAL_GPIO_ReadPin(HVSW_EN_GPIO_Port, HVSW_EN_Pin);
	TestInformation.hvSupplyEnable 		= HAL_GPIO_ReadPin(VPPSW_EN_GPIO_Port, VPPSW_EN_Pin);
	TestInformation.vddsSupplyEnable 	= HAL_GPIO_ReadPin(VDDS_EN_GPIO_Port, VDDS_EN_Pin);
	TestInformation.vddaSupplyEnable 	= HAL_GPIO_ReadPin(VDDA_EN_GPIO_Port, VDDA_EN_Pin);

	CurrentSourcesConfiguration.src1 	= HAL_GPIO_ReadPin(SRC1_GPIO_Port, SRC1_Pin);
	CurrentSourcesConfiguration.src2 	= HAL_GPIO_ReadPin(SRC2_GPIO_Port, SRC2_Pin);
	CurrentSourcesConfiguration.snk1 	= HAL_GPIO_ReadPin(SNK1_GPIO_Port, SNK1_Pin);
	CurrentSourcesConfiguration.snk2 	= HAL_GPIO_ReadPin(SNK2_GPIO_Port, SNK2_Pin);
	CurrentSourcesConfiguration.snk3 	= HAL_GPIO_ReadPin(SNK3_GPIO_Port, SNK3_Pin);
	CurrentSourcesConfiguration.snk4 	= HAL_GPIO_ReadPin(SNK4_GPIO_Port, SNK4_Pin);
	CurrentSourcesConfiguration.snk5 	= HAL_GPIO_ReadPin(SNK5_GPIO_Port, SNK5_Pin);

	StimSelPositions.stima_sel 			= (!HAL_GPIO_ReadPin(STIMA_SELn_GPIO_Port, STIMA_SELn_Pin));
	StimSelPositions.stimb_sel 			= (!HAL_GPIO_ReadPin(STIMB_SELn_GPIO_Port, STIMB_SELn_Pin));
	StimSelPositions.stim_sel_encl		= (!HAL_GPIO_ReadPin(STIM_SEL_ENCLn_GPIO_Port, STIM_SEL_ENCLn_Pin));
	StimSelPositions.stim_sel_ch1		= (!HAL_GPIO_ReadPin(STIM_SEL_CH1n_GPIO_Port, STIM_SEL_CH1n_Pin));
	StimSelPositions.stim_sel_ch2		= (!HAL_GPIO_ReadPin(STIM_SEL_CH2n_GPIO_Port, STIM_SEL_CH2n_Pin));
	StimSelPositions.stim_sel_ch3		= (!HAL_GPIO_ReadPin(STIM_SEL_CH3n_GPIO_Port, STIM_SEL_CH3n_Pin));
	StimSelPositions.stim_sel_ch4		= (!HAL_GPIO_ReadPin(STIM_SEL_CH4n_GPIO_Port, STIM_SEL_CH4n_Pin));

	ImpSelPositions.imp_in_n_sel0		= HAL_GPIO_ReadPin(IMP_IN_N_SEL0_GPIO_Port, IMP_IN_N_SEL0_Pin);
	ImpSelPositions.imp_in_n_sel1		= HAL_GPIO_ReadPin(IMP_IN_N_SEL1_GPIO_Port, IMP_IN_N_SEL1_Pin);
	ImpSelPositions.imp_in_n_sel2		= HAL_GPIO_ReadPin(IMP_IN_N_SEL2_GPIO_Port, IMP_IN_N_SEL2_Pin);
	ImpSelPositions.imp_in_p_sel0		= HAL_GPIO_ReadPin(IMP_IN_P_SEL_GPIO_Port, IMP_IN_P_SEL_Pin);

	mocked.src1							= HAL_GPIO_ReadPin(SRC1_GPIO_Port, SRC1_Pin);
	mocked.src2							= HAL_GPIO_ReadPin(SRC2_GPIO_Port, SRC2_Pin);
	mocked.vnsb_en 						= HAL_GPIO_ReadPin(VNSb_EN_GPIO_Port, VNSb_EN_Pin);
	mocked.imp_en						= HAL_GPIO_ReadPin(IMP_EN_GPIO_Port, IMP_EN_Pin);

	app_func_stim_dac_init();
	app_func_stim_dac_volt_set(DacAbOutputVoltage.aDacOutputVoltage_mv, DacAbOutputVoltage.bDacOutputVoltage_mv);

	Stimulus_Waveform_t para1 = {
			.pulseWidth_us 			= StimulusCircuitParameters.pulseWidth1_us,
			.pulsePeriod_us 		= StimulusCircuitParameters.pulsePeriod1_us,
			.trainOnDuration_ms 	= StimulusCircuitParameters.stimDuration1_ms,
			.trainOffDuration_ms 	= StimulusCircuitParameters.stimDuration1_ms,
	};
	Stimulus_Waveform_t para2 = {
			.pulseWidth_us 			= StimulusCircuitParameters.pulseWidth2_us,
			.pulsePeriod_us 		= StimulusCircuitParameters.pulsePeriod2_us,
			.trainOnDuration_ms 	= StimulusCircuitParameters.stimDuration2_ms,
			.trainOffDuration_ms 	= StimulusCircuitParameters.stimDuration2_ms,
	};
	NerveBlock_Waveform_t sine_para = {
			.sinePeriod_us			= (uint32_t)(1000000.0 / (_Float64)StimulusCircuitParameters.stimFreqVNS_hz),
			.sinePhaseShift_us		= 0,
			.amplitude_mV 			= DacAbOutputVoltage.bDacOutputVoltage_mv,
			.trainOnDuration_ms 	= StimulusCircuitParameters.stimDurationVNS_ms,
			.trainOffDuration_ms 	= StimulusCircuitParameters.stimDurationVNS_ms,
	};
	app_func_stim_circuit_para1_set(para1);
	app_func_stim_circuit_para2_set(para2);
	app_func_stim_sine_para_set(sine_para);
}

static bool ble_peers_del = false;
extern bool	ble_peers_is_deleted;

/**
 * @brief Handler for DVT mode
 *
 */
void app_mode_dvt_handler(void) {
	bsp_wdg_enable(false);
	app_mode_dvt_init();
	app_func_command_req_parser_set(&app_mode_dvt_command_req_parser);

	uint8_t curr_ble_state = app_func_ble_curr_state_get();
	if (curr_ble_state == BLE_STATE_INVALID) {
		app_func_ble_enable(true);
		curr_ble_state = app_func_ble_curr_state_get();
	}

	while(1) {
		if (curr_ble_state == BLE_STATE_ADV_STOP) {
			if (!ble_peers_del) {
				app_func_ble_peers_del();
				ble_peers_del = true;
			}
			else {
				app_mode_ble_act_adv_msd_update((uint8_t*)setting.msd);
				app_func_ble_adv_start(&setting);
			}
		}
		else {
			app_func_ble_new_state_get();
		}

		while(!bsp_sp_cmd_handler()) {
			HAL_Delay(1);
		}
		curr_ble_state = app_func_ble_curr_state_get();

#ifdef DVT_TEST
		curr_ble_state = BLE_STATE_CONNECT + BLE_STATE_SEC_EN + BLE_STATE_NUS_READY;
		app_mode_dvt_test_handler();
#endif

		if (shutdown) {
			HAL_GPIO_WritePin(IPG_SHDN_GPIO_Port, IPG_SHDN_Pin, GPIO_PIN_SET);
		}

		if (sw_reset) {
			app_func_ble_enable(false);
			app_func_ble_enable(true);
			app_func_ble_peers_del();

			while(!ble_peers_is_deleted) {
				bsp_sp_cmd_handler();
				HAL_Delay(200);
			}

			HAL_NVIC_SystemReset();
		}
	}
}

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
uint8_t app_mode_dvt_acc_start(FreqModeDeviceID_t freqModedeviceID, uint8_t* dataBuffer, uint8_t* dataSize) {
	uint8_t mode = (freqModedeviceID.ModeDeviceID & 0x80) >> 7;
	uint8_t devID = freqModedeviceID.ModeDeviceID & 0x7F;
	uint8_t deviceAddr = devID << 1;

	dataBuffer[0] = devID;
	*dataSize = 1U;

	if (	freqModedeviceID.FreqSampling != 10U &&
			freqModedeviceID.FreqSampling != 25U &&
			freqModedeviceID.FreqSampling != 50U &&
			freqModedeviceID.FreqSampling != 100U &&
			freqModedeviceID.FreqSampling != 200U) {
		return STATUS_START_ACC_INVALID_CONFIGURATION;
	}
	else if (	deviceAddr != MIS2DHTR_DEVICE_ADDR_L &&
				deviceAddr != MIS2DHTR_DEVICE_ADDR_H) {
		return STATUS_START_ACC_INVALID_CONFIGURATION;
	}

	bsp_sp_XL_enable(true);
	for (int i = 0;i < 2;i++) {
		if (acc[i].Device.DeviceAddress == deviceAddr) {
			acc[i].ChannelsOpen = true;
			if (HAL_I2C_IsDeviceReady(&HANDLE_ACC_I2C, acc[i].Device.DeviceAddress, 3, 5) == HAL_OK) {
				HAL_StatusTypeDef status = HAL_OK;
				acc[i].Mode = ON_DEMAND;
				uint8_t Watermark = 28U;

				acc[i].Device.RegisterAddress = MIS2DHTR_WHO_AM_I;
				bsp_sp_MIS2DHTR_read(acc[i].Device.DeviceAddress, acc[i].Device.RegisterAddress, &acc[i].Device.RegisterData, 1);

				if (acc[i].Device.RegisterData == MIS2DHTR_WHO_AM_I_DATA) {
					acc[i].Device.RegisterAddress = MIS2DHTR_CTRL_REG1;
					acc[i].Device.RegisterData = MIS2DHTR_CTRL_REG1_data_get(0U);
					bsp_sp_MIS2DHTR_write(acc[i].Device.DeviceAddress, acc[i].Device.RegisterAddress, &acc[i].Device.RegisterData, 1);

					acc[i].Device.RegisterAddress = MIS2DHTR_CTRL_REG5;
					acc[i].Device.RegisterData = MIS2DHTR_FIFO_EN;
					bsp_sp_MIS2DHTR_write(acc[i].Device.DeviceAddress, acc[i].Device.RegisterAddress, &acc[i].Device.RegisterData, 1);

					acc[i].Device.RegisterAddress = MIS2DHTR_FIFO_CTRL_REG;
					acc[i].Device.RegisterData = MIS2DHTR_FM_STREAM + MIS2DHTR_TR_INT1 + Watermark;
					bsp_sp_MIS2DHTR_write(acc[i].Device.DeviceAddress, MIS2DHTR_FIFO_CTRL_REG, &acc[i].Device.RegisterData, 1);

					acc[i].Device.RegisterAddress = MIS2DHTR_FIFO_SRC_REG;
					bsp_sp_MIS2DHTR_read(acc[i].Device.DeviceAddress, acc[i].Device.RegisterAddress, &acc[i].Device.Fifo.SrcRegister, 1);
					if (!MIS2DHTR_FIFO_IS_EMPTY(acc[i].Device.Fifo.SrcRegister)) {
						uint8_t count = MIS2DHTR_FIFO_GET_FSS(acc[i].Device.Fifo.SrcRegister) + 1U;
						acc[i].Device.RegisterAddress = MIS2DHTR_OUT_X_L + MIS2DHTR_ADDR_AUTO_INCREMENT;
						bsp_sp_MIS2DHTR_read(acc[i].Device.DeviceAddress, acc[i].Device.RegisterAddress, (uint8_t*)acc[i].Device.Fifo.Axis, sizeof(XYZ_t) * count);
					}

					acc[i].Device.RegisterAddress = MIS2DHTR_CTRL_REG1;
					acc[i].Device.RegisterData = MIS2DHTR_CTRL_REG1_data_get(freqModedeviceID.FreqSampling);
					status += bsp_sp_MIS2DHTR_write(acc[i].Device.DeviceAddress, acc[i].Device.RegisterAddress, &acc[i].Device.RegisterData, 1);

					if (status != HAL_OK) {
						return STATUS_START_ACC_COMMUNICTION_ERR;
					}
					else {
						acc[i].WriteIndex = 0U;
						acc[i].ReadIndex = 0U;
						acc[i].FifoIndex = 0U;
						acc[i].Overflow = false;
						acc[i].Startup = true;

						do {
							acc[i].Device.RegisterAddress = MIS2DHTR_FIFO_SRC_REG;
							bsp_sp_MIS2DHTR_read(acc[i].Device.DeviceAddress, acc[i].Device.RegisterAddress, &acc[i].Device.Fifo.SrcRegister, 1);
						} while (MIS2DHTR_FIFO_IS_EMPTY(acc[i].Device.Fifo.SrcRegister));

						uint8_t count = MIS2DHTR_FIFO_GET_FSS(acc[i].Device.Fifo.SrcRegister) + 1U;
						acc[i].Device.RegisterAddress = MIS2DHTR_OUT_X_L + MIS2DHTR_ADDR_AUTO_INCREMENT;
						bsp_sp_MIS2DHTR_read(acc[i].Device.DeviceAddress, acc[i].Device.RegisterAddress, (uint8_t*)acc[i].Device.Fifo.Axis, sizeof(XYZ_t) * count);
						acc[i].LastFifoTimestamp = HAL_GetTick();
						acc[i].Mode = mode;

						AccMeasure_t AccMeasure = {
								.DeviceID = devID,
						};
						memcpy((uint8_t*)&AccMeasure.XYZ, (uint8_t*)&acc[i].Device.Fifo.Axis[count - 1U], sizeof(XYZ_t));

						uint8_t* payload_offset = dataBuffer;
						payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&AccMeasure.DeviceID, sizeof(AccMeasure.DeviceID));
						payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&AccMeasure.XYZ.X, sizeof(AccMeasure.XYZ.X));
						payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&AccMeasure.XYZ.Y, sizeof(AccMeasure.XYZ.Y));
						payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&AccMeasure.XYZ.Z, sizeof(AccMeasure.XYZ.Z));

						*dataSize = (uint8_t)(payload_offset - dataBuffer);
						return STATUS_SUCCESS;
					}
				}
				else {
					return STATUS_START_ACC_DEVICE_NOT_READY;
				}
			}
			else {
				return STATUS_START_ACC_DEVICE_NOT_READY;
			}
		}
	}
	return STATUS_START_ACC_DEVICE_NOT_READY;
}

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
uint8_t app_mode_dvt_acc_data_get(DeviceID_t deviceID, uint8_t* dataBuffer, uint8_t* dataSize) {
	uint8_t deviceAddr = deviceID.DeviceID << 1;

	dataBuffer[0] = deviceID.DeviceID;
	*dataSize = 1U;

	if (deviceAddr != MIS2DHTR_DEVICE_ADDR_L &&
		deviceAddr != MIS2DHTR_DEVICE_ADDR_H) {
		return STATUS_GET_ACC_COMMUNICTION_ERR;
	}

	for (int i = 0;i < 2;i++) {
		if (acc[i].Device.DeviceAddress == deviceAddr) {
			if (acc[i].Startup == false) {
				return STATUS_GET_ACC_STARTUP_NOT_PERFORMED;
			}
			else if (acc[i].Mode == ON_DEMAND) {
				acc[i].Device.RegisterAddress = MIS2DHTR_FIFO_SRC_REG;
				HAL_StatusTypeDef status = bsp_sp_MIS2DHTR_read(acc[i].Device.DeviceAddress, acc[i].Device.RegisterAddress, &acc[i].Device.Fifo.SrcRegister, 1);
				if (status != HAL_OK) {
					return STATUS_GET_ACC_COMMUNICTION_ERR;
				}
				else if (MIS2DHTR_FIFO_IS_EMPTY(acc[i].Device.Fifo.SrcRegister)) {
					return STATUS_GET_ACC_BUFFER_EMPTY;
				}
				else {
					uint8_t status = STATUS_SUCCESS;
					if (MIS2DHTR_FIFO_IS_OVRN(acc[i].Device.Fifo.SrcRegister)) {
						status = STATUS_GET_ACC_BUFFER_OVERFLOW;
					}

					uint8_t count = MIS2DHTR_FIFO_GET_FSS(acc[i].Device.Fifo.SrcRegister) + 1U;
					acc[i].Device.RegisterAddress = MIS2DHTR_OUT_X_L + MIS2DHTR_ADDR_AUTO_INCREMENT;
					bsp_sp_MIS2DHTR_read(acc[i].Device.DeviceAddress, acc[i].Device.RegisterAddress, (uint8_t*)acc[i].Device.Fifo.Axis, sizeof(XYZ_t) * count);

					InfoData_t InfoData = {
							.DeviceID = deviceID.DeviceID,
							.Timestamp = HAL_GetTick(),
							.SampleIndex = acc[i].FifoIndex,
					};
					memcpy((uint8_t*)InfoData.XYZ, (uint8_t*)acc[i].Device.Fifo.Axis, sizeof(XYZ_t) * count);
					acc[i].FifoIndex += count;

					uint8_t* payload_offset = dataBuffer;
					payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&InfoData.DeviceID, sizeof(InfoData.DeviceID));
					payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&InfoData.Timestamp, sizeof(InfoData.Timestamp));
					payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&InfoData.SampleIndex, sizeof(InfoData.SampleIndex));
					for (int j = 0;j < count;j++) {
						payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&InfoData.XYZ[j].X, sizeof(InfoData.XYZ[j].X));
						payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&InfoData.XYZ[j].Y, sizeof(InfoData.XYZ[j].Y));
						payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&InfoData.XYZ[j].Z, sizeof(InfoData.XYZ[j].Z));
					}

					*dataSize = (uint8_t)(payload_offset - dataBuffer);
					return status;
				}
			}
			else if (acc[i].Mode == BUFFER_STORAGE) {
				acc[i].WriteSuspend = true;
				if (acc[i].Overflow == false && acc[i].ReadIndex == acc[i].WriteIndex) {
					acc[i].WriteSuspend = false;
					return STATUS_GET_ACC_BUFFER_EMPTY;
				}
				else {
					uint8_t status = STATUS_SUCCESS;
					if (acc[i].Overflow) {
						status = STATUS_GET_ACC_BUFFER_OVERFLOW;
					}

					InfoData_t InfoData = {
							.DeviceID = deviceID.DeviceID,
							.Timestamp = acc[i].Timestamp[acc[i].ReadIndex],
							.SampleIndex = acc[i].ReadIndex,
					};
					uint16_t count_max = 32U;
					uint16_t count = count_max;
					for (uint16_t j = 0; j < count_max; j++) {
						memcpy((uint8_t*)&InfoData.XYZ[j], (uint8_t*)&acc[i].SampleBuffer[acc[i].ReadIndex], sizeof(Axis_t));
						acc[i].ReadIndex = (acc[i].ReadIndex + 1) % XYZ_BUFF_SIZE;

						if (acc[i].ReadIndex == acc[i].WriteIndex) {
							count = j + 1U;
							j = count_max;
						}
					}
					acc[i].Overflow = false;

					uint8_t* payload_offset = dataBuffer;
					payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&InfoData.DeviceID, sizeof(InfoData.DeviceID));
					payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&InfoData.Timestamp, sizeof(InfoData.Timestamp));
					payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&InfoData.SampleIndex, sizeof(InfoData.SampleIndex));
					for (int j = 0;j < count;j++) {
						payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&InfoData.XYZ[j].X, sizeof(InfoData.XYZ[j].X));
						payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&InfoData.XYZ[j].Y, sizeof(InfoData.XYZ[j].Y));
						payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&InfoData.XYZ[j].Z, sizeof(InfoData.XYZ[j].Z));
					}

					*dataSize = (uint8_t)(payload_offset - dataBuffer);
					acc[i].WriteSuspend = false;
					return status;
				}
			}
		}
	}
	return STATUS_GET_ACC_COMMUNICTION_ERR;
}

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
uint8_t app_mode_dvt_acc_stop(DeviceID_t deviceID, uint8_t* dataBuffer, uint8_t* dataSize) {
	uint8_t deviceAddr = deviceID.DeviceID << 1;

	dataBuffer[0] = deviceID.DeviceID;
	*dataSize = 1U;

	for (int i = 0;i < 2;i++) {
		if (acc[i].Device.DeviceAddress == deviceAddr) {
			acc[i].ChannelsOpen = false;
			acc[i].Mode = ON_DEMAND;
			if (acc[i].Startup) {
				acc[i].Device.RegisterAddress = MIS2DHTR_CTRL_REG1;
				acc[i].Device.RegisterData = MIS2DHTR_CTRL_REG1_data_get(0U);
				bsp_sp_MIS2DHTR_write(acc[i].Device.DeviceAddress, acc[i].Device.RegisterAddress, &acc[i].Device.RegisterData, 1);
				acc[i].Startup = false;
			}
		}
	}

	if (acc[0].ChannelsOpen == false && acc[1].ChannelsOpen == false) {
		bsp_sp_XL_enable(false);
	}

	return STATUS_SUCCESS;
}

/**
 * @brief Callback for the accelerometer timer, unit: ms
 *
 */
void app_mode_dvt_acc_timer_cb(void) {
	for (int i = 0;i < 2;i++) {
		if (acc[i].Mode == BUFFER_STORAGE && acc[i].WriteSuspend == false) {
			acc[i].Device.RegisterAddress = MIS2DHTR_FIFO_SRC_REG;
			HAL_StatusTypeDef status = bsp_sp_MIS2DHTR_read(acc[i].Device.DeviceAddress, acc[i].Device.RegisterAddress, &acc[i].Device.Fifo.SrcRegister, 1);
			if (status == HAL_OK) {
				if (MIS2DHTR_FIFO_IS_WTM(acc[i].Device.Fifo.SrcRegister)) {
					uint8_t count = MIS2DHTR_FIFO_GET_FSS(acc[i].Device.Fifo.SrcRegister);
					acc[i].Device.RegisterAddress = MIS2DHTR_OUT_X_L + MIS2DHTR_ADDR_AUTO_INCREMENT;
					status = bsp_sp_MIS2DHTR_read(acc[i].Device.DeviceAddress, acc[i].Device.RegisterAddress, (uint8_t*)acc[i].Device.Fifo.Axis, sizeof(XYZ_t) * count);
					if (status == HAL_OK) {
						uint32_t lastTick = acc[i].LastFifoTimestamp;
						uint32_t currTick = HAL_GetTick();
						for (uint16_t j = 0; j < count; j++) {
							uint16_t nextWriteIndex = (acc[i].WriteIndex + 1) % XYZ_BUFF_SIZE;
							if (nextWriteIndex == acc[i].ReadIndex) {
								acc[i].Overflow = true;
								acc[i].ReadIndex = (acc[i].ReadIndex + 1) % XYZ_BUFF_SIZE;
							}
							acc[i].SampleBuffer[acc[i].WriteIndex] = acc[i].Device.Fifo.Axis[j];
							acc[i].Timestamp[acc[i].WriteIndex] = lastTick + ((currTick - lastTick) * (j + 1)) / count;
							acc[i].LastFifoTimestamp = acc[i].Timestamp[acc[i].WriteIndex];
							acc[i].WriteIndex = nextWriteIndex;
						}
					}
				}
			}
		}
	}
}
