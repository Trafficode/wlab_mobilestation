/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: adc_battery_vol.c
 * --------------------------------------------------------------------------*/
#include "periphery/adc_battery_vol.h"

#include <stdint.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

static const struct adc_dt_spec BattVoltageCh =
    ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 0);

LOG_MODULE_REGISTER(ADC_BATT, LOG_LEVEL_DBG);

void adc_battery_vol_init(void) {
    int err = adc_channel_setup_dt(&BattVoltageCh);
    if (err < 0) {
        LOG_ERR("Could not setup channel %d", err);
        return;
    }
}

int32_t adc_battery_vol_get_milliv(void) {
    int32_t milliv = 0;
    int16_t buf = 0;

    struct adc_sequence sequence = {
        .buffer = &buf,
        // buffer size in bytes, not number of samples
        .buffer_size = sizeof(buf),
    };

    int err = adc_read(BattVoltageCh.dev, &sequence);
    if (err < 0) {
        LOG_ERR("Could not read (%d)", err);
        goto DONE;
    } else {
        LOG_INF("Raw adc value %" PRId16, buf);
    }

    // conversion to mV may not be supported, skip if not
    milliv = (int32_t)buf;
    err = adc_raw_to_millivolts_dt(&BattVoltageCh, &milliv);
    if (err < 0) {
        milliv = 0;
        LOG_ERR("Adc (value in mV not available)");
    } else {
        LOG_INF("Adc battery = %" PRId32 " mV", milliv);
    }

DONE:
    return (milliv);
}

/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/