/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: gpio_status_led.c
 * --------------------------------------------------------------------------*/
#include "periphery/gpio_status_led.h"

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

static const struct gpio_dt_spec StatusLedPin =
    GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

void gpio_status_led_init(void) {
    gpio_pin_configure_dt(&StatusLedPin, GPIO_OUTPUT_HIGH);
}

void gpio_status_led_set_state(uint8_t led_on) {
    if (led_on) {
        gpio_pin_set_dt(&StatusLedPin, 0);
    } else {
        gpio_pin_set_dt(&StatusLedPin, 1);
    }
}

/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/