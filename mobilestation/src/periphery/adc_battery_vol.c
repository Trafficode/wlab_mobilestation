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

static const struct adc_dt_spec AdcChannel =
    ADC_DT_SPEC_GET(DT_PATH(zephyr_user));

LOG_MODULE_REGISTER(ADC_BATT, LOG_LEVEL_DBG);

void adc_battery_vol_init(void) {
    if (!adc_is_ready_dt(&AdcChannel)) {
        LOG_ERR("ADC controller devivce %s not ready", AdcChannel.dev->name);
    }

    int err = adc_channel_setup_dt(&AdcChannel);
    if (err < 0) {
        LOG_ERR("Could not setup channel (%d)", err);
    }
}

int32_t adc_battery_vol_get_milliv(void) {
    int32_t milliv = 0;
    int16_t buf;

    struct adc_sequence sequence = {
        .buffer = &buf,
        // buffer size in bytes, not number of samples
        .buffer_size = sizeof(buf),
    };

    int err = adc_sequence_init_dt(&AdcChannel, &sequence);
    if (err < 0) {
        LOG_ERR("Could not initalize sequnce");
        goto DONE;
    }

    err = adc_read(AdcChannel.dev, &sequence);
    if (err < 0) {
        LOG_ERR("Could not read (%d)", err);
        goto DONE;
    }

    if (AdcChannel.channel_cfg.differential) {
        milliv = (int32_t)((int16_t)buf);
    } else {
        milliv = (int32_t)buf;
    }

    err = adc_raw_to_millivolts_dt(&AdcChannel, &milliv);
    if (err < 0) {
        LOG_WRN(" (value in mV not available)");
        goto DONE;
    }

DONE:
    return (milliv);
}

/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/