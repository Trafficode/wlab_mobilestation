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
#include <zephyr/sys/reboot.h>

#include "nvs_data.h"
#include "periphery/adc_battery_vol.h"
#include "periphery/gpio_ext3v3.h"
#include "periphery/gpio_status_led.h"
#include "periphery/gpio_user_btn.h"
#include "sample_storage.h"
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

LOG_MODULE_REGISTER(WLAB, LOG_LEVEL_DBG);

#define WLAB_RESEND_MAX_NUM (1)

static void wlab_buffer_commit(struct wlab_buffer *buffer, int16_t val,
                               int64_t ts);
static void wlab_buffer_init(struct wlab_buffer *buffer, int64_t ts);

static int64_t wlab_timestamp_utc_get(void);
static bool wlab_timestamp_sync(void);
static void wlab_timestamp_check(void);
static void wlab_bin_package_prepare(struct wlab_db_bin *sample);
static bool wlab_publish(void);

static void wlab_publish_succ_led_scene(void) {
    // No aciton
}

static void wlab_publish_failed_led_scene(void) {
    gpio_status_led_set_state(1);
    k_sleep(K_MSEC(4));
    gpio_status_led_set_state(0);
    k_sleep(K_MSEC(1000));
    gpio_status_led_set_state(1);
    k_sleep(K_MSEC(4));
    gpio_status_led_set_state(0);
    k_sleep(K_MSEC(1000));
    gpio_status_led_set_state(1);
    k_sleep(K_MSEC(4));
    gpio_status_led_set_state(0);
}

static void wlab_startup_succ_led_scene(void) {
    gpio_status_led_set_state(1);
    k_sleep(K_MSEC(4));
    gpio_status_led_set_state(0);
}

static void wlab_startup_failed_led_scene(void) {
    gpio_status_led_set_state(1);
    k_sleep(K_MSEC(4));
    gpio_status_led_set_state(0);
    k_sleep(K_MSEC(1000));
    gpio_status_led_set_state(1);
    k_sleep(K_MSEC(4));
    gpio_status_led_set_state(0);
}

static void wlab_sensor_succ_led_scene(void) {
    gpio_status_led_set_state(1);
    k_sleep(K_MSEC(4));
    gpio_status_led_set_state(0);
}

static void wlab_sensor_failed_led_scene(void) {
    gpio_status_led_set_state(1);
    k_sleep(K_MSEC(4));
    gpio_status_led_set_state(0);
    k_sleep(K_MSEC(1000));
    gpio_status_led_set_state(1);
    k_sleep(K_MSEC(4));
    gpio_status_led_set_state(0);
}

static void wlab_sensor_sys_reboot_led_scene(void) {
    gpio_status_led_set_state(1);
    k_sleep(K_MSEC(1000));
    gpio_status_led_set_state(0);
}

struct wlab_buffer TempBuffer;
struct wlab_buffer HumBuffer;

static int64_t PublishPeriodSec;
static int64_t SampleTsSec;
static int64_t UpdateTsSec;
static int64_t UpdateTsUptimeSec;
static uint64_t DevieId;
struct mqtt_config MqttConfig = {};
struct apn_config ApnConfig = {};
static uint8_t PubErrorCnt = 0;

const struct device *const Sht3xDev = DEVICE_DT_GET_ONE(sensirion_sht3xd);

