/* ---------------------------------------------------------------------------
 *  wlab_station
 * ---------------------------------------------------------------------------
 *  Name: nvs_data.c
 * --------------------------------------------------------------------------*/
#include "nvs_data.h"

#include <stdint.h>
#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/logging/log.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/sys/util.h>

#include "wlab.h"

LOG_MODULE_REGISTER(NVSD, LOG_LEVEL_DBG);

static struct nvs_fs Fs = {0};

void nvs_data_init(void) {
    int ret = 0;
    struct flash_pages_info info = {0};

    Fs.flash_device = NVS_PARTITION_DEVICE;
    ret = device_is_ready(Fs.flash_device);
    __ASSERT((ret != 0), "Flash not ready");

    Fs.offset = NVS_PARTITION_OFFSET;
    ret = flash_get_page_info_by_offs(Fs.flash_device, Fs.offset, &info);
    __ASSERT((0 == ret), "Unable to get page info err %d", ret);

    LOG_INF("NVS sector size %u part size %u", info.size, NVS_PARTITION_SIZE);

    Fs.sector_size = info.size;
    Fs.sector_count = NVS_PARTITION_SIZE / info.size;

    ret = nvs_mount(&Fs);
    __ASSERT((0 == ret), "Flash mount failed");

    uint32_t boot_counter = UINT32_C(0);
    size_t area_len = sizeof(boot_counter);
    ret = nvs_read(&Fs, NVS_ID_BOOT_COUNT, &boot_counter, area_len);
    if (ret > 0) { /* item was found, show it */
        LOG_INF("boot counter: %d", boot_counter);
    } else {       /* item was not found, add it */
        LOG_INF("No boot counter found, adding it at id %d", NVS_ID_BOOT_COUNT);
    }
    boot_counter++;

    if (area_len ==
        nvs_write(&Fs, NVS_ID_BOOT_COUNT, &boot_counter, area_len)) {
        LOG_INF("Save boot counter %d succ", boot_counter);
    } else {
        LOG_ERR("Save boot counter %d err", boot_counter);
    }
}

void nvs_data_mqtt_config_get(struct mqtt_config *mqttconf) {
    __ASSERT((mqttconf != NULL), "Null pointer passed");
    size_t mqtt_config_len = sizeof(struct mqtt_config);

    int ret = nvs_read(&Fs, NVS_ID_MQTT_CONFIG, mqttconf, mqtt_config_len);
    if (ret <= 0) {
        LOG_WRN("No mqtt config found, clear...");
        memset(mqttconf, 0x00, mqtt_config_len);
        if (mqtt_config_len ==
            nvs_write(&Fs, NVS_ID_MQTT_CONFIG, mqttconf, mqtt_config_len)) {
            LOG_DBG("Mqtt config cleared");
        } else {
            LOG_ERR("Mqtt config clear failed");
        }
    }
}

int nvs_data_mqtt_config_set(struct mqtt_config *mqttconf) {
    __ASSERT((mqttconf != NULL), "Null pointer passed");
    int ret = 0;
    size_t mqtt_config_len = sizeof(struct mqtt_config);
    if (mqtt_config_len ==
        nvs_write(&Fs, NVS_ID_MQTT_CONFIG, mqttconf, mqtt_config_len)) {
        LOG_DBG("Mqtt config set success");
    } else {
        LOG_ERR("Mqtt config set failed");
        ret = -EIO;
    }
    return (ret);
}

void nvs_data_wlab_device_id_get(uint64_t *device_id) {
    __ASSERT((device_id != NULL), "Null pointer passed");
    size_t wlab_device_id_len = sizeof(uint64_t);

    int ret =
        nvs_read(&Fs, NVS_ID_WLAB_DEVICE_ID, device_id, wlab_device_id_len);
    if (ret <= 0) {
        LOG_WRN("No device id found, clear...");
        memset(device_id, 0x00, wlab_device_id_len);
        if (wlab_device_id_len == nvs_write(&Fs, NVS_ID_WLAB_DEVICE_ID,
                                            device_id, wlab_device_id_len)) {
            LOG_DBG("Wlab device id clear success");
        } else {
            LOG_ERR("Wlab device id clear failed");
        }
    }
}

int nvs_data_wlab_device_id_set(uint64_t *device_id) {
    __ASSERT((device_id != NULL), "Null pointer passed");
    size_t wlab_device_id_len = sizeof(uint64_t);

    int ret = 0;
    if (wlab_device_id_len ==
        nvs_write(&Fs, NVS_ID_WLAB_DEVICE_ID, device_id, wlab_device_id_len)) {
        LOG_DBG("Wlab device id set success");
    } else {
        LOG_ERR("Wlab device id set failed");
        ret = -EIO;
    }
    return (ret);
}

void nvs_data_wlab_pub_period_get(int64_t *pub_period) {
    __ASSERT((pub_period != NULL), "Null pointer passed");
    size_t wlab_pub_period_len = sizeof(int64_t);

    int ret =
        nvs_read(&Fs, NVS_ID_WLAB_PUB_PERIOD, pub_period, wlab_pub_period_len);
    if (ret <= 0) {
        LOG_WRN("No wlab publish period found, default");
        memset(pub_period, WLAB_DEFAULT_PUB_PERIOD_MIN, wlab_pub_period_len);
        if (wlab_pub_period_len == nvs_write(&Fs, NVS_ID_WLAB_PUB_PERIOD,
                                             pub_period, wlab_pub_period_len)) {
            LOG_DBG("Wlab device id clear success");
        } else {
            LOG_ERR("Wlab device id clear failed");
        }
    }
}

int nvs_data_wlab_pub_period_set(int64_t *pub_period) {
    __ASSERT((pub_period != NULL), "Null pointer passed");
    size_t wlab_pub_period_len = sizeof(int64_t);

    int ret = 0;
    if (wlab_pub_period_len == nvs_write(&Fs, NVS_ID_WLAB_PUB_PERIOD,
                                         pub_period, wlab_pub_period_len)) {
        LOG_DBG("Wlab device id set success");
    } else {
        LOG_ERR("Wlab device id set failed");
        ret = -EIO;
    }
    return (ret);
}

/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/