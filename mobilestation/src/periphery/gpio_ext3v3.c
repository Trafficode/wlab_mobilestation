/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: gpio_ext3v3.c
 * --------------------------------------------------------------------------*/
#include "periphery/gpio_ext3v3.h"

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

const struct gpio_dt_spec External3v3Pin =
    GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), external3v3_gpios);

LOG_MODULE_REGISTER(GPIO_EXT3V3, LOG_LEVEL_DBG);

void gpio_ext3v3_init(void) {
    if (!device_is_ready(External3v3Pin.port)) {
        LOG_ERR("External3v3Pin enable not ready");
    }

    gpio_pin_configure_dt(&External3v3Pin, GPIO_OUTPUT_HIGH);
}

void gpio_ext3v3_enable(void) {
    gpio_pin_set_dt(&External3v3Pin, 1);
}

void gpio_ext3v3_disable(void) {
    gpio_pin_set_dt(&External3v3Pin, 0);
}

/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/