void wlab_init(void) {
    if (!device_is_ready(Sht3xDev)) {
        LOG_ERR("Device %s is not ready", Sht3xDev->name);
    }

    nvs_data_wlab_pub_period_get(&PublishPeriodSec);
    PublishPeriodSec *= INT64_C(60);   // Conver to seconds
    nvs_data_wlab_device_id_get(&DevieId);
    nvs_data_mqtt_config_get(&MqttConfig);
    nvs_data_apn_config_get(&ApnConfig);

    LOG_INF("PublishPeriodSec %" PRIi64, PublishPeriodSec);
    LOG_INF("DevieId %" PRIX64, DevieId);
    LOG_INF("MqttConfig.broker <%s>", MqttConfig.broker);
    LOG_INF("MqttConfig.port <%u>", MqttConfig.port);
    LOG_INF("ApnConfig apn <%s>, user <%s>, password <%s>", ApnConfig.apn,
            ApnConfig.user, ApnConfig.password);

    int32_t startup_try = 0;
    while (true) {
        if (startup_try > 0) {
            wlab_startup_failed_led_scene();
        }

        startup_try++;
        if (!gsm_modem_wakeup()) {
            LOG_ERR("Wakeup failed");
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

        if (!gsm_modem_config()) {
            LOG_ERR("Configure modem failed");
            continue;
        }

        if (!gsm_modem_net_setup(&ApnConfig)) {
            LOG_ERR("Network up failed");
            continue;
        }

        if (!wlab_timestamp_sync()) {
            LOG_ERR("Time sync failed");
            continue;
        }

        // All done sucessfully, put modem into sleep mode
        gsm_modem_sleep();
        wlab_startup_succ_led_scene();
        break;
    }

    int64_t ts = wlab_timestamp_utc_get();
    // Make sure that sample timestamp is exactly the second when the sample
    // should be. TS will be a bit bigger so substract this difference
    SampleTsSec = ts - (ts % PublishPeriodSec);
    wlab_buffer_init(&TempBuffer, SampleTsSec);
    wlab_buffer_init(&HumBuffer, SampleTsSec);
}

void wlab_proc(void) {
    struct sensor_value temp, hum;
    int16_t i_temp, i_humidity;
    int32_t batt_milliv;
    static uint8_t sensror_err_counter = 0;

    LOG_INF("Heart beat, sensor read...");
    int sensor_rc = sensor_sample_fetch(Sht3xDev);
    if (0 == sensor_rc) {
        sensor_channel_get(Sht3xDev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
        sensor_channel_get(Sht3xDev, SENSOR_CHAN_HUMIDITY, &hum);

        i_temp = temp.val1 * 10 + temp.val2 / 100000;
        i_humidity = hum.val1 * 10 + hum.val2 / 100000;
        LOG_INF("SHT3XD: %d Cel ; %d %%RH", i_temp, i_humidity);
        sensror_err_counter = 0;
        wlab_sensor_succ_led_scene();
    } else {
        LOG_ERR("SHT3XD: failed: %d", sensor_rc);
        sensror_err_counter++;
        wlab_sensor_failed_led_scene();
    }

    if (3 == sensror_err_counter) {
        // Reboot station once sensor failed 3 times, one by oen
        wlab_sensor_sys_reboot_led_scene();
        sys_reboot(SYS_REBOOT_COLD);
    }

    batt_milliv = adc_battery_vol_get_milliv();
    LOG_INF("Battery voltage: %d[mv]", batt_milliv);
    // Temporary instead of humidity, send battery voltage
    i_humidity = (batt_milliv) % 1000;

    int64_t ts = wlab_timestamp_utc_get();
    if (ts >= SampleTsSec + PublishPeriodSec) {
        // Send sample and sync time
        if (TempBuffer.cnt > 0) {
            if (wlab_publish()) {
                wlab_publish_succ_led_scene();
            } else {
                wlab_publish_failed_led_scene();
            }
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

    k_sleep(K_MSEC(10 * 1000));   // read sample every 10sec
}

static bool wlab_publish(void) {
    bool res = false;
    struct wlab_db_bin sample_bin[2];

    if ((0 == MqttConfig.broker[0]) || (0 == MqttConfig.port) ||
        (0 == DevieId)) {
        LOG_ERR("Device not configured");
        goto DONE;
    }

    wlab_bin_package_prepare(&sample_bin[0]);

    if (!gsm_modem_wakeup()) {
        LOG_ERR("Failed to wakeup modem");
        goto DONE;
    }

    if (4 == PubErrorCnt) {
        LOG_WRN("Reset gsm modem!");
        gsm_modem_reset();
        PubErrorCnt = 0;
    }

    if (!gsm_modem_test()) {
        LOG_ERR("No communication with modem");
        goto DONE;
    }

    if (!gsm_modem_config()) {
        LOG_ERR("Configure modem failed");
        goto DONE;
    }

    if (!gsm_modem_net_setup(&ApnConfig)) {
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
    uint16_t pull_idx = 0;
    bool pull_rc = sample_storage_pull(&sample_bin[1],
                                       sizeof(struct wlab_db_bin), &pull_idx);
    if (true == pull_rc) {
        LOG_INF("PULL IDX %u ID %02X%02X%02X%02X%02X%02X %lli", pull_idx,
                sample_bin[1].id[5], sample_bin[1].id[4], sample_bin[1].id[3],
                sample_bin[1].id[2], sample_bin[1].id[1], sample_bin[1].id[0],
                sample_bin[1].ts);

        // XXXYYYYY
        // XXX sumples number
        // YYYYY semples type
        sample_bin[0].version |= (WLAB_RESEND_MAX_NUM << 5);
        sample_bin[1].humidity_min = 0;
    } else {
        LOG_INF("Arch empty");
    }

    size_t publish_len =
        (1 + (sample_bin[0].version >> 5)) * sizeof(struct wlab_db_bin);

    if (!gsm_modem_mqtt_publish(WLAB_DEFAULT_SAMPLE_TOPIC,
                                (uint8_t *)&sample_bin, publish_len, 2,
                                MQTT_PUBLISH_QOS_1)) {
        LOG_ERR("Publish to MQTT failed");
        gsm_modem_mqtt_close();
        goto DONE;
    }

    if (pull_rc) {
        sample_storage_mark_as_sent(pull_idx);
    }

    gsm_modem_mqtt_close();
    PubErrorCnt = 0;
    res = true;
DONE:
    if (false == res) {
        LOG_INF("PUSH ID %02X%02X%02X%02X%02X%02X %lli", sample_bin[0].id[5],
                sample_bin[0].id[4], sample_bin[0].id[3], sample_bin[0].id[2],
                sample_bin[0].id[1], sample_bin[0].id[0], sample_bin[0].ts);
        sample_storage_push(&sample_bin[0], sizeof(struct wlab_db_bin));
        PubErrorCnt++;
    }
    gsm_modem_sleep();   // shuld it be repeated and repeated?
    return (res);
}

static void wlab_bin_package_prepare(struct wlab_db_bin *sample) {
    sample->version = 0x01;
    sample->ts = TempBuffer.sample_ts;   // never mind if it is temp or humidity
    memcpy(sample->id, (uint8_t *)&DevieId, 6);
    sample->battery_voltage = 0;

    sample->humidity_act = (uint8_t)(HumBuffer.act / 10);
    sample->humidity_avg = (uint8_t)((HumBuffer.buff / HumBuffer.cnt) / 10);
    sample->humidity_max = (uint8_t)(HumBuffer.max / 10);
    sample->humidity_min = (uint8_t)(HumBuffer.min / 10);
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
        buffer->max_ts_offset = ts - buffer->sample_ts;
    }

    if (val < buffer->min) {
        buffer->min = val;
        buffer->min_ts_offset = ts - buffer->sample_ts;
    }

    buffer->act = val;
    buffer->buff += (int32_t)val;
    buffer->cnt++;
}

static int64_t wlab_timestamp_utc_get(void) {
    int64_t uptime_sec = k_uptime_get() / 1000;
    return (UpdateTsSec + (uptime_sec - UpdateTsUptimeSec));
}

static bool wlab_timestamp_sync(void) {
    bool res = false;

    if (gsm_modem_get_ts_utc(&UpdateTsSec)) {
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
        if (wlab_timestamp_utc_get() < (SampleTsSec + PublishPeriodSec)) {
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