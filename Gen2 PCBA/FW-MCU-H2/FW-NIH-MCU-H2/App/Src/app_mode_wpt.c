/**
 * @file app_mode_wpt.c
 * @brief Wireless Power Transfer (WPT) charging state handler
 *
 * Implements the autonomous charging state machine defined in CAR-212.
 * Manages two Li-ion batteries via the CHGx_EN/STATUS/OVP_ERRn signals,
 * monitors temperature via the 104AP-2 NTC thermistor, and advertises
 * charging status over BLE.
 *
 * @copyright Copyright (c) 2024
 */
#include "app_mode_wpt.h"
#include "app_config.h"

/* ---- Module-level state ---- */

static bool    battery_a_present   = false;
static bool    battery_b_present   = false;
static uint8_t battery_a_low_count = 0U;
static uint8_t battery_b_low_count = 0U;

static uint32_t wpt_ms_timer       = 0U;   /*!< Counts down to 0 for 1-second sample ticks */
static uint32_t wpt_paused_hold_ms = 0U;   /*!< Minimum hold time in PAUSED after an OVP event */

/* ---- Thermistor lookup table (104AP-2 NTC, from charger firmware svc_wpt_manager.h) ---- */

typedef struct {
    int16_t  temp_c;
    uint32_t resistance_ohm;
} ThermEntry_t;

static const ThermEntry_t k_therm_table[] = {
    {20, 126400U},
    {25, 100000U},
    {30,  79590U},
    {40,  51320U},
    {50,  33790U},
};

#define THERM_TABLE_SIZE  ((uint8_t)(sizeof(k_therm_table) / sizeof(k_therm_table[0])))

/**
 * @brief Calculate thermistor temperature using the 104AP-2 NTC curve.
 *
 * Ported from OpenNerve-Charger CalculateTemperatureFromBle().
 * All voltage inputs in mV (as returned by app_func_meas_therm_meas).
 *
 * @param therm_ref_mv   THERM_REF ADC reading, mV
 * @param therm_out_mv   THERM_OUT ADC reading, mV
 * @param therm_ofst_mv  THERM_OFST ADC reading, mV
 * @return float         Calculated temperature in °C
 */
static float app_mode_wpt_calc_temperature(uint16_t therm_ref_mv, uint16_t therm_out_mv, uint16_t therm_ofst_mv)
{
    float voltage      = (float)therm_out_mv  - (float)therm_ofst_mv;  /* across thermistor */
    float voltage_drop = (float)therm_ref_mv  - (float)therm_out_mv;   /* across 49.9kΩ sense resistor */

    if (voltage_drop <= 0.0f || voltage <= 0.0f) {
        return 0.0f;  /* invalid / unpowered — treat as cold */
    }

    float current    = voltage_drop / WPT_THERM_SENSE_RESISTOR_OHM;
    float resistance = voltage / current;

    /* Clamp to table bounds */
    if (resistance >= (float)k_therm_table[0].resistance_ohm) {
        return (float)k_therm_table[0].temp_c;
    }
    if (resistance <= (float)k_therm_table[THERM_TABLE_SIZE - 1U].resistance_ohm) {
        return (float)k_therm_table[THERM_TABLE_SIZE - 1U].temp_c;
    }

    /* Linear interpolation */
    for (uint8_t i = 0U; i < (THERM_TABLE_SIZE - 1U); i++) {
        float r_hi = (float)k_therm_table[i].resistance_ohm;
        float r_lo = (float)k_therm_table[i + 1U].resistance_ohm;
        if (resistance <= r_hi && resistance >= r_lo) {
            float frac = (r_hi - resistance) / (r_hi - r_lo);
            float t_lo = (float)k_therm_table[i].temp_c;
            float t_hi = (float)k_therm_table[i + 1U].temp_c;
            return t_lo + frac * (t_hi - t_lo);
        }
    }

    return 0.0f;
}

/**
 * @brief ms timer callback — decremented from HAL_IncTick (1 ms period).
 */
void app_mode_wpt_timer_cb(void)
{
    if (wpt_ms_timer > 0U) {
        wpt_ms_timer--;
    }
    if (wpt_paused_hold_ms > 0U) {
        wpt_paused_hold_ms--;
    }
}

/**
 * @brief WPT charging mode handler.
 *
 * Entry conditions: VRECT_DETn has gone low (coil detected), state is
 * STATE_ACT_MODE_WPT_HIGH.  Configures the charging hardware, starts BLE
 * advertising, and runs the charging state machine until the coil is removed
 * (state reverts to STATE_ACT_MODE_BLE_ACT via the VRECT EXTI callback).
 */
