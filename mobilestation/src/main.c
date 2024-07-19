/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: main.c
 * --------------------------------------------------------------------------*/
#include <stdint.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/kernel_version.h>
#include <zephyr/logging/log.h>

#include "nvs_data.h"
#include "version.h"

LOG_MODULE_REGISTER(MAIN, LOG_LEVEL_DBG);

int main(void) {
    uint32_t ver = sys_kernel_version_get();

    LOG_INF("Board: %s", CONFIG_BOARD);
    LOG_INF("Firmware: %s Zephyr: %u.%u.%u", FIRMWARE_VERSION,
            SYS_KERNEL_VER_MAJOR(ver), SYS_KERNEL_VER_MINOR(ver),
            SYS_KERNEL_VER_PATCHLEVEL(ver));

    nvs_data_init();

    const struct device *const dev = DEVICE_DT_GET_ONE(sensirion_sht3xd);
    int rc;

    if (!device_is_ready(dev)) {
        LOG_ERR("Device %s is not ready\n", dev->name);
    }

    struct sensor_value temp, hum;

    for (;;) {
        LOG_INF("Main loop, systick = %" PRIi64, k_uptime_get());

        rc = sensor_sample_fetch(dev);
        if (rc == 0) {
            rc = sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
        }

        if (rc == 0) {
            rc = sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &hum);
        }

        if (rc != 0) {
            LOG_ERR("SHT3XD: failed: %d\n", rc);
        }

        LOG_INF("SHT3XD: %.2f Cel ; %0.2f %%RH\n",
                sensor_value_to_double(&temp), sensor_value_to_double(&hum));

        k_sleep(K_MSEC(2000));
    }
    return 0;
}

/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/