/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: gpio_user_btn.c
 * --------------------------------------------------------------------------*/
#include "periphery/gpio_user_btn.h"

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

static const struct gpio_dt_spec UserBtnDev =
    GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);

struct gpio_callback BtnIrqCb;

static void (*BtnIntEvtCb)(uint8_t state) = NULL;

static void gpio_callback(const struct device *dev, struct gpio_callback *cb,
                          uint32_t pin) {
    if (NULL != BtnIntEvtCb) {
        BtnIntEvtCb(gpio_pin_get_dt(&UserBtnDev));
    }
}

void gpio_user_btn_init(void) {
    gpio_pin_configure_dt(&UserBtnDev, GPIO_INPUT);
    gpio_pin_interrupt_configure_dt(&UserBtnDev, GPIO_INT_EDGE_BOTH);
    gpio_init_callback(&BtnIrqCb, gpio_callback, BIT(UserBtnDev.pin));
    gpio_add_callback(UserBtnDev.port, &BtnIrqCb);
}

void gpio_user_btn_register_cb(void (*cb)(uint8_t state)) {
    BtnIntEvtCb = cb;
}

/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/