void app_mode_wpt_handler(void)
{
    uint16_t curr_state = app_func_sm_current_state_get();

    /* ---- Entry: configure charging hardware ---- */
    HAL_GPIO_WritePin(CHG_RATE1_GPIO_Port, CHG_RATE1_Pin, GPIO_PIN_RESET); /* LOW  */
    HAL_GPIO_WritePin(CHG_RATE2_GPIO_Port, CHG_RATE2_Pin, GPIO_PIN_SET);   /* HIGH → 50 mA */
    HAL_GPIO_WritePin(VRECT_MON_EN_GPIO_Port, VRECT_MON_EN_Pin, GPIO_PIN_SET);   /* enable VRECT monitor */
    HAL_GPIO_WritePin(VCHG_DISABLE_GPIO_Port, VCHG_DISABLE_Pin, GPIO_PIN_RESET); /* enable converter */

    /* Enable thermistor and battery monitor */
    app_func_meas_therm_enable(true);
    app_func_meas_batt_mon_enable(true);

    /* Initial battery presence detection */
    uint16_t vbat[2] = {0U, 0U};
    app_func_meas_batt_mon_meas(&vbat[0], &vbat[1]);
    battery_a_present   = (vbat[0] >= WPT_BATT_ABSENT_THRESHOLD_MV);
    battery_b_present   = (vbat[1] >= WPT_BATT_ABSENT_THRESHOLD_MV);
    battery_a_low_count = 0U;
    battery_b_low_count = 0U;
    wpt_paused_hold_ms  = 0U;

    /* Enable chargers for present batteries */
    if (battery_a_present) {
        HAL_GPIO_WritePin(CHG1_EN_GPIO_Port, CHG1_EN_Pin, GPIO_PIN_SET);
    }
    if (battery_b_present) {
        HAL_GPIO_WritePin(CHG2_EN_GPIO_Port, CHG2_EN_Pin, GPIO_PIN_SET);
    }

    /* ---- Set up BLE advertising (same pattern as BLE active mode) ---- */
    BLE_ADV_Setting_t setting = {0};
    _Float64 adv_timeout_f = 0.0;

    uint8_t bleid_len = app_func_para_datalen_get((const uint8_t*)HPID_IPG_BLE_ID);
    uint8_t offset    = app_mode_ble_act_adv_msd_update((uint8_t*)setting.msd);

    app_func_para_data_get((const uint8_t*)BPID_BLE_PASSKEY,          setting.advance.passkey,              (uint8_t)sizeof(setting.advance.passkey));
    app_func_para_data_get((const uint8_t*)BPID_BLE_WHITELIST,        &setting.advance.whitelist_enable,    (uint8_t)sizeof(setting.advance.whitelist_enable));
    app_func_para_data_get((const uint8_t*)HPID_BLE_BROADCAST_TIMEOUT,(uint8_t*)&adv_timeout_f,             (uint8_t)sizeof(adv_timeout_f));
    app_func_para_data_get((const uint8_t*)HPID_IPG_BLE_ID,           (uint8_t*)(&setting.msd[offset]),     bleid_len);
    app_func_para_data_get((const uint8_t*)BPID_BLE_COMPANY_ID,        setting.companyid,                   (uint8_t)sizeof(setting.companyid));

    /* 1-second advertisement burst interval */
    const uint32_t wpt_adv_interval_s = 1U;
    uint32_t passkey_timeout = (uint32_t)adv_timeout_f / 2U;
    (void)memcpy((uint8_t*)setting.passkey_timeout, (uint8_t*)&passkey_timeout,    sizeof(uint32_t));
    (void)memcpy((uint8_t*)setting.adv_timeout,     (uint8_t*)&wpt_adv_interval_s, sizeof(uint32_t));

    setting.msd[LEN_BLE_MSD_MAX - 1] = HW_VERSION;

    if (app_func_ble_is_default()) {
        app_func_para_defdata_get((const uint8_t*)BPID_BLE_PASSKEY, (uint8_t*)setting.advance.passkey);
        setting.advance.whitelist_enable = false;
    }

    app_func_ble_enable(true);
    uint8_t curr_ble_state = app_func_ble_curr_state_get();

    /* Safety net: if BLE is still in CONNECT state (disconnect may still be
     * in flight from app_mode_ble_conn_handler), wait for the CONNECT bit to
     * clear before starting advertising.  On the normal entry path from
     * STATE_ACT_MODE_BLE_ACT the bit will already be clear and this block is
     * skipped entirely.                                                        */
    if ((curr_ble_state & BLE_STATE_CONNECT) != 0U) {
        uint32_t conn_wait = 3000U;
        while (conn_wait > 0U) {
            bsp_wdg_refresh();
            app_func_ble_new_state_get();
            while (!bsp_sp_cmd_handler()) {
                HAL_Delay(1);
            }
            curr_ble_state = app_func_ble_curr_state_get();
            if ((curr_ble_state & BLE_STATE_CONNECT) == 0U) {
                break;
            }
            HAL_Delay(1);
            conn_wait--;
        }
    }

    app_mode_ble_act_adv_msd_update((uint8_t*)setting.msd);
    app_func_ble_adv_start(&setting);
    while (!bsp_sp_cmd_handler()) {
        HAL_Delay(1);
    }
    /* Wait for advertising to start */
    uint32_t start_wait = 1000U;
    while ((curr_ble_state != BLE_STATE_ADV_START) && (start_wait > 0U)) {
        bsp_wdg_refresh();
        app_func_ble_new_state_get();
        while (!bsp_sp_cmd_handler()) {
            HAL_Delay(1);
        }
        curr_ble_state = app_func_ble_curr_state_get();
        HAL_Delay(1);
        start_wait--;
    }

    /* ---- Main WPT charging loop ---- */
    wpt_ms_timer = 1000U;  /* first sample after 1 second */

    while (curr_state == STATE_ACT_MODE_WPT_HIGH || curr_state == STATE_ACT_MODE_WPT_PAUSED) {
        bsp_wdg_refresh();

        /* 1-second sample tick */
        if (wpt_ms_timer == 0U) {
            wpt_ms_timer = 1000U;

            /* Sample battery voltages */
            app_func_meas_batt_mon_meas(&vbat[0], &vbat[1]);

            /* Sample thermistor */
            uint16_t therm[3] = {0U, 0U, 0U};
            app_func_meas_therm_meas(THERM_ID_REF,  (uint8_t*)&therm[0], sizeof(uint16_t), 1000U);
            app_func_meas_therm_meas(THERM_ID_OUT,  (uint8_t*)&therm[1], sizeof(uint16_t), 1000U);
            app_func_meas_therm_meas(THERM_ID_OFST, (uint8_t*)&therm[2], sizeof(uint16_t), 1000U);
            float temp_c = app_mode_wpt_calc_temperature(therm[0], therm[1], therm[2]);

            /* Battery A absence detection (2-consecutive-below-1V rule) */
            if (vbat[0] < WPT_BATT_ABSENT_THRESHOLD_MV) {
                if (battery_a_low_count < WPT_BATT_ABSENT_CONSECUTIVE) {
                    battery_a_low_count++;
                }
                if (battery_a_low_count >= WPT_BATT_ABSENT_CONSECUTIVE && battery_a_present) {
                    battery_a_present = false;
                    HAL_GPIO_WritePin(CHG1_EN_GPIO_Port, CHG1_EN_Pin, GPIO_PIN_RESET);
                }
            } else {
                battery_a_low_count = 0U;
            }

            /* Battery B absence detection */
            if (vbat[1] < WPT_BATT_ABSENT_THRESHOLD_MV) {
                if (battery_b_low_count < WPT_BATT_ABSENT_CONSECUTIVE) {
                    battery_b_low_count++;
                }
                if (battery_b_low_count >= WPT_BATT_ABSENT_CONSECUTIVE && battery_b_present) {
                    battery_b_present = false;
                    HAL_GPIO_WritePin(CHG2_EN_GPIO_Port, CHG2_EN_Pin, GPIO_PIN_RESET);
                }
            } else {
                battery_b_low_count = 0U;
            }

            /* Read protection status GPIOs.
             * CHGx_STATUS (nCHRG) is telemetry only — it is read by
             * app_mode_ble_act_adv_msd_update() via setBitFromGpioState and
             * does not drive any state transition. CHGx_EN stays HIGH and the
             * LTC4065 manages its own charge cycle autonomously. */
            bool chg1_ovp_err = (HAL_GPIO_ReadPin(CHG1_OVP_ERRn_GPIO_Port, CHG1_OVP_ERRn_Pin) == GPIO_PIN_RESET);
            bool chg2_ovp_err = (HAL_GPIO_ReadPin(CHG2_OVP_ERRn_GPIO_Port, CHG2_OVP_ERRn_Pin) == GPIO_PIN_RESET);
            bool vrect_ovp    = (HAL_GPIO_ReadPin(VRECT_OVPn_GPIO_Port,     VRECT_OVPn_Pin)    == GPIO_PIN_RESET);

            if (curr_state == STATE_ACT_MODE_WPT_HIGH) {
                /* Evaluate HIGH → PAUSED transition.
                 * Only external protection conditions trigger a pause — thermal,
                 * per-battery OVP error, and rectifier OVP. Charge completion
                 * (CHGx_STATUS) is handled autonomously by the LTC4065 and does
                 * not cause a firmware-level pause. */
                bool pause_condition =
                    (battery_a_present && chg1_ovp_err) ||
                    (battery_b_present && chg2_ovp_err) ||
                    (temp_c > WPT_THERM_PAUSE_THRESHOLD_C) ||
                    vrect_ovp;

                if (pause_condition) {
                    HAL_GPIO_WritePin(VCHG_DISABLE_GPIO_Port, VCHG_DISABLE_Pin, GPIO_PIN_SET);
                    HAL_GPIO_WritePin(CHG1_EN_GPIO_Port,      CHG1_EN_Pin,      GPIO_PIN_RESET);
                    HAL_GPIO_WritePin(CHG2_EN_GPIO_Port,      CHG2_EN_Pin,      GPIO_PIN_RESET);
                    /* Hold in PAUSED for OVP events — gives the charger 2–3 BLE
                     * advertisement cycles to see VRECT_OVP and reduce coil power
                     * before the IPG re-enables and re-triggers the fault. */
                    if (vrect_ovp) {
                        wpt_paused_hold_ms = WPT_OVP_PAUSE_HOLD_MS;
                    }
                    app_func_sm_current_state_set(STATE_ACT_MODE_WPT_PAUSED);
                }
            } else {
                /* STATE_ACT_MODE_WPT_PAUSED — evaluate PAUSED → HIGH transition.
                 * Resume as soon as the protection conditions that caused the pause
                 * have cleared. The LTC4065 will self-manage charging once re-enabled. */
                if ((battery_a_present || battery_b_present) &&
                    (temp_c < WPT_THERM_RESUME_THRESHOLD_C) &&
                    !vrect_ovp &&
                    (wpt_paused_hold_ms == 0U)) {
                    HAL_GPIO_WritePin(VCHG_DISABLE_GPIO_Port, VCHG_DISABLE_Pin, GPIO_PIN_RESET);
                    if (battery_a_present) {
                        HAL_GPIO_WritePin(CHG1_EN_GPIO_Port, CHG1_EN_Pin, GPIO_PIN_SET);
                    }
                    if (battery_b_present) {
                        HAL_GPIO_WritePin(CHG2_EN_GPIO_Port, CHG2_EN_Pin, GPIO_PIN_SET);
                    }
                    app_func_sm_current_state_set(STATE_ACT_MODE_WPT_HIGH);
                }
            }

            /* Update BLE advertisement with fresh measurements + WPT state bits */
            app_mode_ble_act_adv_msd_update((uint8_t*)setting.msd);
        }

        /* BLE state machine — restart advertising when a burst ends */
        app_func_ble_new_state_get();
        bsp_sp_cmd_handler();
        HAL_Delay(50);
        curr_ble_state = app_func_ble_curr_state_get();
        if (curr_ble_state == BLE_STATE_ADV_STOP) {
            app_mode_ble_act_adv_msd_update((uint8_t*)setting.msd);
            app_func_ble_adv_start(&setting);
            while (!bsp_sp_cmd_handler()) {
                HAL_Delay(1);
            }
            uint32_t wait = 1000U;
            while ((curr_ble_state != BLE_STATE_ADV_START) && (wait > 0U)) {
                bsp_wdg_refresh();
                app_func_ble_new_state_get();
                while (!bsp_sp_cmd_handler()) {
                    HAL_Delay(1);
                }
                curr_ble_state = app_func_ble_curr_state_get();
                HAL_Delay(1);
                wait--;
            }
        }

        curr_state = app_func_sm_current_state_get();
    }

    /* ---- Exit: coil removed (or state changed externally) — disable charging ---- */
    HAL_GPIO_WritePin(VCHG_DISABLE_GPIO_Port, VCHG_DISABLE_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(CHG1_EN_GPIO_Port,      CHG1_EN_Pin,      GPIO_PIN_RESET);
    HAL_GPIO_WritePin(CHG2_EN_GPIO_Port,      CHG2_EN_Pin,      GPIO_PIN_RESET);
    HAL_GPIO_WritePin(VRECT_MON_EN_GPIO_Port, VRECT_MON_EN_Pin, GPIO_PIN_RESET);
    app_func_ble_enable(false);
}
