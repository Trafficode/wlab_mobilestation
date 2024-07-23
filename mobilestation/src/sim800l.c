/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: sim800l.c
 * --------------------------------------------------------------------------*/
#include "sim800l.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#include "uart_gsm.h"

const struct gpio_dt_spec GsmResetPin =
    GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), gsmreset_gpios);

static uint8_t GsmComBuffer[1024];

void gsm_modem_init(void) {
    if (!device_is_ready(GsmResetPin.port)) {
        LOG_ERR("GsmResetPin not ready");
    }

    gpio_pin_configure_dt(&GsmResetPin, GPIO_OUTPUT_HIGH);
}

void gsm_modem_reset(void) {
    // # AT&F                  Factory settings
    // "OK"
}

void gsm_modem_config(void) {
    // # ATE0                  Echo OFF
    // "OK"

    // # AT+CLTS=1             Enable the network time synchronization
    // "OK"
}

bool gsm_modem_net_wakeup(void) {
    bool result = false;

    return (result);
}

bool gsm_modem_net_sleep(void) {
    bool result = false;

    return (result);
}

bool gsm_modem_net_setup(void) {
    bool result = false;
    // # AT+CGATT=1            Attach to GPRS service
    // "OK"

    // # AT+CIPSHUT            Reset IP session
    // "SHUT OK"

    // # AT+CIPSTATUS          Check IP status
    // "IP INITIAL" ??

    // # AT+CIPMUX=0           Set single connection mode
    // "OK"

    // # AT+CSTT="TM","",""    Set APN's
    // "OK"

    // # AT+CIICR              Bring up wireless connection
    // "OK"

    // # AT+CIFSR              Get IP address (expecting a non-empty response)
    // 10.0.0.0

    // # AT+CREG?              Check gprs conection expected response:
    // "+CREG: 0,1" "+CREG: 0,5"
    return (result);
}

bool gsm_modem_mqtt_connect(const char *domain, uint32_t port,
                            int64_t timeout) {
    bool result = false;
    // AT+CIPSTART="TCP","194.42.111.14","1883"
    // "CONNECT OK"
    return (result);
}

bool gsm_modem_mqtt_publish(const char *topic, uint8_t *data, size_t len) {
    ;
}

void gsm_modem_mqtt_close(void) {
    ;
}

/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/