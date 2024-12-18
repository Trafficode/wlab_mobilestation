/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: gpio_sim800l.c
 * --------------------------------------------------------------------------*/
#include "periphery/gpio_sim800l.h"

#include <stdint.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

const struct gpio_dt_spec GsmResetPin =
    GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), gsmreset_gpios);

const struct gpio_dt_spec GsmDtrPin =
    GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), gsmdtr_gpios);

LOG_MODULE_REGISTER(GPIO_SIM800L, LOG_LEVEL_DBG);

void gpio_sim800l_init(void) {
    if (!device_is_ready(GsmResetPin.port)) {
        LOG_ERR("GsmResetPin not ready");
    }
    if (!device_is_ready(GsmDtrPin.port)) {
        LOG_ERR("GsmResetPin not ready");
    }

    gpio_pin_configure_dt(&GsmResetPin, GPIO_OUTPUT | GsmResetPin.dt_flags);
    gpio_pin_configure_dt(&GsmDtrPin, GPIO_OUTPUT | GsmDtrPin.dt_flags);
}

void gpio_sim800l_dtr_up(void) {
    gpio_pin_set_dt(&GsmDtrPin, 1);
}

void gpio_sim800l_dtr_down(void) {
    gpio_pin_set_dt(&GsmDtrPin, 0);
}

void gpio_sim800l_rst_up(void) {
    gpio_pin_set_dt(&GsmResetPin, 1);
}

void gpio_sim800l_rst_down(void) {
    gpio_pin_set_dt(&GsmResetPin, 0);
}

/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/