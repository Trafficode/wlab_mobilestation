/* ---------------------------------------------------------------------------
 *  wlab_station
 * ---------------------------------------------------------------------------
 *  Name: nvs_data.h
 * --------------------------------------------------------------------------*/
#ifndef NVS_DATA_H_
#define NVS_DATA_H_

#include <stdint.h>
#include <zephyr/device.h>

#define NVS_PARTITION        storage_partition
#define NVS_PARTITION_DEVICE FIXED_PARTITION_DEVICE(NVS_PARTITION)
#define NVS_PARTITION_OFFSET FIXED_PARTITION_OFFSET(NVS_PARTITION)
#define NVS_PARTITION_SIZE   FIXED_PARTITION_SIZE(NVS_PARTITION)

#define NVS_ID_BOOT_COUNT      (1)
#define NVS_ID_MQTT_CONFIG     (2)
#define NVS_ID_WLAB_DEVICE_ID  (3)
#define NVS_ID_WLAB_PUB_PERIOD (4)
#define NVS_ID_APN_CONFIG      (5)

struct mqtt_config {
    char broker[CONFIG_BUFF_MAX_STRING_LEN];
    uint32_t port;
};

#define NVS_ID_APN_MAX_LEN (24)

struct apn_config {
    char apn[NVS_ID_APN_MAX_LEN];
    char user[NVS_ID_APN_MAX_LEN];
    char password[NVS_ID_APN_MAX_LEN];
};

/**
 * @brief Initialise none volatile storage space.
 *
 */
void nvs_data_init(void);

/**
 * @brief Read mqtt settings data, save in dst pointer.
 * Default, if not configured is filled with zeros.
 *
 * @param dst Pointer to save settings.
 */
void nvs_data_mqtt_config_get(struct mqtt_config *dst);

/**
 * @brief Save mqtt settings data.
 *
 * @param mqttconf Pointer with data do save
 * @return int 0 if success, -EIO otherwise
 */
int nvs_data_mqtt_config_set(struct mqtt_config *mqttconf);

/**
 * @brief Read wlab device id, if not exists, save UINT64_MAX
 *
 * @param device_id Pointer to save device_id.
 */
void nvs_data_wlab_device_id_get(uint64_t *device_id);

/**
 * @brief Save wlab device id.
 *
 * @param device_id Pointer with data do save
 * @return int 0 if success, -EIO otherwise
 */
int nvs_data_wlab_device_id_set(uint64_t *device_id);

/**
 * @brief Read wlab publish period.
 *
 * @param pub_period Destination of wlab gps position
 */
void nvs_data_wlab_pub_period_get(int64_t *pub_period);

/**
 * @brief Save wlab publish period.
 *
 * @param pub_period Pointer with data do save
 * @return int 0 if success, -EIO otherwise
 */
int nvs_data_wlab_pub_period_set(int64_t *pub_period);

/**
 * @brief Get APN configuration
 * Default apn is "TM" user "", password ""
 * @param apnconf 
 */
void nvs_data_apn_config_get(struct apn_config *apnconf);

/**
 * @brief Set APN configuration, maximum length
 * for all parameters is 23 chars
 * 
 * @param apnconf 
 * @return int 0 if success, -EIO otherwise
 */
int nvs_data_apn_config_set(struct apn_config *apnconf);

#endif /* NVS_DATA_H_ */
/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/