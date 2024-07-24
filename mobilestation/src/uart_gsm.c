/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: uart_gsm.c
 * --------------------------------------------------------------------------*/
#include "uart_gsm.h"

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

void uart_gsm_init(void) {}

void uart_gsm_send(uint8_t *tx_data, size_t tx_len) {
    int rc = (UartDev, tx_data, tx_len, 0);
    LOG_INF("rc = %d", rc);
}

bool uart_gsm_read_bytes(uint8_t *rx_data, size_t exp_len, int32_t timeout) {
    uint8_t rchar = 0;
    size_t read_len = 0;
    bool result = false;
    int64_t start_ts = k_uptime_get();

    while ((start_ts + (int64_t)timeout) < k_uptime_get()) {
        if (0 != uart_poll_in(UartDev, &rchar)) {
            k_sleep(K_MSEC(1));
            continue;
        }

        rx_data[read_len++] = rchar;

        if (read_len == exp_len) {
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

bool uart_gsm_read_line(char *rx_line, size_t max_len, int32_t timeout) {
    uint8_t rchar = 0;
    size_t read_len = 0;
    bool result = false;
    int64_t start_ts = k_uptime_get();

    max_len--;   // \0 has to be added at the end of line to make it as string
    while (start_ts + (int64_t)timeout < k_uptime_get()) {
        int32_t chrcnt = 0;   // 100ms timeout for character
        while (chrcnt < 100) {
            if (0 != uart_poll_in(UartDev, &rchar)) {
                k_sleep(K_MSEC(1));
                chrcnt++;
            } else {
                break;
            }
        }

        if (100 == chrcnt) {
            continue;
        }

        if ((rchar == '\n' || rchar == '\r') && (0 == read_len)) {
            // Skip first new line characters
            continue;
        }

        if (rchar == '\n' || rchar == '\r') {
            // End of line got
            rx_line[read_len] = '\0';   // append end of string
            result = true;
            break;
        }

        if (rchar >= ' ' || rchar <= '~') {
            rx_line[read_len++] = rchar;
            if (read_len == max_len) {
                break;
            }
        }
    }

    return (result);
}

/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/