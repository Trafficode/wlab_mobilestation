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

bool gsm_modem_cipsend(uint8_t *data, size_t len, int32_t timeout) {
    // AT+CIPSEND
    // 0a 3e 20 - "\n> "
    // data has to be ended with 0x1A
    bool res = false;
    int64_t start_ts = k_uptime_get();

    uart_gsm_rx_clear();

    uint8_t read_buffer[4];
    const char *at_cipsend = "\nAT+CIPSEND\n";
    uart_gsm_send(at_cipsend, sizeof(at_cipsend) - 1);
    if (!uart_gsm_read_bytes(read_buffer, 3, 1000)) {
        goto DONE;
    } else {
        const uint8_t expected[] = {'\n', '>', ' '};
        if (0 != memcmp(read_buffer, expected, 3)) {
            goto DONE;
        }
    }

    if (!gsm_modem_cmd_base(data, len, "SEND_OK", timeout)) {
        goto DONE;
    }

    res = true;
DONE:
    return (res);
}

bool gsm_modem_cmd_base(uint8_t *data, size_t len, const char *expected,
                        int32_t timeout) {
    bool res = false;
    size_t read_len = 0;
    int64_t start_ts = k_uptime_get();

    uart_gsm_rx_clear();
    uart_gsm_send(data, len);
    int64_t rl_timeout = timeout;
    do {
        res = uart_gsm_read_line(GsmReadBuffer, &read_len, (int64_t)timeout);
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

    // # ATE0                  Echo OFF
    // "OK"
    char at_e0[] = "\nATE0\n";
    if (!gsm_modem_cmd_base(at_e0, sizeof(at_e0), "OK", 100)) {
        goto DONE;
    }

    // AT+CIURC=0   # Disable "Call Ready" message
    // OK
    char at_ciurc[] = "\nAT+CIURC=0\n";
    if (!gsm_modem_cmd_base(at_ciurc, sizeof(at_ciurc), "OK", 100)) {
        goto DONE;
    }

    // AT+CNMI=0,0,0,0,0  # Disable SMS notifications
    // OK
    char at_cnmi[] = "\nAT+CNMI=0,0,0,0,0\n";
    if (!gsm_modem_cmd_base(at_cnmi, sizeof(at_cnmi), "OK", 100)) {
        goto DONE;
    }

    // AT+CLIP=0    # Disable incoming call indications
    // OK
    char at_clip[] = "\nAT+CLIP=0\n";
    if (!gsm_modem_cmd_base(at_clip, sizeof(at_clip), "OK", 100)) {
        goto DONE;
    }

    // AT+COLP=0    # Disable connected line identification
    // OK
    char at_colp[] = "\rAT+COLP=0\n";
    if (!gsm_modem_cmd_base(at_colp, sizeof(at_colp), "OK", 100)) {
        goto DONE;
    }

    // AT+CREG=0    # Disable unsolicited network registration information
    // OK
    char at_creg[] = "\nAT+CREG=0\n";
    if (!gsm_modem_cmd_base(at_creg, sizeof(at_creg), "OK", 100)) {
        goto DONE;
    }

    // AT+CGREG=0   # Disable unsolicited GPRS network status
    // OK
    char at_cgreg[] = "\nAT+CGREG=0\n";
    if (!gsm_modem_cmd_base(at_cgreg, sizeof(at_cgreg), "OK", 100)) {
        goto DONE;
    }

    // # AT+CLTS=1  Enable the network time synchronization
    // "OK"
    char at_clts[] = "\nAT+CLTS=1\n";
    if (!gsm_modem_cmd_base(at_cgreg, sizeof(at_cgreg), "OK", 100)) {
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

    // # AT+CGATT=1            Attach to GPRS service
    // OK
    char at_cgatt[] = "\nAT+CGATT=1\n";
    if (!gsm_modem_cmd_base(at_cgatt, sizeof(at_cgatt), "OK", 100)) {
        goto DONE;
    }

    // # AT+CIPSHUT            Reset IP session
    // SHUT OK
    char at_cipshut[] = "\nAT+CIPSHUT\n";
    if (!gsm_modem_cmd_base(at_cipshut, sizeof(at_cipshut), "SHUT OK", 100)) {
        goto DONE;
    }

    // # AT+CIPSTATUS          Check IP status
    // STATE: IP INITIAL
    char at_cipstatus[] = "\nAT+CIPSTATUS\n";
    if (!gsm_modem_cmd_base(at_cipstatus, sizeof(at_cipstatus),
                            "STATE: IP INITIAL", 100)) {
        goto DONE;
    }

    // # AT+CIPMUX=0           Set single connection mode
    // OK
    char at_cipmux[] = "\nAT+CIPMUX=0\n";
    if (!gsm_modem_cmd_base(at_cipmux, sizeof(at_cipmux), "OK", 100)) {
        goto DONE;
    }

    // # AT+CSTT="TM","",""    Set APN's
    // OK
    char at_cstt[] = "\nAT+CSTT=\"TM\",\"\",\"\"\n";
    if (!gsm_modem_cmd_base(at_cstt, sizeof(at_cstt), "OK", 100)) {
        goto DONE;
    }

    // # AT+CIICR              Bring up wireless connection
    // OK
    char at_ciicr[] = "\nAT+CIICR\n";
    if (!gsm_modem_cmd_base(at_ciicr, sizeof(at_ciicr), "OK", 4000)) {
        goto DONE;
    }

    // # AT+CREG?              Check gprs conection expected response:
    // home: "+CREG: 0,1", roaming: "+CREG: 0,5"
    char at_creg[] = "\nAT+CREG?\n";
    if (!gsm_modem_cmd_base(at_creg, sizeof(at_creg), "+CREG: 0,", 2000)) {
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
    bool res = false;

    // AT+CIPSTART="TCP","194.42.111.14","1883"
    // OK
    // CONNECT OK
    char at_cipstart[64];
    size_t at_cipstart_len;
    at_cipstart_len = sprintf(
        at_cipstart, "\nAT+CIPSTART=\"TCP\",\"%s\",\"%u\"\n", domain, port);
    if (!gsm_modem_cmd_base(at_cipstart, at_cipstart_len, "CONNECT OK", 4000)) {
        goto DONE;
    }

    GsmSendBuffer[0] = 0x10;   // MQTT CONNECT packet type
    GsmSendBuffer[1] = 0x0C;   // Remaing length 10 bytes
    GsmSendBuffer[2] = 0x00;
    GsmSendBuffer[3] = 0x04;
    GsmSendBuffer[4] = 'M';
    GsmSendBuffer[5] = 'Q';
    GsmSendBuffer[6] = 'T';
    GsmSendBuffer[7] = 'T';     // Protocol name
    GsmSendBuffer[8] = 0x04;    // Protocol level (4 for MQTT v3.1.1)
    GsmSendBuffer[9] = 0x02;    // Connect flags (Clean session)
    GsmSendBuffer[10] = 0x00;
    GsmSendBuffer[11] = 0x3C;   // Keep - alive timer(60 seconds)
    GsmSendBuffer[12] = 0x00;
    GsmSendBuffer[13] = 0x00;   // No client id
    GsmSendBuffer[14] = 0x1A;   // End of data to send
    if (!gsm_modem_cipsend(GsmSendBuffer, 15, 4000)) {
        goto DONE;
    }

    uint8_t read_buffer[6];
    if (!uart_gsm_read_bytes(read_buffer, 4, 4000)) {
        goto DONE;
    } else {
        // server answers: 0a 53 45 4e 44 20 4f 4b 0a 20 02 00 00(SEND OK, 20 02 00 00)
        // https: docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc385349251
        // 20 - mqtt control packet
        // 02 - remainging length
        // 00 - session not present, 01 - session present
        // 00 - connect return code, 0 - success
        const uint8_t expected[] = {0x20, 0x02, 0x00, 0x00};
        if (0 != memcmp(read_buffer, expected, 4)) {
            goto DONE;
        }
    }

    res = true;
DONE:
    return (res);
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