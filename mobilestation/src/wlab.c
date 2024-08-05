/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: wlab.c
 * --------------------------------------------------------------------------*/
#include "wlab.h"

#include <stdint.h>
#include <stdio.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/kernel_version.h>
#include <zephyr/logging/log.h>

#include "nvs_data.h"
#include "sim800l.h"
#include "version.h"

struct __attribute__((packed)) wlab_buffer {
    int16_t min;
    int16_t max;
    int16_t max_ts_offset;
    int16_t min_ts_offset;
    int32_t buff;
    int32_t cnt;
    int64_t sample_ts;
    int16_t act;
};

struct __attribute__((packed)) wlab_db_bin {
    uint8_t version;
    uint8_t id[6];
    int64_t ts;
    int16_t temp_act;
    int16_t temp_avg;
    int16_t temp_max;
    int16_t temp_min;
    int16_t temp_max_ts_offset;
    int16_t temp_min_ts_offset;
    uint8_t humidity_act;
    uint8_t humidity_avg;
    uint8_t humidity_max;
    uint8_t humidity_min;
    int16_t humidity_max_ts_offset;
    int16_t humidity_min_ts_offset;
    int16_t battery_voltage;
};

static void wlab_buffer_commit(struct wlab_buffer *buffer, int16_t val,
                               int64_t ts);

static void wlab_buffer_init(struct wlab_buffer *buffer, int64_t ts);

static int64_t wlab_timestamp_get(void);
static bool wlab_timestamp_sync(void);
static void wlab_timestamp_check(void);
static void wlab_bin_package_prepare(struct wlab_db_bin *sample);
static bool wlab_publish(void);

LOG_MODULE_REGISTER(WLAB, LOG_LEVEL_DBG);

struct wlab_buffer TempBuffer;
struct wlab_buffer HumBuffer;

static int64_t PublishPeriodSec;
static int64_t SampleTsSec;
static int64_t UpdateTsSec;
static int64_t UpdateTsUptimeSec;
static uint64_t DevieId;
struct mqtt_config MqttConfig = {};

const struct device *const Sht3xDev = DEVICE_DT_GET_ONE(sensirion_sht3xd);

const struct gpio_dt_spec External3v3Pin =
    GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), external3v3_gpios);

void wlab_init(void) {
    if (!device_is_ready(Sht3xDev)) {
        LOG_ERR("Device %s is not ready\n", Sht3xDev->name);
    }
    nvs_data_wlab_pub_period_get(&PublishPeriodSec);
    PublishPeriodSec *= INT64_C(60);   // conver to seconds
    nvs_data_wlab_device_id_get(&DevieId);
    nvs_data_mqtt_config_get(&MqttConfig);
    LOG_INF("PublishPeriodSec %" PRIi64, PublishPeriodSec);
    LOG_INF("DevieId %" PRIX64, DevieId);
    LOG_INF("MqttConfig.broker <%s>", MqttConfig.broker);
    LOG_INF("MqttConfig.port <%u>", MqttConfig.port);

    if (!device_is_ready(External3v3Pin.port)) {
        LOG_ERR("External3v3Pin enable not ready");
    }

    gpio_pin_configure_dt(&External3v3Pin, GPIO_OUTPUT_HIGH);
    gsm_modem_init();

    while (true) {
        if (!gsm_modem_test()) {
            LOG_ERR("No communication with modem");
            continue;
        }

        if (!gsm_modem_config()) {
            LOG_ERR("Configure modem failed");
            continue;
        }

        if (!gsm_modem_reset()) {
            LOG_ERR("Reset modem failed");
            continue;
        }

        if (!gsm_modem_test()) {
            LOG_ERR("No communication with modem");
            continue;
        }

        if (!gsm_modem_net_setup()) {
            LOG_ERR("Network up failed");
            continue;
        }

        if (!gsm_modem_net_setup()) {
            LOG_ERR("Network up failed");
            continue;
        }

        if (!wlab_timestamp_sync()) {
            LOG_ERR("Time sync failed");
            continue;
        }

        // all done sucessfully, put modem into sleep mode
        gsm_modem_sleep();
        break;
    }

    int64_t ts = wlab_timestamp_get();
    // Make sure that sample timestamp is exactly the second when the sample
    // should be. TS will be a bit bigger so substract this difference
    SampleTsSec = ts - (ts % PublishPeriodSec);
    wlab_buffer_init(&TempBuffer, SampleTsSec);
    wlab_buffer_init(&HumBuffer, SampleTsSec);
}

