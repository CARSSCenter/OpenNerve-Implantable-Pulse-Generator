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

static TestInformation_t TestInformation = {
		.hvSupplyEnable = 0,
		.vddsSupplyEnable = 0,
		.vddaSupplyEnable = 0,
		.reserved = {0,0,0,0,0},
};

static StimulusCircuitParameters_t StimulusCircuitParameters = {
		.pulseWidth_us = 100,
		.pulseNumber_ms = 1,
		.pulsePeriod_us = 1000,
		.reserved = {0,0,0},
};

static CurrentSourcesConfiguration_t CurrentSourcesConfiguration = {
		.src1 = false,
		.sink1 = false,
		.src2 = false,
		.sink2 = false,
		.src3 = false,
		.sink3 = false,
		.src4 = false,
		.sink4 = false,
		.src5 = false,
		.sink5 = false,
		.src6 = false,
		.sink6 = false,
		.src7 = false,
		.sink7 = false,
		.src8 = false,
		.sink8 = false,
		.src17 = false,
		.sink17 = false,
};

static SrSelPositions_t SrSelPositions = {
		.sr_sel1 = 0,
		.sr_sel2 = 0,
		.ecg_sel = 0,
};

static ImpSelPositions_t ImpSelPositions = {
		.impa_sel0 = false,
		.impa_sel1 = false,
		.impa_sel2 = false,
		.impa_sel3 = false,
		.impb_sel0 = false,
		.impb_sel1 = false,
		.impb_sel2 = false,
		.impb_sel3 = false,
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

static Current_Sources_t configureCurrentSources(ImpSelPositions_t impselpositions) {
    Current_Sources_t configuration = {0}; // Initialize all pins to {false, false}

    // Calculate src and sink indexes from impselpositions bits (range: 0 to 15)
    uint8_t src_index = (impselpositions.impa_sel3 << 3) |
                        (impselpositions.impa_sel2 << 2) |
                        (impselpositions.impa_sel1 << 1) |
                        (impselpositions.impa_sel0);

    uint8_t sink_index = (impselpositions.impb_sel3 << 3) |
                         (impselpositions.impb_sel2 << 2) |
                         (impselpositions.impb_sel1 << 1) |
                         (impselpositions.impb_sel0);

    // Create a pointer to the Src_Sink_t array
    Src_Sink_t* p_config = (Src_Sink_t*)&configuration;

    // Set src and sink based on src_index and sink_index
    if (src_index <= 8) {
        p_config[src_index].src = true; // s1 to s8 or s17
    }
    if (sink_index <= 8) {
        p_config[sink_index].sink = true; // s1 to s8 or s17
    }

    return configuration;
}

static void generatePhiSinglePulse(bool isPh1) {
	Stimulus_Waveform_t parameters = {
			.pulseWidth_us 			= StimulusCircuitParameters.pulseWidth_us,
			.pulsePeriod_us 		= StimulusCircuitParameters.pulsePeriod_us,
			.trainOnDuration_ms 	= StimulusCircuitParameters.pulseNumber_ms,
			.trainOffDuration_ms 	= 0,
	};
	Current_Sources_t config = {
			.s1.src 	= CurrentSourcesConfiguration.src1,
			.s1.sink 	= CurrentSourcesConfiguration.sink1,
			.s2.src 	= CurrentSourcesConfiguration.src2,
			.s2.sink 	= CurrentSourcesConfiguration.sink2,
			.s3.src 	= CurrentSourcesConfiguration.src3,
			.s3.sink 	= CurrentSourcesConfiguration.sink3,
			.s4.src 	= CurrentSourcesConfiguration.src4,
			.s4.sink 	= CurrentSourcesConfiguration.sink4,
			.s5.src 	= CurrentSourcesConfiguration.src5,
			.s5.sink 	= CurrentSourcesConfiguration.sink5,
			.s6.src 	= CurrentSourcesConfiguration.src6,
			.s6.sink 	= CurrentSourcesConfiguration.sink6,
			.s7.src 	= CurrentSourcesConfiguration.src7,
			.s7.sink 	= CurrentSourcesConfiguration.sink7,
			.s8.src 	= CurrentSourcesConfiguration.src8,
			.s8.sink 	= CurrentSourcesConfiguration.sink8,
			.s17.src 	= CurrentSourcesConfiguration.src17,
			.s17.sink 	= CurrentSourcesConfiguration.sink17,
	};
	app_func_stim_circuit_para_set(parameters);
	app_func_stim_curr_src_set(config);
	app_func_stim_stimulus_start(isPh1, !isPh1, true);
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
			app_func_stim_vdda_sup_enable(TestInformation.vddaSupplyEnable);
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
		len_payload = 8;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			StimulusCircuitParameters_t stimuluscircuitparameters;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&stimuluscircuitparameters.pulseWidth_us, sizeof(stimuluscircuitparameters.pulseWidth_us));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&stimuluscircuitparameters.pulseNumber_ms, sizeof(stimuluscircuitparameters.pulseNumber_ms));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&stimuluscircuitparameters.pulsePeriod_us, sizeof(stimuluscircuitparameters.pulsePeriod_us));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)stimuluscircuitparameters.reserved, sizeof(stimuluscircuitparameters.reserved));

			memcpy(&StimulusCircuitParameters, &stimuluscircuitparameters, sizeof(StimulusCircuitParameters_t));
			Stimulus_Waveform_t parameters = {
					.pulseWidth_us 			= StimulusCircuitParameters.pulseWidth_us,
					.pulsePeriod_us 		= StimulusCircuitParameters.pulsePeriod_us,
					.trainOnDuration_ms 	= StimulusCircuitParameters.pulseNumber_ms,
					.trainOffDuration_ms 	= StimulusCircuitParameters.pulseNumber_ms,
			};
			app_func_stim_circuit_para_set(parameters);
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
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&StimulusCircuitParameters.pulseWidth_us, sizeof(StimulusCircuitParameters.pulseWidth_us));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&StimulusCircuitParameters.pulseNumber_ms, sizeof(StimulusCircuitParameters.pulseNumber_ms));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&StimulusCircuitParameters.pulsePeriod_us, sizeof(StimulusCircuitParameters.pulsePeriod_us));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)StimulusCircuitParameters.reserved, sizeof(StimulusCircuitParameters.reserved));

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

			if (app_func_stim_dac_init(DACTYPE_STIM) != HAL_OK) {
				resp.Status = STATUS_INVALID;
			}

			if (app_func_stim_dac_volt_set(dacaboutputvoltage.aDacOutputVoltage_mv, dacaboutputvoltage.bDacOutputVoltage_mv) != HAL_OK) {
				resp.Status = STATUS_INVALID;
			}
		}
	}
		break;

	case OP_GENERATE_PHI_ONE_SINGLE_PULSE:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			generatePhiSinglePulse(true);
		}
	}
		break;

	case OP_GENERATE_PHI_TWO_SINGLE_PULSE:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			generatePhiSinglePulse(false);
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
		len_payload = 18;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			CurrentSourcesConfiguration_t currentsourcesconfiguration;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&currentsourcesconfiguration.src1, sizeof(currentsourcesconfiguration.src1));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&currentsourcesconfiguration.sink1, sizeof(currentsourcesconfiguration.sink1));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&currentsourcesconfiguration.src2, sizeof(currentsourcesconfiguration.src2));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&currentsourcesconfiguration.sink2, sizeof(currentsourcesconfiguration.sink2));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&currentsourcesconfiguration.src3, sizeof(currentsourcesconfiguration.src3));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&currentsourcesconfiguration.sink3, sizeof(currentsourcesconfiguration.sink3));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&currentsourcesconfiguration.src4, sizeof(currentsourcesconfiguration.src4));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&currentsourcesconfiguration.sink4, sizeof(currentsourcesconfiguration.sink4));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&currentsourcesconfiguration.src5, sizeof(currentsourcesconfiguration.src5));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&currentsourcesconfiguration.sink5, sizeof(currentsourcesconfiguration.sink5));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&currentsourcesconfiguration.src6, sizeof(currentsourcesconfiguration.src6));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&currentsourcesconfiguration.sink6, sizeof(currentsourcesconfiguration.sink6));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&currentsourcesconfiguration.src7, sizeof(currentsourcesconfiguration.src7));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&currentsourcesconfiguration.sink7, sizeof(currentsourcesconfiguration.sink7));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&currentsourcesconfiguration.src8, sizeof(currentsourcesconfiguration.src8));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&currentsourcesconfiguration.sink8, sizeof(currentsourcesconfiguration.sink8));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&currentsourcesconfiguration.src17, sizeof(currentsourcesconfiguration.src17));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&currentsourcesconfiguration.sink17, sizeof(currentsourcesconfiguration.sink17));

			memcpy(&CurrentSourcesConfiguration, &currentsourcesconfiguration, sizeof(CurrentSourcesConfiguration_t));
			Current_Sources_t config = {
					.s1.src 	= CurrentSourcesConfiguration.src1,
					.s1.sink 	= CurrentSourcesConfiguration.sink1,
					.s2.src 	= CurrentSourcesConfiguration.src2,
					.s2.sink 	= CurrentSourcesConfiguration.sink2,
					.s3.src 	= CurrentSourcesConfiguration.src3,
					.s3.sink 	= CurrentSourcesConfiguration.sink3,
					.s4.src 	= CurrentSourcesConfiguration.src4,
					.s4.sink 	= CurrentSourcesConfiguration.sink4,
					.s5.src 	= CurrentSourcesConfiguration.src5,
					.s5.sink 	= CurrentSourcesConfiguration.sink5,
					.s6.src 	= CurrentSourcesConfiguration.src6,
					.s6.sink 	= CurrentSourcesConfiguration.sink6,
					.s7.src 	= CurrentSourcesConfiguration.src7,
					.s7.sink 	= CurrentSourcesConfiguration.sink7,
					.s8.src 	= CurrentSourcesConfiguration.src8,
					.s8.sink 	= CurrentSourcesConfiguration.sink8,
					.s17.src 	= CurrentSourcesConfiguration.src17,
					.s17.sink 	= CurrentSourcesConfiguration.sink17,
			};
			app_func_stim_curr_src_set(config);
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
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&CurrentSourcesConfiguration.sink1, sizeof(CurrentSourcesConfiguration.sink1));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&CurrentSourcesConfiguration.src2, sizeof(CurrentSourcesConfiguration.src2));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&CurrentSourcesConfiguration.sink2, sizeof(CurrentSourcesConfiguration.sink2));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&CurrentSourcesConfiguration.src3, sizeof(CurrentSourcesConfiguration.src3));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&CurrentSourcesConfiguration.sink3, sizeof(CurrentSourcesConfiguration.sink3));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&CurrentSourcesConfiguration.src4, sizeof(CurrentSourcesConfiguration.src4));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&CurrentSourcesConfiguration.sink4, sizeof(CurrentSourcesConfiguration.sink4));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&CurrentSourcesConfiguration.src5, sizeof(CurrentSourcesConfiguration.src5));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&CurrentSourcesConfiguration.sink5, sizeof(CurrentSourcesConfiguration.sink5));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&CurrentSourcesConfiguration.src6, sizeof(CurrentSourcesConfiguration.src6));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&CurrentSourcesConfiguration.sink6, sizeof(CurrentSourcesConfiguration.sink6));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&CurrentSourcesConfiguration.src7, sizeof(CurrentSourcesConfiguration.src7));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&CurrentSourcesConfiguration.sink7, sizeof(CurrentSourcesConfiguration.sink7));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&CurrentSourcesConfiguration.src8, sizeof(CurrentSourcesConfiguration.src8));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&CurrentSourcesConfiguration.sink8, sizeof(CurrentSourcesConfiguration.sink8));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&CurrentSourcesConfiguration.src17, sizeof(CurrentSourcesConfiguration.src17));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&CurrentSourcesConfiguration.sink17, sizeof(CurrentSourcesConfiguration.sink17));

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
			Stimulus_Waveform_t parameters = {
					.pulseWidth_us 			= StimulusCircuitParameters.pulseWidth_us,
					.pulsePeriod_us 		= StimulusCircuitParameters.pulsePeriod_us,
					.trainOnDuration_ms 	= StimulusCircuitParameters.pulseNumber_ms,
					.trainOffDuration_ms 	= StimulusCircuitParameters.pulseNumber_ms,
			};
			Current_Sources_t config = {
					.s1.src 	= CurrentSourcesConfiguration.src1,
					.s1.sink 	= CurrentSourcesConfiguration.sink1,
					.s2.src 	= CurrentSourcesConfiguration.src2,
					.s2.sink 	= CurrentSourcesConfiguration.sink2,
					.s3.src 	= CurrentSourcesConfiguration.src3,
					.s3.sink 	= CurrentSourcesConfiguration.sink3,
					.s4.src 	= CurrentSourcesConfiguration.src4,
					.s4.sink 	= CurrentSourcesConfiguration.sink4,
					.s5.src 	= CurrentSourcesConfiguration.src5,
					.s5.sink 	= CurrentSourcesConfiguration.sink5,
					.s6.src 	= CurrentSourcesConfiguration.src6,
					.s6.sink 	= CurrentSourcesConfiguration.sink6,
					.s7.src 	= CurrentSourcesConfiguration.src7,
					.s7.sink 	= CurrentSourcesConfiguration.sink7,
					.s8.src 	= CurrentSourcesConfiguration.src8,
					.s8.sink 	= CurrentSourcesConfiguration.sink8,
					.s17.src 	= CurrentSourcesConfiguration.src17,
					.s17.sink 	= CurrentSourcesConfiguration.sink17,
			};
			app_func_stim_circuit_para_set(parameters);
			app_func_stim_curr_src_set(config);
			app_func_stim_stimulus_start(true, true, false);
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
				bsp_mux_enable(true);
		}
	}
		break;

	case OP_SET_SR_SEL_POSITIONS:
	{
		len_payload = 3;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			SrSelPositions_t srselpositions;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&srselpositions.sr_sel1, sizeof(srselpositions.sr_sel1));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&srselpositions.sr_sel2, sizeof(srselpositions.sr_sel2));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&srselpositions.ecg_sel, sizeof(srselpositions.ecg_sel));

			memcpy(&SrSelPositions, &srselpositions, sizeof(SrSelPositions_t));
			bsp_mux_sr_sel_set(SrSelPositions.sr_sel1, SrSelPositions.sr_sel2);
			bsp_mux_ecg_sel_set(SrSelPositions.ecg_sel);
		}
	}
		break;

	case OP_GET_SR_SEL_POSITIONS:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			uint8_t resp_payload[sizeof(SrSelPositions)];
			uint8_t* payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&SrSelPositions.sr_sel1, sizeof(SrSelPositions.sr_sel1));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&SrSelPositions.sr_sel2, sizeof(SrSelPositions.sr_sel2));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&SrSelPositions.ecg_sel, sizeof(SrSelPositions.ecg_sel));

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
					.det = HAL_GPIO_ReadPin(VRECT_DET_GPIO_Port, VRECT_DET_Pin),
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
					.ovp = HAL_GPIO_ReadPin(VRECT_OVP_GPIO_Port, VRECT_OVP_Pin),
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
					.ovpErr = HAL_GPIO_ReadPin(CHG1_OVP_ERRn_GPIO_Port, CHG1_OVP_ERRn_Pin),
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
					.ovpErr = HAL_GPIO_ReadPin(CHG2_OVP_ERRn_GPIO_Port, CHG2_OVP_ERRn_Pin),
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
			ThermOfstReq_t thermofstreq;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&thermofstreq.bufferSize, sizeof(thermofstreq.bufferSize));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&thermofstreq.samplingFrequency_hz, sizeof(thermofstreq.samplingFrequency_hz));

			ThermOfstResp_t thermofstresp;
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

	case OP_DISABLE_EMG1_AFE:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			app_func_meas_sensor_enable(SENSOR_ID_EMG1, false);
		}
	}
		break;

	case OP_DISABLE_EMG2_AFE:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			app_func_meas_sensor_enable(SENSOR_ID_EMG2, false);
		}
	}
		break;

	case OP_ENABLE_EMG1_AFE:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			app_func_meas_sensor_enable(SENSOR_ID_EMG1, true);
		}
	}
		break;

	case OP_ENABLE_EMG2_AFE:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			app_func_meas_sensor_enable(SENSOR_ID_EMG2, true);
		}
	}
		break;

	case OP_GET_EMG1_LOD:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			Emg1Lod_t emg1lod = {
					.emg1Lod = HAL_GPIO_ReadPin(EMG1_LOD_GPIO_Port, EMG1_LOD_Pin),
			};
			uint8_t resp_payload[sizeof(emg1lod)];
			uint8_t* payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&emg1lod.emg1Lod, sizeof(emg1lod.emg1Lod));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_GET_EMG2_LOD:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			Emg2Lod_t emg2lod = {
					.emg2Lod = HAL_GPIO_ReadPin(EMG2_LOD_GPIO_Port, EMG2_LOD_Pin),
			};
			uint8_t resp_payload[sizeof(emg2lod)];
			uint8_t* payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&emg2lod.emg2Lod, sizeof(emg2lod.emg2Lod));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_GET_EMG1_AFE_OUTPUT:
	{
		len_payload = 3;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			Emg1AfeOutputReq_t emg1afeoutputreq;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&emg1afeoutputreq.bufferSize, sizeof(emg1afeoutputreq.bufferSize));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&emg1afeoutputreq.samplingFrequency_hz, sizeof(emg1afeoutputreq.samplingFrequency_hz));

			Emg1AfeOutputResp_t emg1afeoutputresp;
			emg1afeoutputresp.bufferSize = emg1afeoutputreq.bufferSize;
			app_func_meas_sensor_meas(SENSOR_ID_EMG1, emg1afeoutputresp.buffer, emg1afeoutputresp.bufferSize, emg1afeoutputreq.samplingFrequency_hz);

			uint8_t resp_payload[sizeof(emg1afeoutputresp)];
			payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&emg1afeoutputresp.bufferSize, sizeof(emg1afeoutputresp.bufferSize));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)emg1afeoutputresp.buffer, sizeof(emg1afeoutputresp.buffer));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_GET_EMG2_AFE_OUTPUT:
	{
		len_payload = 3;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			Emg2AfeOutputReq_t emg2afeoutputreq;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&emg2afeoutputreq.bufferSize, sizeof(emg2afeoutputreq.bufferSize));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&emg2afeoutputreq.samplingFrequency_hz, sizeof(emg2afeoutputreq.samplingFrequency_hz));

			Emg2AfeOutputResp_t emg2afeoutputresp;
			emg2afeoutputresp.bufferSize = emg2afeoutputreq.bufferSize;
			app_func_meas_sensor_meas(SENSOR_ID_EMG2, emg2afeoutputresp.buffer, emg2afeoutputresp.bufferSize, emg2afeoutputreq.samplingFrequency_hz);

			uint8_t resp_payload[sizeof(emg2afeoutputresp)];
			payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&emg2afeoutputresp.bufferSize, sizeof(emg2afeoutputresp.bufferSize));
			payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)emg2afeoutputresp.buffer, sizeof(emg2afeoutputresp.buffer));

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
		}
	}
		break;

	case OP_SET_IMP_SEL_POSITIONS:
	{
		len_payload = 8;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			ImpSelPositions_t impselpositions;
			uint8_t* payload_offset = req.Payload;
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&impselpositions.impa_sel0, sizeof(impselpositions.impa_sel0));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&impselpositions.impa_sel1, sizeof(impselpositions.impa_sel1));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&impselpositions.impa_sel2, sizeof(impselpositions.impa_sel2));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&impselpositions.impa_sel3, sizeof(impselpositions.impa_sel3));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&impselpositions.impb_sel0, sizeof(impselpositions.impb_sel0));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&impselpositions.impb_sel1, sizeof(impselpositions.impb_sel1));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&impselpositions.impb_sel2, sizeof(impselpositions.impb_sel2));
			payload_offset = copyPayloadToStructField (payload_offset, (uint8_t*)&impselpositions.impb_sel3, sizeof(impselpositions.impb_sel3));

			memcpy(&ImpSelPositions, &impselpositions, sizeof(ImpSelPositions_t));
			app_func_meas_imp_sel_set(	ImpSelPositions.impa_sel0,
										ImpSelPositions.impa_sel1,
										ImpSelPositions.impa_sel2,
										ImpSelPositions.impa_sel3,
										ImpSelPositions.impb_sel0,
										ImpSelPositions.impb_sel1,
										ImpSelPositions.impb_sel2,
										ImpSelPositions.impb_sel3	);
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
			payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&ImpSelPositions.impa_sel0, sizeof(ImpSelPositions.impa_sel0));
			payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&ImpSelPositions.impa_sel1, sizeof(ImpSelPositions.impa_sel1));
			payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&ImpSelPositions.impa_sel2, sizeof(ImpSelPositions.impa_sel2));
			payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&ImpSelPositions.impa_sel3, sizeof(ImpSelPositions.impa_sel3));
			payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&ImpSelPositions.impb_sel0, sizeof(ImpSelPositions.impb_sel0));
			payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&ImpSelPositions.impb_sel1, sizeof(ImpSelPositions.impb_sel1));
			payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&ImpSelPositions.impb_sel2, sizeof(ImpSelPositions.impb_sel2));
			payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&ImpSelPositions.impb_sel3, sizeof(ImpSelPositions.impb_sel3));

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
			const Stimulus_Waveform_t parameters = {
					.pulseWidth_us 			= 500,
					.pulsePeriod_us 		= 4000,
					.trainOnDuration_ms 	= 1000,
					.trainOffDuration_ms	= 0,
			};

			Current_Sources_t configuration = configureCurrentSources(ImpSelPositions);

			const _Float64 pulse_amplitude_mA = 1;
			uint16_t dacVoltage_mv = (uint16_t)(pulse_amplitude_mA * BSP_PULSE_AMP_RATIO);

			const uint16_t samplingFrequency_hz = 20000;
			const uint16_t samplingPoints = 80;
			const uint8_t samplingPeriods = 10;
			static uint16_t impVoltageBuffer[80*10];

			HAL_ERROR_CHECK(app_func_stim_dac_init(DACTYPE_STIM));
			HAL_ERROR_CHECK(app_func_stim_dac_volt_set(dacVoltage_mv, dacVoltage_mv));
			HAL_Delay(10);

			app_func_stim_circuit_para_set(parameters);
			app_func_stim_curr_src_set(configuration);
			app_func_stim_stimulus_start(true, true, false);
			app_func_meas_imp_volt_meas(impVoltageBuffer, samplingPoints*samplingPeriods, samplingFrequency_hz);

			uint16_t maxVoltage[10] = {0};
			uint16_t avgMaxvoltage = 0;
			for(uint8_t period = 0;period<samplingPeriods;period++) {
				for(uint16_t point=0;point<samplingPoints;point++) {
					maxVoltage[period] = (maxVoltage[period] > impVoltageBuffer[period * samplingPoints + point])?maxVoltage[period]:impVoltageBuffer[period * samplingPoints + point];
				}
				avgMaxvoltage += maxVoltage[period];
			}
			avgMaxvoltage /= samplingPeriods;

			_Float64 impedance = (_Float64)avgMaxvoltage / pulse_amplitude_mA;

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
			app_func_stim_vdda_sup_enable(TestInformation.vddaSupplyEnable);
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
			uint8_t mode = (freqModedeviceID.ModeDeviceID & 0x80) >> 7;
			uint8_t deviceAddr = (freqModedeviceID.ModeDeviceID & 0x7F) << 1;
			if (	freqModedeviceID.FreqSampling != 10U &&
					freqModedeviceID.FreqSampling != 25U &&
					freqModedeviceID.FreqSampling != 50U &&
					freqModedeviceID.FreqSampling != 100U &&
					freqModedeviceID.FreqSampling != 200U) {
				resp.Status = STATUS_START_ACC_INVALID_CONFIGURATION;
			}
			else if (	deviceAddr != MIS2DHTR_DEVICE_ADDR_L &&
						deviceAddr != MIS2DHTR_DEVICE_ADDR_H) {
				resp.Status = STATUS_START_ACC_INVALID_CONFIGURATION;
			}
			else {
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
									resp.Status = STATUS_START_ACC_COMMUNICTION_ERR;
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
											.DeviceID = freqModedeviceID.ModeDeviceID & 0b01111111,
									};
									memcpy((uint8_t*)&AccMeasure.XYZ, (uint8_t*)&acc[i].Device.Fifo.Axis[count - 1U], sizeof(XYZ_t));

									uint8_t resp_payload[sizeof(AccMeasure_t)];
									uint8_t* payload_offset = resp_payload;
									payload_offset = copyStructFieldToPayload(payload_offset, (uint8_t*)&AccMeasure.DeviceID, sizeof(AccMeasure.DeviceID));
									payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&AccMeasure.XYZ.X, sizeof(AccMeasure.XYZ.X));
									payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&AccMeasure.XYZ.Y, sizeof(AccMeasure.XYZ.Y));
									payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&AccMeasure.XYZ.Z, sizeof(AccMeasure.XYZ.Z));

									resp.PayloadLen = payload_offset - resp_payload;
									resp.Payload = resp_payload;
								}
							}
							else {
								resp.Status = STATUS_START_ACC_DEVICE_NOT_READY;
							}
						}
						else {
							resp.Status = STATUS_START_ACC_DEVICE_NOT_READY;
						}
					}
				}
			}
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
			uint8_t deviceAddr = deviceID.DeviceID << 1;
			if (deviceAddr != MIS2DHTR_DEVICE_ADDR_L &&
				deviceAddr != MIS2DHTR_DEVICE_ADDR_H) {
				resp.Status = STATUS_GET_ACC_COMMUNICTION_ERR;
			}
			else {
				for (int i = 0;i < 2;i++) {
					if (acc[i].Device.DeviceAddress == deviceAddr) {
						if (acc[i].Startup == false) {
							resp.Status = STATUS_GET_ACC_STARTUP_NOT_PERFORMED;
						}
						else if (acc[i].Mode == ON_DEMAND) {
							acc[i].Device.RegisterAddress = MIS2DHTR_FIFO_SRC_REG;
							HAL_StatusTypeDef status = bsp_sp_MIS2DHTR_read(acc[i].Device.DeviceAddress, acc[i].Device.RegisterAddress, &acc[i].Device.Fifo.SrcRegister, 1);
							if (status != HAL_OK) {
								resp.Status = STATUS_GET_ACC_COMMUNICTION_ERR;
							}
							else if (MIS2DHTR_FIFO_IS_EMPTY(acc[i].Device.Fifo.SrcRegister)) {
								resp.Status = STATUS_GET_ACC_BUFFER_EMPTY;
							}
							else {
								if (MIS2DHTR_FIFO_IS_OVRN(acc[i].Device.Fifo.SrcRegister)) {
									resp.Status = STATUS_GET_ACC_BUFFER_OVERFLOW;
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

								uint8_t resp_payload[sizeof(InfoData)];
								payload_offset = resp_payload;
								payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&InfoData.DeviceID, sizeof(InfoData.DeviceID));
								payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&InfoData.Timestamp, sizeof(InfoData.Timestamp));
								payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&InfoData.SampleIndex, sizeof(InfoData.SampleIndex));
								for (int j = 0;j < count;j++) {
									payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&InfoData.XYZ[j].X, sizeof(InfoData.XYZ[j].X));
									payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&InfoData.XYZ[j].Y, sizeof(InfoData.XYZ[j].Y));
									payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&InfoData.XYZ[j].Z, sizeof(InfoData.XYZ[j].Z));
								}
								resp.PayloadLen = payload_offset - resp_payload;
								resp.Payload = resp_payload;
							}
						}
						else if (acc[i].Mode == BUFFER_STORAGE) {
							acc[i].WriteSuspend = true;
							if (acc[i].Overflow == false && acc[i].ReadIndex == acc[i].WriteIndex) {
								resp.Status = STATUS_GET_ACC_BUFFER_EMPTY;
							}
							else {
								if (acc[i].Overflow) {
									resp.Status = STATUS_GET_ACC_BUFFER_OVERFLOW;
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

								uint8_t resp_payload[sizeof(InfoData)];
								payload_offset = resp_payload;
								payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&InfoData.DeviceID, sizeof(InfoData.DeviceID));
								payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&InfoData.Timestamp, sizeof(InfoData.Timestamp));
								payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&InfoData.SampleIndex, sizeof(InfoData.SampleIndex));
								for (int j = 0;j < count;j++) {
									payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&InfoData.XYZ[j].X, sizeof(InfoData.XYZ[j].X));
									payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&InfoData.XYZ[j].Y, sizeof(InfoData.XYZ[j].Y));
									payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&InfoData.XYZ[j].Z, sizeof(InfoData.XYZ[j].Z));
								}
								resp.PayloadLen = payload_offset - resp_payload;
								resp.Payload = resp_payload;
							}
							acc[i].WriteSuspend = false;
						}
					}
				}
			}
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
			uint8_t deviceAddr = deviceID.DeviceID << 1;
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
			uint8_t resp_payload[sizeof(deviceID)];
			payload_offset = resp_payload;
			payload_offset = copyStructFieldToPayload (payload_offset, (uint8_t*)&deviceID.DeviceID, sizeof(deviceID.DeviceID));

			resp.PayloadLen = payload_offset - resp_payload;
			resp.Payload = resp_payload;
			if (acc[0].ChannelsOpen == false && acc[1].ChannelsOpen == false) {
				bsp_sp_XL_enable(false);
			}
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
		.bleid = {
				0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,
		},
};

