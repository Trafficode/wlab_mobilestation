/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: uart_gsm.c
 * --------------------------------------------------------------------------*/
#include "periphery/uart_gsm.h"

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

static const struct device *const UartDev = DEVICE_DT_GET(DT_ALIAS(uart_gsm));

LOG_MODULE_REGISTER(GSMU, LOG_LEVEL_DBG);

void uart_gsm_init(void) {
    ;   // nothing to initialize
}

bool uart_gsm_getc(uint8_t *chr, int32_t timeout) {
    bool res = false;
    int64_t start_ts = k_uptime_get();

    do {
        if (0 == uart_poll_in(UartDev, chr)) {
            res = true;
            break;
        }
    } while ((start_ts + (int64_t)timeout) > k_uptime_get());

    return (res);
}

void uart_gsm_send(uint8_t *tx_data, size_t tx_len) {
    for (size_t idx = 0; idx < tx_len; idx++) {
        uart_poll_out(UartDev, tx_data[idx]);
    }
}

bool uart_gsm_read_bytes(uint8_t *rx_data, size_t exp_len, int32_t timeout) {
    uint8_t rchar = 0;
    size_t read_len = 0;
    bool res = false;
    int64_t start_ts = k_uptime_get();

    do {
        while (uart_gsm_getc(&rchar, INT32_C(10))) {
            rx_data[read_len++] = rchar;
            if (read_len == exp_len) {
                res = true;
                break;
            }
        }
    } while (!res && (start_ts + (int64_t)timeout) > k_uptime_get());

    return (res);
}

void uart_gsm_rx_clear(void) {
    uint8_t rchar = 0;
    while (uart_gsm_getc(&rchar, INT32_C(10))) {
        ;   // skip read data
    }
}

bool uart_gsm_read_line(char *rx_line, size_t max_len, int32_t timeout) {
    uint8_t rchar = 0;
    size_t read_len = 0;
    bool res = false;
    int64_t start_ts = k_uptime_get();

    max_len--;   // \0 has to be added at the end of line to make it as string
    do {
        while (uart_gsm_getc(&rchar, INT32_C(10))) {
            if ((rchar == '\n' || rchar == '\r') && (0 == read_len)) {
                // Skip first new line characters
                continue;
            }

            if (('\n' == rchar) || ('\r' == rchar)) {
                // End of line got
                rx_line[read_len] = '\0';   // append end of string
                res = true;
                break;
            }

            if ((rchar >= ' ') || (rchar <= '~')) {
                rx_line[read_len++] = rchar;
                if (read_len == max_len) {
                    break;
                }
            }
        }
    } while (!res && (start_ts + (int64_t)timeout) > k_uptime_get());

    return (res);
}

/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/