static bool wlab_publish(void) {
    bool res = false;
    struct wlab_db_bin sample_bin = {};

    if ((0 == MqttConfig.broker[0]) || (0 == MqttConfig.port) ||
        (0 == DevieId)) {
        LOG_ERR("Device not configured");
        goto DONE;
    }

    wlab_bin_package_prepare(&sample_bin);

    if (!gsm_modem_wakeup()) {
        LOG_ERR("Failed to wakeup modem");
        goto DONE;
    }

    if (!gsm_modem_test()) {
        LOG_ERR("No communication with modem");
        goto DONE;
    }

    if (!gsm_modem_config()) {
        LOG_ERR("Configure modem failed");
        goto DONE;
    }

    if (!gsm_modem_net_setup()) {
        LOG_ERR("Network up failed");
        goto DONE;
    }

    if (!wlab_timestamp_sync()) {
        LOG_ERR("Time sync failed");
        goto DONE;
    }

    if (!gsm_modem_mqtt_connect(MqttConfig.broker, MqttConfig.port)) {
        LOG_ERR("Connect to MQTT failed");
        goto DONE;
    }

    if (!gsm_modem_mqtt_publish(WLAB_DEFAULT_SAMPLE_TOPIC,
                                (uint8_t *)&sample_bin,
                                sizeof(struct wlab_db_bin))) {
        LOG_ERR("Publish to MQTT failed");
        gsm_modem_mqtt_close();
        goto DONE;
    }

    gsm_modem_mqtt_close();
    res = true;
DONE:
    gsm_modem_sleep();   // shuld it be repeated and repeated?
    return (res);
}

void wlab_proc(void) {
    struct sensor_value temp, hum;
    int16_t i_temp, i_humidity;

    int sensor_rc = sensor_sample_fetch(Sht3xDev);
    if (0 == sensor_rc) {
        sensor_channel_get(Sht3xDev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
        sensor_channel_get(Sht3xDev, SENSOR_CHAN_HUMIDITY, &hum);

        i_temp = temp.val1 * 10 + temp.val2 / 100000;
        i_humidity = hum.val1 * 10 + hum.val2 / 100000;
        LOG_INF("SHT3XD: %d Cel ; %d %%RH\n", i_temp, i_humidity);
    } else {
        LOG_ERR("SHT3XD: failed: %d\n", sensor_rc);
    }

    int64_t ts = wlab_timestamp_get();
    if (ts >= SampleTsSec + PublishPeriodSec) {
        // Send sample and sync time

        if (TempBuffer.cnt > 0) {
            wlab_publish();
        } else {
            // Device not configured or sensor problem
        }

        // Check if time after sycn is not lower that counted ts
        wlab_timestamp_check();

        // Make sure that sample timestamp is exactly the second when the sample
        // should be. TS will be a bit bigger so substract this difference
        SampleTsSec = ts - (ts % PublishPeriodSec);

        // Buffers reset
        wlab_buffer_init(&TempBuffer, SampleTsSec);
        wlab_buffer_init(&HumBuffer, SampleTsSec);
    }

    if (0 == sensor_rc) {
        wlab_buffer_commit(&TempBuffer, i_temp, ts);
        wlab_buffer_commit(&HumBuffer, i_humidity, ts);
    }

    k_sleep(K_MSEC(20 * 1000));   // read sample every 20sec
}

static void wlab_bin_package_prepare(struct wlab_db_bin *sample) {
    sample->version = 0x01;
    sample->ts = TempBuffer.sample_ts;   // never mind if it is temp or humidity
    memcpy(sample->id, (uint8_t *)&DevieId, 6);
    sample->battery_voltage = 0;

    sample->humidity_act = (uint8_t)HumBuffer.act;
    sample->humidity_avg = (uint8_t)(HumBuffer.buff / HumBuffer.cnt);
    sample->humidity_max = (uint8_t)HumBuffer.max;
    sample->humidity_min = (uint8_t)HumBuffer.min;
    sample->humidity_min_ts_offset = HumBuffer.min_ts_offset;
    sample->humidity_max_ts_offset = HumBuffer.max_ts_offset;

    sample->temp_act = TempBuffer.act;
    sample->temp_avg = (int16_t)(TempBuffer.buff / TempBuffer.cnt);
    sample->temp_max = TempBuffer.max;
    sample->temp_min = TempBuffer.min;
    sample->temp_min_ts_offset = TempBuffer.min_ts_offset;
    sample->temp_max_ts_offset = TempBuffer.max_ts_offset;
}

static void wlab_buffer_init(struct wlab_buffer *buffer, int64_t ts) {
    buffer->buff = 0;
    buffer->cnt = 0;
    buffer->max = INT16_MIN;
    buffer->min = INT16_MAX;
    buffer->max_ts_offset = 0;
    buffer->min_ts_offset = 0;
    buffer->act = INT16_MAX;
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

    buffer->act = val;
    buffer->buff += (int32_t)val;
    buffer->cnt++;
}

static int64_t wlab_timestamp_get(void) {
    int64_t uptime_sec = k_uptime_get() / 1000;
    return (UpdateTsSec + (uptime_sec - UpdateTsUptimeSec));
}

static bool wlab_timestamp_sync(void) {
    bool res = false;

    if (gsm_modem_get_timestamp(&UpdateTsSec)) {
        UpdateTsUptimeSec = k_uptime_get() / 1000;
        res = true;
    }

    return (res);
}

static void wlab_timestamp_check(void) {
    // in case of very long publish period there can be situation where time after update
    // will not be bigger than last SampleTs + PublishPeriod.
    // Lets verify it here and wait if needed

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