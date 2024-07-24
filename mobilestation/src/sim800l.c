/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: sim800l.c
 * --------------------------------------------------------------------------*/
#include "sim800l.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "uart_gsm.h"

LOG_MODULE_REGISTER(SIM800L, LOG_LEVEL_DBG);

static bool gsm_modem_cmd_base(uint8_t *data, size_t len, const char *expected,
                               int32_t timeout);

const struct gpio_dt_spec GsmResetPin =
    GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), gsmreset_gpios);

void gsm_modem_init(void) {
    if (!device_is_ready(GsmResetPin.port)) {
        LOG_ERR("GsmResetPin not ready");
    }

    gpio_pin_configure_dt(&GsmResetPin, GPIO_OUTPUT_HIGH);
}

bool gsm_modem_reset(void) {
    bool res = false;

    // # AT&F                  Factory settings
    // "OK"
    char at_f[] = "\rAT&F\r";
    if (!gsm_modem_cmd_base(at_f, sizeof(at_f), "SEND_OK", 1000)) {
        goto DONE;
    }

    res = true;
DONE:
    return (res);
}

bool gsm_modem_cipsend(uint8_t *data, size_t len, int32_t timeout) {
    // AT+CIPSEND
    // 0a 3e 20 - "\n> "
    // data has to be ended with 0x1A
    bool res = false;

    uart_gsm_rx_clear();

    uint8_t read_buffer[4];
    char *at_cipsend = "\nAT+CIPSEND\n";
    uart_gsm_send(at_cipsend, sizeof(at_cipsend) - 1);
    if (!uart_gsm_read_bytes(read_buffer, 3, 1000)) {
        goto DONE;
    } else {
        uint8_t expected[] = {'\n', '>', ' '};
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

static bool gsm_modem_cmd_base(uint8_t *data, size_t len, const char *expected,
                               int32_t timeout) {
    bool res = false;
    int64_t start_ts = k_uptime_get();
    char read_line[64];

    uart_gsm_rx_clear();
    uart_gsm_send(data, len);
    int64_t rl_timeout = (int64_t)timeout;
    do {
        if (uart_gsm_read_line(read_line, sizeof(read_line), timeout)) {
            if (0 == strstr(read_line, expected)) {
                res = true;
                break;
            } else if (0 == strstr(read_line, "ERROR")) {
                break;
            } else if (0 == strstr(read_line, "SEND FAIL")) {
                break;
            } else if (0 == strstr(read_line, "CLOSED")) {
                break;
            } else {
                // TODO Test!
                // int64_t elapsed = k_uptime_get() - start_ts;
                // if (0 < rl_timeout - elapsed) {
                //     rl_timeout -= elapsed;
                // }
            }
        }
    } while (start_ts + rl_timeout > k_uptime_get());

    return (res);
}

bool gsm_modem_config(void) {
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
    if (!gsm_modem_cmd_base(at_clts, sizeof(at_clts), "OK", 100)) {
        goto DONE;
    }

    res = true;
DONE:
    return (res);
}

bool gsm_modem_wakeup(void) {
    bool result = false;
    // To wake up the module from sleep mode, pull the DTR pin low.
    // This action will wake up the module and bring it back to active mode.

    // AT            To confirm wheather module woken up
    // OK
    char at[] = "\nAT\n";
    if (!gsm_modem_cmd_base(at, sizeof(at), "OK", 2000)) {
        goto DONE;
    }
    return (result);
}

bool gsm_modem_sleep(void) {
    bool result = false;

    // AT+CSCLK=1            Enter into sleep mode
    // OK
    char at_csclk[] = "\nAT+CSCLK=1\n";
    if (!gsm_modem_cmd_base(at_csclk, sizeof(at_csclk), "OK", 1000)) {
        goto DONE;
    }

    // The Data Terminal Ready (DTR) pin must be pulled high for the module to enter sleep mode.
    // Configure the DTR pin to high after issuing the AT+CSCLK=1 command.

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
    char at_cifsr[] = "\nAT+CIFSR\n";
    char read_line[20];
    uart_gsm_rx_clear();
    uart_gsm_send(at_cifsr, sizeof(at_cifsr));
    if (uart_gsm_read_line(read_line, sizeof(read_line), 100)) {
        uint32_t addr3, addr2, addr1, addr0;
        int scan_res =
            sscanf(read_line, "%u.%u.%u.%u", &addr3, &addr2, &addr1, &addr0);
        if (4 != scan_res) {
            goto DONE;
        } else {
            LOG_INF("Ip address: %u.%u.%u.%u", addr3, addr2, addr1, addr0);
        }
    } else {
        goto DONE;
    }

    res = true;
DONE:
    return (res);
}

bool gsm_modem_mqtt_connect(const char *domain, uint32_t port,
                            int64_t timeout) {
    bool res = false;

    // AT+CIPSTART="TCP","194.42.111.14","1883"
    // OK
    // CONNECT OK
    char send_data[64];
    size_t send_data_len;
    send_data_len = sprintf(send_data, "\nAT+CIPSTART=\"TCP\",\"%s\",\"%u\"\n",
                            domain, port);
    if (!gsm_modem_cmd_base(send_data, send_data_len, "CONNECT OK", 4000)) {
        goto DONE;
    }

    send_data[0] = 0x10;   // MQTT CONNECT packet type
    send_data[1] = 0x0C;   // Remaing length 10 bytes
    send_data[2] = 0x00;
    send_data[3] = 0x04;
    send_data[4] = 'M';
    send_data[5] = 'Q';
    send_data[6] = 'T';
    send_data[7] = 'T';     // Protocol name
    send_data[8] = 0x04;    // Protocol level (4 for MQTT v3.1.1)
    send_data[9] = 0x02;    // Connect flags (Clean session)
    send_data[10] = 0x00;
    send_data[11] = 0x3C;   // Keep - alive timer(60 seconds)
    send_data[12] = 0x00;
    send_data[13] = 0x00;   // No client id
    send_data[14] = 0x1A;   // End of data to send
    if (!gsm_modem_cipsend(send_data, 15, 4000)) {
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
    bool res = false;
    uint8_t publish_buffer[128];
    size_t topic_len = strlen(topic);
    // 1B: PacketType 1B: RemainingLen + 2B: TopicLen + Topic + Data
    size_t total_len = 1 + 1 + 2 + strlen(topic) + len;

    publish_buffer[0] = 0x30;                      // MQTT PUBLISH packet type
    publish_buffer[1] = 2 + strlen(topic) + len;   // Remaining length
    publish_buffer[2] = (uint8_t)(topic_len >> 8);
    publish_buffer[3] = (uint8_t)(topic_len & 0x00FF);
    memcpy(publish_buffer + 4, topic, topic_len);
    memcpy(publish_buffer + 4 + topic_len, data, len);
    publish_buffer[total_len++] = 0x1A;   // End

    if (!gsm_modem_cipsend(publish_buffer, total_len, 4000)) {
        goto DONE;
    }

    uint8_t read_buffer[6];
    if (!uart_gsm_read_bytes(read_buffer, 4, 4000)) {
        goto DONE;
    } else {
        // 40 - mqtt control packet
        // 02 - remainging length
        // XX - package identifier being acknowledged msb
        // XX - package identifier being acknowledged lsb
        //  - connect return code, 0 - success
        if ((0x40 != read_buffer[0]) || (0x02 != read_buffer[1])) {
            goto DONE;
        }
    }

    res = true;
DONE:
    return (res);
}

bool gsm_modem_get_timestamp(int64_t *ts) {
    bool res = false;
    char read_line[64];
    struct tm time_struct;

    // AT+CCLK?
    // +CCLK : "21/07/17,12:34:56+00"
    char at_clk[] = "AT+CCLK?";
    uart_gsm_rx_clear();
    uart_gsm_send(at_clk, sizeof(at_clk));
    if (uart_gsm_read_line(read_line, sizeof(read_line), 1000)) {
        if (0 != strstr(read_line, "+CCLK")) {
            LOG_ERR("Failed to get time");
            goto DONE;
        }

        int32_t year, month, day, hour, minute, second;
        if (6 == sscanf(read_line, "+CCLK: \"%d/%d/%d,%d:%d:%d%*d\"", &year,
                        &month, &day, &hour, &minute, &second)) {
            time_struct.tm_year = year + 100;   // Years since 1900
            time_struct.tm_mon = month - 1;     // Months since January
            time_struct.tm_mday = day;
            time_struct.tm_hour = hour;
            time_struct.tm_min = minute;
            time_struct.tm_sec = second;
            // Not considering daylight saving time
            time_struct.tm_isdst = -1;

            time_t epoch_time = mktime(&time_struct);
            if (epoch_time != -1) {
                LOG_DBG("Epoch time: %lld\n", epoch_time);
                *ts = epoch_time;
            } else {
                LOG_ERR("Failed to convert to epoch time\n");
                goto DONE;
            }
        }
    }

    res = true;
DONE:
    return (res);
}

void gsm_modem_mqtt_close(void) {
    // # AT+CIPCLOSE    Close connection socket
    // "OK"
    char at_cipclose[] = "\nAT+CIPCLOSE\n";
    gsm_modem_cmd_base(at_cipclose, sizeof(at_cipclose), "OK", 1000);
}

/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/