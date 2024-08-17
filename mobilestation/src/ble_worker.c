/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: ble_worker.c
 * --------------------------------------------------------------------------*/
#include "ble_worker.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/services/nus.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/sys/util.h>

#include "nvs_data.h"
#include "periphery/gpio_status_led.h"
#include "version.h"

LOG_MODULE_REGISTER(BLEW, LOG_LEVEL_DBG);

static void ble_worker_proc(void *, void *, void *);
#define BLE_WORKER_CMD_STACK_SIZE (2 * 1024)
#define BLE_WORKER_CMD_PRIORITY   (5)
K_THREAD_DEFINE(BleWorkerTid, BLE_WORKER_CMD_STACK_SIZE, ble_worker_proc, NULL,
                NULL, NULL, BLE_WORKER_CMD_PRIORITY, 0, 0);
K_SEM_DEFINE(NusReceivedSem, 0, 1);

static struct bt_conn *CurrentConn = NULL;

static char NusReceivedBuf[128];
static char NusAnswerBuf[1024];

static void connected(struct bt_conn *conn, uint8_t err) {
    if (err) {
        LOG_ERR("Connection failed (err %u)", err);
        return;
    }

    CurrentConn = bt_conn_ref(conn);
    LOG_INF("Connected");
}

static void disconnected(struct bt_conn *conn, uint8_t reason) {
    LOG_ERR("Disconnected (reason %u)", reason);

    CurrentConn = NULL;
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};

static void nus_notif_enabled(bool enabled, void *ctx) {
    ARG_UNUSED(ctx);

    LOG_INF("%s() - %s", __func__, (enabled ? "Enabled" : "Disabled"));
}

static void nus_received(struct bt_conn *conn, const void *data, uint16_t len,
                         void *ctx) {
    size_t message_len = MIN(sizeof(NusReceivedBuf) - 1, len);

    ARG_UNUSED(conn);
    ARG_UNUSED(ctx);

    if (message_len > 0) {
        memcpy(NusReceivedBuf, data, message_len);
        NusReceivedBuf[message_len] = '\0';
        k_sem_give(&NusReceivedSem);
    }
}

struct bt_nus_cb NusListener = {
    .notif_enabled = nus_notif_enabled,
    .received = nus_received,
};

