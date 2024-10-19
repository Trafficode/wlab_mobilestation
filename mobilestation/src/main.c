/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: main.c
 * --------------------------------------------------------------------------*/
#include <debug/cpu_load.h>
#include <inttypes.h>
#include <stdint.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/kernel_version.h>
#include <zephyr/logging/log.h>

#include "ble_worker.h"
#include "nvs_data.h"
#include "periphery/adc_battery_vol.h"
#include "periphery/gpio_ext3v3.h"
#include "periphery/gpio_status_led.h"
#include "periphery/gpio_user_btn.h"
#include "sim800l.h"
#include "version.h"
#include "wlab.h"

LOG_MODULE_REGISTER(MAIN, LOG_LEVEL_DBG);

int main(void) {
    uint32_t ver = sys_kernel_version_get();

    LOG_INF("Board: %s", CONFIG_BOARD);
    LOG_INF("Firmware: v%d Zephyr: %u.%u.%u", FIRMWARE_VERSION,
            SYS_KERNEL_VER_MAJOR(ver), SYS_KERNEL_VER_MINOR(ver),
            SYS_KERNEL_VER_PATCHLEVEL(ver));
    cpu_load_init();

    // Need to be as one of the first, modem is entered into sleep
    // mode to avoid ble radio problems and save energy while ble
    // connection ongoing
    gsm_modem_init();

    gpio_status_led_init();
    gpio_status_led_set_state(0);
    gpio_user_btn_init();
    adc_battery_vol_init();
    gpio_ext3v3_init();   // Enable external 3v3 by default
    nvs_data_init();

    ble_worker_init(20);
    wlab_init();

    for (;;) {
        wlab_proc();
        uint32_t cpu_load = cpu_load_get();
        LOG_WRN("CPU load: %u.%03u%%", cpu_load / 1000, cpu_load % 1000);
        cpu_load_reset();
        k_sleep(K_MSEC(10 * 1000));
    }
    return 0;
}

/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/