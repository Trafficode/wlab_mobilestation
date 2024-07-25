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
#include "wlab.h"

LOG_MODULE_REGISTER(MAIN, LOG_LEVEL_DBG);

// status led
// reset button

int main(void) {
    uint32_t ver = sys_kernel_version_get();

    LOG_INF("Board: %s", CONFIG_BOARD);
    LOG_INF("Firmware: %s Zephyr: %u.%u.%u", FIRMWARE_VERSION,
            SYS_KERNEL_VER_MAJOR(ver), SYS_KERNEL_VER_MINOR(ver),
            SYS_KERNEL_VER_PATCHLEVEL(ver));

    nvs_data_init();

    wlab_init();

    for (;;) {
        LOG_INF("Main loop, systick = %" PRIi64, k_uptime_get());
        wlab_proc();
        k_sleep(K_MSEC(2000));
    }
    return 0;
}

/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/