static void ble_worker_proc(void *, void *, void *) {
    char reboot[] = "reboot";
    char pconfig[] = "pconfig";
    char apn[] = "apn";
    char mqttconf[] = "mqttconf";
    char pubp[] = "pubp";
    char help[] = "help";

    while (true) {
        if (0 != k_sem_take(&NusReceivedSem, K_MSEC(16 * 1000))) {
            continue;
        }

        size_t answer_len = 0;
        if (!memcmp(NusReceivedBuf, reboot, sizeof(reboot) - 1)) {
            // Reboot device
            sys_reboot(SYS_REBOOT_COLD);
        } else if (!memcmp(NusReceivedBuf, help, sizeof(help) - 1)) {
            uint32_t ver = sys_kernel_version_get();
            answer_len += sprintf(
                NusAnswerBuf + answer_len, "Firmware: v%d Zephyr: %u.%u.%u\n",
                FIRMWARE_VERSION, SYS_KERNEL_VER_MAJOR(ver),
                SYS_KERNEL_VER_MINOR(ver), SYS_KERNEL_VER_PATCHLEVEL(ver));
            answer_len +=
                sprintf(NusAnswerBuf + answer_len, "pubp <minutes>\n");
            answer_len += sprintf(NusAnswerBuf + answer_len,
                                  "mqttconf <broker_ip> <port>\n");
            answer_len += sprintf(NusAnswerBuf + answer_len,
                                  "apn <name> <user> <pass>\napn <name>\n");
            answer_len += sprintf(NusAnswerBuf + answer_len, "pconfig\n");
            answer_len += sprintf(NusAnswerBuf + answer_len, "help\n");
        } else if (!memcmp(NusReceivedBuf, pubp, sizeof(pubp) - 1)) {
            uint64_t pub_period = 0;
            uint32_t pubp_value = 0;
            int res = sscanf(NusReceivedBuf, "pubp %u", &pubp_value);
            if (1 == res) {
                pub_period = (uint64_t)pubp_value;
                nvs_data_wlab_pub_period_set(&pub_period);
                answer_len += sprintf(NusAnswerBuf + answer_len,
                                      "Use \"pconfig\" to verify\n");
            } else {
                answer_len += sprintf(NusAnswerBuf + answer_len,
                                      "Failed! Use \"help\"\n");
            }
        } else if (!memcmp(NusReceivedBuf, mqttconf, sizeof(mqttconf) - 1)) {
            struct mqtt_config mqttcfg = {};
            int res = sscanf(NusReceivedBuf, "mqttconf %s %u", mqttcfg.broker,
                             &mqttcfg.port);
            if (2 == res) {
                nvs_data_mqtt_config_set(&mqttcfg);
                answer_len += sprintf(NusAnswerBuf + answer_len,
                                      "Use \"pconfig\" to verify\n");
            } else {
                answer_len += sprintf(NusAnswerBuf + answer_len,
                                      "Failed! Use \"help\"\n");
            }
        } else if (!memcmp(NusReceivedBuf, apn, sizeof(apn) - 1)) {
            struct apn_config apn = {};
            int res = sscanf(NusReceivedBuf, "apn %s %s %s", apn.apn, apn.user,
                             apn.password);
            if (1 == res || 3 == res) {
                nvs_data_apn_config_set(&apn);
                answer_len += sprintf(NusAnswerBuf + answer_len,
                                      "Use \"pconfig\" to verify\n");
            } else {
                answer_len += sprintf(NusAnswerBuf + answer_len,
                                      "Failed! Use \"help\"\n");
            }
        } else if (!memcmp(NusReceivedBuf, pconfig, sizeof(pconfig) - 1)) {
            struct mqtt_config mqttcfg = {};
            uint64_t device_id = 0;
            uint64_t pub_period = 0;
            struct apn_config apn = {};

            nvs_data_mqtt_config_get(&mqttcfg);
            nvs_data_wlab_device_id_get(&device_id);
            nvs_data_wlab_pub_period_get(&pub_period);
            nvs_data_apn_config_get(&apn);

            answer_len += sprintf(NusAnswerBuf + answer_len,
                                  "mqtt_broker: <%s>\n", mqttcfg.broker);
            answer_len += sprintf(NusAnswerBuf + answer_len, "mqtt_port: %u\n",
                                  mqttcfg.port);
            answer_len +=
                sprintf(NusAnswerBuf + answer_len, "device_id: %04X%08X\n",
                        (uint32_t)(device_id >> 32), (uint32_t)(device_id));
            answer_len +=
                sprintf(NusAnswerBuf + answer_len, "publish_period: %u[mins]\n",
                        (uint32_t)pub_period);
            answer_len += sprintf(NusAnswerBuf + answer_len,
                                  "apn <%s>, user <%s>, password <%s>\n",
                                  apn.apn, apn.user, apn.password);
        } else {
            answer_len +=
                sprintf(NusAnswerBuf + answer_len, "Command not found");
        }

        if (answer_len > 0) {
            bt_nus_send(NULL, NusAnswerBuf, answer_len);
        }
    }
}

void ble_worker_init(int32_t adv_time_sec) {
    char ble_adv_name[16];
    uint64_t device_id = 0;

    nvs_data_wlab_device_id_get(&device_id);

    size_t ble_adv_name_len =
        sprintf(ble_adv_name, "WLAB_%X", ((uint32_t)device_id) & 0xFFFF);

    struct bt_data ad[] = {
        BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
        BT_DATA(BT_DATA_NAME_COMPLETE, ble_adv_name, ble_adv_name_len),
    };

    int err = bt_nus_cb_register(&NusListener, NULL);
    if (err) {
        LOG_ERR("Failed to register NUS callback: %d", err);
        return;
    }

    err = bt_enable(NULL);
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)", err);
        goto DONE;
    }

    LOG_INF("Bluetooth initialized");

    err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        LOG_ERR("Advertising failed to start (err %d)", err);
        goto DONE;
    }

    int64_t adv_stop_ts =
        k_uptime_get() + INT64_C(1000) * (int64_t)adv_time_sec;

    do {
        k_sleep(K_MSEC(400));
        gpio_status_led_set_state(1);
        k_sleep(K_MSEC(2));
        gpio_status_led_set_state(0);
    } while ((adv_stop_ts > k_uptime_get()) || (CurrentConn != NULL));

    bt_le_adv_stop();
DONE:
    return;
}

/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/