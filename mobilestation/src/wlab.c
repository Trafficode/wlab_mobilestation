/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: wlab.c
 * --------------------------------------------------------------------------*/
#include "wlab.h"

#include <stdint.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/kernel_version.h>
#include <zephyr/logging/log.h>

#include "nvs_data.h"
#include "sim800l.h"
#include "version.h"

struct wlab_buffer {
    int16_t min;
    int16_t max;
    int16_t max_ts_offset;
    int16_t min_ts_offset;
    int32_t buff;
    int32_t cnt;
    int64_t sample_ts;
    int16_t sample_ts_val;
};

static void wlab_buffer_commit(struct wlab_buffer *buffer, int16_t val,
                               int64_t ts);
static void wlab_buffer_init(struct wlab_buffer *buffer, int64_t ts);

static int64_t wlab_timestamp_get(void);
static bool wlab_timestamp_sync(void);
static void wlab_timestamp_check(void);

LOG_MODULE_REGISTER(WLAB, LOG_LEVEL_DBG);

struct wlab_buffer TempBuffer;
struct wlab_buffer HumBuffer;

static int64_t PublishPeriodSec;
static int64_t SampleTsSec;
static int64_t UpdateTsSec;
static int64_t UpdateTsUptimeSec;

const struct device *const Sht3xDev = DEVICE_DT_GET_ONE(sensirion_sht3xd);

const struct gpio_dt_spec External3v3Pin =
    GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), external3v3_gpios);

void wlab_init(void) {
    if (!device_is_ready(Sht3xDev)) {
        LOG_ERR("Device %s is not ready\n", Sht3xDev->name);
    }
    nvs_data_wlab_pub_period_get(&PublishPeriodSec);

    if (!device_is_ready(External3v3Pin.port)) {
        LOG_ERR("External3v3Pin enable not ready");
    }

    gpio_pin_configure_dt(&External3v3Pin, GPIO_OUTPUT_HIGH);
    gsm_modem_init();
    // while (!wlab_timestamp_sync()) {
    //     LOG_ERR("Failed to sync timestamp...");
    //     k_sleep(K_MSEC(2000));
    // }

    // int64_t ts = wlab_timestamp_get();
    // // Make sure that sample timestamp is exactly the second when the sample
    // // should be. TS will be a bit bigger so substract this difference
    // SampleTsSec = ts - (ts % PublishPeriodSec);
    // wlab_buffer_init(&TempBuffer, SampleTsSec);
    // wlab_buffer_init(&HumBuffer, SampleTsSec);

    if (!gsm_modem_config()) {
        LOG_ERR("Configure GSM modem failed");
    }
}

void wlab_proc(void) {
    if (!gsm_modem_config()) {
        LOG_ERR("Configure GSM modem failed");
    } else {
        LOG_INF("GSM communication OOK!");
    }
    // struct sensor_value temp, hum;
    // int16_t i_temp, i_humidity;

    // int sensor_rc = sensor_sample_fetch(Sht3xDev);
    // if (0 == sensor_rc) {
    //     sensor_channel_get(Sht3xDev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
    //     sensor_channel_get(Sht3xDev, SENSOR_CHAN_HUMIDITY, &hum);

    //     // static inline double sensor_value_to_double(
    //     //     const struct sensor_value *val) {
    //     //     return (double)val->val1 + (double)val->val2 / 1000000;
    //     // }
    //     i_temp = temp->val1 * 10 + temp->val2 / 100000;
    //     i_humidity = hum->val1 * 10 + hum->val2 / 100000;
    //     LOG_INF("SHT3XD: %d Cel ; %d %%RH\n", i_temp, i_humidity);
    // } else {
    //     LOG_ERR("SHT3XD: failed: %d\n", rc);
    // }

    // int64_t ts = wlab_timestamp_get();
    // if (ts >= SampleTsSec + PublishPeriodSec) {
    //     // Send sample and sync time

    //     int32_t sync_attempt = 0;
    //     for (sync_attempt = 0; sync_attempt < 4; sync_attempt++) {
    //     }

    //     if (sync_attempt < 4) {
    //         // Confirm that timestamp is correct
    //         wlab_timestamp_check();
    //     }

    //     SampleTsSec += PublishPeriodSec;
    // } else if (0 == sensor_rc) {
    //     wlab_buffer_commit(&TempBuffer, i_temp, ts);
    //     wlab_buffer_commit(&HumBuffer, i_humidity, ts);
    // }
}

static void wlab_buffer_init(struct wlab_buffer *buffer, int64_t ts) {
    buffer->buff = 0;
    buffer->cnt = 0;
    buffer->max = INT16_MIN;
    buffer->min = INT16_MAX;
    buffer->max_ts_offset = 0;
    buffer->min_ts_offset = 0;
    buffer->sample_ts_val = INT16_MAX;
    buffer->sample_ts = ts;
}

static void wlab_buffer_commit(struct wlab_buffer *buffer, int16_t val,
                               int64_t ts) {
    if (val > buffer->max) {
        buffer->max = val;
        buffer->max_ts_offset = buffer->sample_ts - ts;
    }

    if (val < buffer->min) {
        buffer->min = val;
        buffer->min_ts_offset = ts - (ts % INT64_C(60));
    }

    buffer->buff += (int32_t)val;
    buffer->cnt++;
}

static int64_t wlab_timestamp_get(void) {
    int64_t uptime_sec = k_uptime_get() / 1000;
    return (UpdateTsSec + (uptime_sec - UpdateTsUptimeSec));
}

static bool wlab_timestamp_sync(void) {
    bool rc = false;
    // ret = sntp_simple("0.pl.pool.ntp.org", 4000, &sntp_time);
    // if (0 == ret) {
    //     SntpSyncSec = (int64_t)sntp_time.seconds;
    //     UptimeSyncMs = k_uptime_get();
    //     LOG_INF("Acquire SNTP success");
    // } else {
    //     LOG_ERR("Failed to acquire SNTP, code %d", ret);
    // }
    UpdateTsSec = 0;
    UpdateTsUptimeSec = k_uptime_get() / 1000;
    return (rc);
}

static void wlab_timestamp_check(void) {
    // in case of very long publish period there can be situation where time after update
    // will not be bigger that last SampleTs + PublishPeriod. Lets verify it here

    bool check_status = false;
    do {
        if (wlab_timestamp_get() < (SampleTsSec + PublishPeriodSec)) {
            // keep continue waiting
            k_sleep(K_MSEC(1000));
        } else {
            check_status = true;
        }
    } while (!check_status);
}

/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/