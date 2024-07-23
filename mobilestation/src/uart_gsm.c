/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: uart_gsm.c
 * --------------------------------------------------------------------------*/
#include "uart_gsm.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>

static const struct device *const UartDev = DEVICE_DT_GET(DT_ALIAS(uart_gsm));

void uart_gsm_init(void) {}

void uart_gsm_send(uint8_t *tx_data, size_t tx_len) {
    uart_tx(UartDev, tx_data, tx_len, 0);
}

bool uart_gsm_read_bytes(uint8_t *rx_data, size_t rx_len, int64_t timeout) {
    uint8_t rchar = 0;
    size_t read_len = 0;
    bool result = false;
    int64_t start_ts = k_uptime_get();

    while (start_ts + timeout < k_uptime_get()) {
        if (0 != uart_poll_in(UartDev, &rchar)) {
            k_sleep(K_MSEC(1));
            continue;
        }

        rx_data[read_len++] = rchar;

        if (read_len == rx_len) {
            result = true;
            break;
        }
    }

    return (result);
}

void uart_gsm_rx_clear(void) {
    uint8_t rchar = 0;
    while (0 == uart_poll_in(UartDev, &rchar)) {
        ;   // skip read data
    }
}

bool uart_gsm_read_line(char *rx_line, size_t *rx_len, int64_t timeout) {
    uint8_t rchar = 0;
    size_t read_len = 0;
    bool result = false;
    int64_t start_ts = k_uptime_get();

    while (start_ts + timeout < k_uptime_get()) {
        if (0 != uart_poll_in(UartDev, &rchar)) {
            k_sleep(K_MSEC(1));
            continue;
        }

        if ((rchar == '\n' || rchar == '\r') && (0 == read_len)) {
            // Skip first new line characters
            continue;
        }

        if (rchar == '\n' || rchar == '\r') {
            // End of line got
            *rx_len = read_len;
            result = true;
            break;
        }

        if (rchar >= ' ' || rchar <= '~') {
            rx_line[read_len++] = rchar;
        }
    }

    return (result);
}

/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/