static HAL_StatusTypeDef setBitFromGpioState(uint8_t *buffer, size_t buffer_size, size_t bit_index, const GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    if (!buffer || !GPIOx) {
        return HAL_ERROR; // Null pointer error
    }

    size_t byte_index = bit_index / 8; // Calculate the byte index
    size_t bit_position = bit_index % 8; // Calculate the bit position within the byte

    if (byte_index >= buffer_size) {
        return HAL_ERROR; // Out of range error
    }

    GPIO_PinState state = HAL_GPIO_ReadPin(GPIOx, GPIO_Pin); // Read the GPIO pin state

    if (state == GPIO_PIN_SET) {
        buffer[byte_index] |= (1 << bit_position); // Set the bit to 1
    } else {
        buffer[byte_index] &= ~(1 << bit_position); // Clear the bit to 0
    }

    return HAL_OK; // Success
}

static BLE_ADV_Setting_t* ble_adv_update(void) {
#ifdef SINE
	setting.adv_timeout[0] = 60;
	return &setting;
#endif
	static uint16_t dvdd_div4;
	static uint16_t batt[2];
	static uint16_t imp[2];
	static uint16_t threm[3];

	uint32_t vref = bsp_adc_vref_get(SYSCFG_VREFBUF_VOLTAGE_SCALE3);
	bsp_adc_single_sampling(HANDLE_ID_ADC1, ADC1_CHANNEL_DVDD, &dvdd_div4, 1, 1000, vref);
	app_func_meas_batt_mon_meas(&batt[0], &batt[1]);
	bsp_adc_single_sampling(HANDLE_ID_ADC4, ADC4_CHANNEL_IMP_INA, &imp[0], 1, 1000, vref);
	bsp_adc_single_sampling(HANDLE_ID_ADC4, ADC4_CHANNEL_IMP_INB, &imp[1], 1, 1000, vref);
	app_func_meas_therm_meas(THERM_ID_REF, (uint8_t*)&threm[0], sizeof(uint16_t), 1000);
	app_func_meas_therm_meas(THERM_ID_OUT, (uint8_t*)&threm[1], sizeof(uint16_t), 1000);
	app_func_meas_therm_meas(THERM_ID_OFST, (uint8_t*)&threm[2], sizeof(uint16_t), 1000);

	uint8_t dvdd_100mv = dvdd_div4 * 4 / 100;
	uint8_t battA_100mv = batt[0] / 100;
	uint8_t battB_100mv = batt[1] / 100;
	uint8_t impA_10mv = imp[0] / 10;
	uint8_t impB_10mv = imp[1] / 10;
	uint8_t thremRef_10mv = threm[0] / 10;
	uint8_t thremOut_10mv = threm[1] / 10;
	uint8_t thremOfst_10mv = threm[2] / 10;

	uint8_t* payload_offset = setting.bleid;
	payload_offset = copyStructFieldToPayload(payload_offset, &dvdd_100mv, sizeof(uint8_t));
	payload_offset = copyStructFieldToPayload(payload_offset, &battA_100mv, sizeof(uint8_t));
	payload_offset = copyStructFieldToPayload(payload_offset, &battB_100mv, sizeof(uint8_t));
	payload_offset = copyStructFieldToPayload(payload_offset, &impB_10mv, sizeof(uint8_t));
	payload_offset = copyStructFieldToPayload(payload_offset, &impA_10mv, sizeof(uint8_t));
	payload_offset = copyStructFieldToPayload(payload_offset, &thremRef_10mv, sizeof(uint8_t));
	payload_offset = copyStructFieldToPayload(payload_offset, &thremOut_10mv, sizeof(uint8_t));
	payload_offset = copyStructFieldToPayload(payload_offset, &thremOfst_10mv, sizeof(uint8_t));

	uint8_t payload_freesize = LEN_BLE_ID_MAX - ((uint8_t)(payload_offset - setting.bleid));
	HAL_ERROR_CHECK(setBitFromGpioState(payload_offset, payload_freesize, 0, 	VRECT_DET_GPIO_Port, VRECT_DET_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(payload_offset, payload_freesize, 1, 	VRECT_OVP_GPIO_Port, VRECT_OVP_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(payload_offset, payload_freesize, 2, 	VCHG_PGOOD_GPIO_Port, VCHG_PGOOD_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(payload_offset, payload_freesize, 3, 	CHG1_STATUS_GPIO_Port, CHG1_STATUS_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(payload_offset, payload_freesize, 4, 	CHG1_OVP_ERRn_GPIO_Port, CHG1_OVP_ERRn_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(payload_offset, payload_freesize, 5, 	CHG2_STATUS_GPIO_Port, CHG2_STATUS_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(payload_offset, payload_freesize, 6, 	CHG2_OVP_ERRn_GPIO_Port, CHG2_OVP_ERRn_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(payload_offset, payload_freesize, 7, 	EMG2_SDNn_GPIO_Port, EMG2_SDNn_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(payload_offset, payload_freesize, 8, 	EMG1_SDNn_GPIO_Port, EMG1_SDNn_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(payload_offset, payload_freesize, 9, 	ECG_RLD_GPIO_Port, ECG_RLD_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(payload_offset, payload_freesize, 10, 	ECG_HR_SDNn_GPIO_Port, ECG_HR_SDNn_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(payload_offset, payload_freesize, 11, 	ECG_RR_SDNn_GPIO_Port, ECG_RR_SDNn_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(payload_offset, payload_freesize, 12, 	TEMP_EN_GPIO_Port, TEMP_EN_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(payload_offset, payload_freesize, 13, 	IMP_EN_GPIO_Port, IMP_EN_Pin));

	return &setting;
}

static void app_mode_dvt_init(void) {
	TestInformation.hvSupplyEnable 		= HAL_GPIO_ReadPin(VPPSW_EN_GPIO_Port, VPPSW_EN_Pin);
	TestInformation.vddsSupplyEnable 	= HAL_GPIO_ReadPin(VDDS_EN_GPIO_Port, VDDS_EN_Pin);
	TestInformation.vddaSupplyEnable 	= HAL_GPIO_ReadPin(VDDA_EN_GPIO_Port, VDDA_EN_Pin);

	CurrentSourcesConfiguration.src1 	= HAL_GPIO_ReadPin(SRC1_GPIO_Port, SRC1_Pin);
	CurrentSourcesConfiguration.sink1 	= HAL_GPIO_ReadPin(SNK1_GPIO_Port, SNK1_Pin);
	CurrentSourcesConfiguration.src2 	= HAL_GPIO_ReadPin(SRC2_GPIO_Port, SRC2_Pin);
	CurrentSourcesConfiguration.sink2 	= HAL_GPIO_ReadPin(SNK2_GPIO_Port, SNK2_Pin);
	CurrentSourcesConfiguration.src3 	= HAL_GPIO_ReadPin(SRC3_GPIO_Port, SRC3_Pin);
	CurrentSourcesConfiguration.sink3 	= HAL_GPIO_ReadPin(SNK3_GPIO_Port, SNK3_Pin);
	CurrentSourcesConfiguration.sink17 = HAL_GPIO_ReadPin(SRC17_GPIO_Port, SRC17_Pin);
	CurrentSourcesConfiguration.sink17 = HAL_GPIO_ReadPin(SNK17_GPIO_Port, SNK17_Pin);

	SrSelPositions.sr_sel1 = HAL_GPIO_ReadPin(SR_SEL1_GPIO_Port, SR_SEL1_Pin);
	SrSelPositions.sr_sel2 = HAL_GPIO_ReadPin(SR_SEL2_GPIO_Port, SR_SEL2_Pin);
}

#ifdef SINE
#define OP_SET_SINE   0xE0U
#define OP_GET_SINE   0xE1U

static _Float64 sineFrequency_Hz = 1;
static _Float64 sineAmplitude_mA = 0;
static uint8_t sineAmplitude_step;
static uint8_t sineFrequency_step;

static void sineWave_init(void) {
	HAL_ERROR_CHECK(app_func_stim_hv_sup_volt_set((uint16_t)HV_SUPPLY_MV));
	HAL_Delay(10);
	app_func_stim_vdds_sup_enable(true);
	app_func_stim_stimulus_enable(true);
	app_func_stim_stimulus_mux_enable();
	HAL_Delay(10);
	HAL_ERROR_CHECK(app_func_stim_dac_init(DACTYPE_SINE));
	HAL_Delay(10);
}

static void sineWave_stop(void) {
	app_func_stim_hv_supply_set(false, true);
	app_func_stim_sine_stop();
	app_func_stim_ph_out_stop();
}

static void sineWave_start(void) {
	_Float64 sine_period_us = 1000000.0 / sineFrequency_Hz;
	_Float64 sineAmplitude_mV = sineAmplitude_mA * BSP_SINE_AMP_RATIO;

	NerveBlock_Waveform_t sine_parameters = {
			.sinePeriod_us 		= (uint32_t)sine_period_us,
			.sinePhaseShift_us 	= 0,
			.dacUpdateRate_Hz 	= 2000,	//1000~1000000
			.amplitude_mV 		= (uint16_t)sineAmplitude_mV,
	};
	app_func_stim_sine_start(sine_parameters);
	app_func_stim_ph_out_force();
	app_func_stim_hv_supply_set(true, true);
}

static Cmd_Resp_t sineWave_command_req_parser(Cmd_Req_t req) {
	Cmd_Resp_t resp = {
			.Opcode 		= req.Opcode,
			.Status 		= STATUS_SUCCESS,
			.Payload 		= NULL,
			.PayloadLen 	= 0,
	};
	uint8_t len_payload;

	switch(req.Opcode) {
	case OP_SET_SINE:
	{
		len_payload = 2;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			sineAmplitude_step = req.Payload[0];
			sineFrequency_step = req.Payload[1];
			if (sineAmplitude_step == 0) {
				sineAmplitude_mA = 0;
				sineWave_stop();
			}
			else if (sineAmplitude_step >= 1 && sineAmplitude_step <= 10 &&
					sineFrequency_step >= 1 && sineFrequency_step <= 10) {
				sineAmplitude_mA = sineAmplitude_step * 0.5;
				sineFrequency_Hz = sineFrequency_step * 1.0;
				sineWave_stop();
				sineWave_start();
			}
			else {
				resp.Status = STATUS_INVALID;
			}
		}
	}
		break;

	case OP_GET_SINE:
	{
		len_payload = 0;
		if (req.PayloadLen != len_payload) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			sineAmplitude_step = sineAmplitude_mA / 0.5;
			sineFrequency_step = sineFrequency_Hz / 1.0;
			resp.PayloadLen = 2;
			resp.Payload[0] = sineAmplitude_step;
			resp.Payload[1] = sineFrequency_step;
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
#endif

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

#ifdef SINE
	app_func_command_req_parser_set(&sineWave_command_req_parser);
	sineWave_init();
	sineWave_start();
#endif

	while(1) {
		if (curr_ble_state == BLE_STATE_ADV_STOP) {
			if (!ble_peers_del) {
				app_func_ble_peers_del();
				ble_peers_del = true;
			}
			else {
				app_func_ble_adv_start(ble_adv_update(), LEN_BLE_ID_MAX);
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
 * @brief Callback for the timer in DVT mode, unit: ms
 *
 */
void app_mode_dvt_ms_timer_cb(void) {
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
