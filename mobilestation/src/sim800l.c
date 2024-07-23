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

static uint8_t GsmSendBuffer[1024];
static uint8_t GsmReadBuffer[1024];

void gsm_modem_init(void) {
    if (!device_is_ready(GsmResetPin.port)) {
        LOG_ERR("GsmResetPin not ready");
    }

    gpio_pin_configure_dt(&GsmResetPin, GPIO_OUTPUT_HIGH);
}

bool gsm_modem_reset(void) {
    size_t len = 0;
    bool res = false;
    // # AT&F                  Factory settings
    // "OK"
    char at_f[] = "\rAT&F\r";
    uart_gsm_send(at_f, sizeof(at_f) - 1);
    res = uart_gsm_read_line(GsmReadBuffer, &len, INT64_C(1000));
    if (res) {
        if (0 != strstr(GsmReadBuffer, "OK")) {
            res = false;
        }
    }
    return (res);
}

bool gsm_modem_cmd_base(const char *at, size_t at_len, const char *expected,
                        int64_t timeout) {
    bool res = false;
    int64_t start_ts = k_uptime_get();

    uart_gsm_rx_clear();
    uart_gsm_send(at, sizeof(at) - 1);
    int64_t rl_timeout = timeout;
    do {
        res = uart_gsm_read_line(GsmReadBuffer, &at_len, timeout);
        if (res) {
            if (0 == strstr(GsmReadBuffer, expected)) {
                break;
            } else if (0 == strstr(GsmReadBuffer, "ERROR")) {
                res = false;
                break;
            } else {
                // TODO Test!
                // int64_t elapsed = k_uptime_get() - start_ts;
                // if (0 < rl_timeout - elapsed) {
                //     rl_timeout -= elapsed;
                // }
                res = false;
            }
        }
    } while (start_ts + timeout > k_uptime_get());

    return (res);
}

void gsm_modem_config(void) {
    size_t len = 0;
    bool res = false;
    char at_e0[] = "\rATE0\r";
    char at_ciurc[] = "\rAT+CIURC=0\r";
    char at_cnmi[] = "\rAT+CNMI=0,0,0,0,0\r";
    char at_clip[] = "\rAT+CLIP=0\r";
    char at_colp[] = "\rAT+COLP=0\r";
    char at_creg[] = "\rAT+CREG=0\r";
    char at_cgreg[] = "\rAT+CGREG=0\r";
    char at_clts[] = "\rAT+CLTS=1\r";

    // # ATE0                  Echo OFF
    // "OK"
    if (!gsm_modem_cmd_base(at_e0, sizeof(at_e0), "OK", INT64_C(1000))) {
        goto DONE;
    }

    // AT+CIURC=0   # Disable "Call Ready" message
    // OK
    if (!gsm_modem_cmd_base(at_ciurc, sizeof(at_ciurc), "OK", INT64_C(1000))) {
        goto DONE;
    }

    // AT+CNMI=0,0,0,0,0  # Disable SMS notifications
    // OK
    if (!gsm_modem_cmd_base(at_cnmi, sizeof(at_cnmi), "OK", INT64_C(1000))) {
        goto DONE;
    }

    // AT+CLIP=0    # Disable incoming call indications
    // OK
    if (!gsm_modem_cmd_base(at_clip, sizeof(at_clip), "OK", INT64_C(1000))) {
        goto DONE;
    }

    // AT+COLP=0    # Disable connected line identification
    // OK
    if (!gsm_modem_cmd_base(at_colp, sizeof(at_colp), "OK", INT64_C(1000))) {
        goto DONE;
    }

    // AT+CREG=0    # Disable unsolicited network registration information
    // OK
    if (!gsm_modem_cmd_base(at_creg, sizeof(at_creg), "OK", INT64_C(1000))) {
        goto DONE;
    }

    // AT+CGREG=0   # Disable unsolicited GPRS network status
    // OK
    if (!gsm_modem_cmd_base(at_cgreg, sizeof(at_cgreg), "OK", INT64_C(1000))) {
        goto DONE;
    }

    // # AT+CLTS=1  Enable the network time synchronization
    // "OK"
    if (!gsm_modem_cmd_base(at_cgreg, sizeof(at_cgreg), "OK", INT64_C(1000))) {
        goto DONE;
    }

    res = true;
DONE:
    return (res);
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
    bool res = false;
    char at_cgatt[] = "\rAT+CGATT=1\r";
    char at_cipshut[] = "\rAT+CIPSHUT\r";
    char at_cipstatus[] = "\rAT+CIPSTATUS\r";
    char at_cipmux[] = "\rAT+CIPMUX=0\r";
    char at_cipmux[] = "\rAT+CIPMUX=0\r";
    char at_cstt[] = "\rAT+CSTT=\"TM\",\"\",\"\"\r";
    char at_ciicr[] = "\rAT+CIICR\r";
    char at_creg[] = "\rAT+CREG?\r";

    // # AT+CGATT=1            Attach to GPRS service
    // "OK"
    if (!gsm_modem_cmd_base(at_cgatt, sizeof(at_cgatt), "OK", INT64_C(4000))) {
        goto DONE;
    }

    // # AT+CIPSHUT            Reset IP session
    // "SHUT OK"
    if (!gsm_modem_cmd_base(at_cipshut, sizeof(at_cipshut), "SHUT OK",
                            INT64_C(4000))) {
        goto DONE;
    }

    // # AT+CIPSTATUS          Check IP status
    // "IP INITIAL" ??
    if (!gsm_modem_cmd_base(at_cipstatus, sizeof(at_cipstatus), "IP INITIAL",
                            INT64_C(4000))) {
        goto DONE;
    }

    // # AT+CIPMUX=0           Set single connection mode
    // "OK"
    if (!gsm_modem_cmd_base(at_cipmux, sizeof(at_cipmux), "OK",
                            INT64_C(2000))) {
        goto DONE;
    }

    // # AT+CSTT="TM","",""    Set APN's
    // "OK"
    if (!gsm_modem_cmd_base(at_cstt, sizeof(at_cstt), "OK", INT64_C(2000))) {
        goto DONE;
    }

    // # AT+CIICR              Bring up wireless connection
    // "OK"
    if (!gsm_modem_cmd_base(at_ciicr, sizeof(at_ciicr), "OK", INT64_C(4000))) {
        goto DONE;
    }

    // # AT+CREG?              Check gprs conection expected response:
    // "+CREG: 0,1" "+CREG: 0,5"
    if (!gsm_modem_cmd_base(at_creg, sizeof(at_creg), "+CREG: 0,5",
                            INT64_C(2000))) {
        goto DONE;
    }

    // # AT+CIFSR              Get IP address (expecting a non-empty response)
    // 10.0.0.0

    res = true;
DONE:
    return (result);
}

bool gsm_modem_mqtt_connect(const char *domain, uint32_t port,
                            int64_t timeout) {
    char at_cipstart[] = "\rAT+CIPSTART=\"TCP\",\"194.42.111.14\",\"1883\"\r";
    // AT+CIPSTART="TCP","194.42.111.14","1883"
    // "CONNECT OK"
    return gsm_modem_cmd_base(at_cipstart, sizeof(at_cipstart), "CONNECT OK",
                              INT64_C(4000));
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