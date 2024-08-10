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

#include "periphery/gpio_sim800l.h"
#include "periphery/uart_gsm.h"

LOG_MODULE_REGISTER(SIM800L, LOG_LEVEL_DBG);

static bool gsm_modem_cmd_base(uint8_t *data, size_t len, const char *expected,
                               int32_t timeout);

void gsm_modem_init(void) {
    gpio_sim800l_init();
    uart_gsm_init();
}

bool gsm_modem_test(void) {
    int try = 0;
    bool res = false;

    LOG_INF("gsm_modem_test start");
    for (try = 0; try < 24; try++) {
        char at[] = "\nAT\n";
        if (gsm_modem_cmd_base(at, sizeof(at), "OK", 1000)) {
            res = true;
            break;
        }
    }

    LOG_INF("... gsm_modem_test done res %u", res);
    return (res);
}

bool gsm_modem_reset(void) {
    bool res = false;
    LOG_INF("gsm_modem_reset start");
    // AT+CFUN=1,1  Restart module
    // OK
    char at_cfun11[] = "\nAT+CFUN=1,1\n";
    if (!gsm_modem_cmd_base(at_cfun11, sizeof(at_cfun11), "OK", 1000)) {
        goto DONE;
    }

    res = true;
DONE:
    LOG_INF("... gsm_modem_reset done res %u", res);
    return (res);
}

bool gsm_modem_cipsend(uint8_t *data, size_t len, int32_t timeout) {
    // AT+CIPSEND
    // 0a 3e 20 - "\n> "
    // data has to be ended with 0x1A
    bool res = false;

    uart_gsm_rx_clear();

    char at_cipsend[] = "\nAT+CIPSEND\n";
    LOG_INF("> %s", at_cipsend + 1);
    uart_gsm_send(at_cipsend, sizeof(at_cipsend) - 1);

    int64_t start_ts = k_uptime_get();
    uint8_t chr = 0;
    do {
        if (uart_gsm_getc(&chr, 10)) {
            if ('>' == chr) {
                break;
            }
        }
    } while ((start_ts + INT64_C(8 * 1000)) > k_uptime_get());

    uart_gsm_rx_clear();

    if (chr != '>') {
        LOG_ERR("Read prompt failed");
        goto DONE;
    }

    if (!gsm_modem_cmd_base(data, len, "SEND OK", timeout)) {
        LOG_ERR("No SEND OK answer");
        goto DONE;
    }

    res = true;
DONE:
    return (res);
}

static bool gsm_modem_cmd_base(uint8_t *data, size_t len, const char *expected,
                               int32_t timeout) {
    bool res = false;
    char read_line[64];

    uart_gsm_rx_clear();
    uart_gsm_send(data, len);
    // LOG_INF("> %s", data + 1);

    int64_t start_ts = k_uptime_get();
    int64_t rl_timeout = (int64_t)timeout;
    do {
        if (uart_gsm_read_line(read_line, sizeof(read_line), timeout)) {
            LOG_INF("< %s/%s", read_line, expected);
            if (NULL != strstr(read_line, expected)) {
                res = true;
                break;
            } else if (NULL != strstr(read_line, "ERROR")) {
                break;
            } else if (NULL != strstr(read_line, "SEND FAIL")) {
                break;
            } else if (NULL != strstr(read_line, "CLOSED")) {
                break;
            } else {
                // TODO Test!
                // int64_t elapsed = k_uptime_get() - start_ts;
                // if (0 < rl_timeout - elapsed) {
                //     rl_timeout -= elapsed;
                // }
            }
        }
    } while ((start_ts + rl_timeout) > k_uptime_get());

    return (res);
}

bool gsm_modem_config(void) {
    bool res = false;

    LOG_INF("gsm_modem_config start");

    // # AT                  Echo OFF
    // "OK"
    char at[] = "\nAT\n";
    if (!gsm_modem_cmd_base(at, sizeof(at), "OK", 1000)) {
        goto DONE;
    }

    // // # AT&F                  Factory settings
    // // "OK"
    // char at_f[] = "\rAT&F\r";
    // if (!gsm_modem_cmd_base(at_f, sizeof(at_f), "OK", 1000)) {
    //     goto DONE;
    // }

    // # ATE0                  Echo OFF
    // "OK"
    char at_e0[] = "\nATE0\n";
    if (!gsm_modem_cmd_base(at_e0, sizeof(at_e0), "OK", 100)) {
        goto DONE;
    }

    // AT+CIURC=0   # Disable "Call Ready" message
    // OK
    // char at_ciurc[] = "\nAT+CIURC=0\n";
    // if (!gsm_modem_cmd_base(at_ciurc, sizeof(at_ciurc), "OK", 100)) {
    //     goto DONE;
    // }

    // AT+CNMI=0,0,0,0,0  # Disable SMS notifications
    // OK
    // char at_cnmi[] = "\nAT+CNMI=0,0,0,0,0\n";
    // if (!gsm_modem_cmd_base(at_cnmi, sizeof(at_cnmi), "OK", 100)) {
    //     goto DONE;
    // }

    // AT+CLIP=0    # Disable incoming call indications
    // OK
    // char at_clip[] = "\nAT+CLIP=0\n";
    // if (!gsm_modem_cmd_base(at_clip, sizeof(at_clip), "OK", 100)) {
    //     goto DONE;
    // }

    // AT+COLP=0    # Disable connected line identification
    // OK
    // char at_colp[] = "\rAT+COLP=0\n";
    // if (!gsm_modem_cmd_base(at_colp, sizeof(at_colp), "OK", 100)) {
    //     goto DONE;
    // }

    // AT+CREG=0    # Disable unsolicited network registration information
    // OK
    // char at_creg[] = "\nAT+CREG=0\n";
    // if (!gsm_modem_cmd_base(at_creg, sizeof(at_creg), "OK", 100)) {
    //     goto DONE;
    // }

    // // AT+CGREG=0   # Disable unsolicited GPRS network status
    // // OK
    // char at_cgreg[] = "\nAT+CGREG=0\n";
    // if (!gsm_modem_cmd_base(at_cgreg, sizeof(at_cgreg), "OK", 100)) {
    //     goto DONE;
    // }

    // AT+CLTS=1  Enable the network time synchronization
    // OK
    char at_clts[] = "\nAT+CLTS=1\n";
    if (!gsm_modem_cmd_base(at_clts, sizeof(at_clts), "OK", 1000)) {
        goto DONE;
    }

    // AT+CNETLIGHT=0  Disable blinking led
    // OK
    char at_cnetlight[] = "\nAT+CNETLIGHT=0\n";
    if (!gsm_modem_cmd_base(at_cnetlight, sizeof(at_cnetlight), "OK", 1000)) {
        goto DONE;
    }

    // AT&W  Save configuration
    // OK
    char at_w[] = "\nAT&W\n";
    if (!gsm_modem_cmd_base(at_w, sizeof(at_w), "OK", 1000)) {
        goto DONE;
    }

    res = true;
DONE:
    LOG_INF("... gsm_modem_config done res %u", res);
    return (res);
}

bool gsm_modem_wakeup(void) {
    bool res = false;

    // AT+CFUN=0    Minimum functionality. Lowest power consumption, RF disabled, 0.796mA.
    // AT+CFUN=1    Full functionality (default).
    // AT+CFUN=4    Flight mode (disable RF function).
    // OK
    char at_wakeup[] = "\nAT+CFUN=1\n";
    if (!gsm_modem_cmd_base(at_wakeup, sizeof(at_wakeup), "OK", 8000)) {
        goto DONE;
    }

    res = true;
DONE:
    return (res);
}

bool gsm_modem_sleep(void) {
    bool res = false;

    // AT+CFUN=0    Minimum functionality. Lowest power consumption, RF disabled, 0.796mA.
    // AT+CFUN=1    Full functionality (default).
    // AT+CFUN=4    Flight mode (disable RF function).
    // OK
    char at_sleep[] = "\nAT+CFUN=0\n";
    if (!gsm_modem_cmd_base(at_sleep, sizeof(at_sleep), "OK", 1000)) {
        goto DONE;
    }

    res = true;
DONE:
    return (res);
}

bool gsm_modem_net_setup(void) {
    bool res = false;

    LOG_INF("gsm_modem_net_setup start");
    int try = 0;

    for (try = 0; try < CONFIG_SIM800L_NET_WAIT_MAX_SEC; try++) {
        // # AT+CGATT=1            Attach to GPRS service
        // OK
        char at_cgatt[] = "\nAT+CGATT=1\n";
        if (!gsm_modem_cmd_base(at_cgatt, sizeof(at_cgatt), "OK", 1000)) {
            k_sleep(K_MSEC(1000));
        } else {
            break;   // success
        }
    }

    if (CONFIG_SIM800L_NET_WAIT_MAX_SEC == try) {
        goto DONE;
    }

    // # AT+CIPSHUT            Reset IP session
    // SHUT OK
    char at_cipshut[] = "\nAT+CIPSHUT\n";
    if (!gsm_modem_cmd_base(at_cipshut, sizeof(at_cipshut), "SHUT OK", 2000)) {
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
    // OK
    // 10.0.0.0
    char at_cifsr[] = "\nAT+CIFSR\n";
    char read_line[32];
    uart_gsm_rx_clear();
    uart_gsm_send(at_cifsr, sizeof(at_cifsr));
    int64_t start_ts = k_uptime_get();
    LOG_INF("> %s", at_cifsr + 1);
    do {
        if (uart_gsm_read_line(read_line, sizeof(read_line), 100)) {
            LOG_INF("< %s", read_line);
            uint32_t addr3, addr2, addr1, addr0;
            int scan_res = sscanf(read_line, "%u.%u.%u.%u", &addr3, &addr2,
                                  &addr1, &addr0);
            if (4 != scan_res) {   // check wheather whole ip scanned properly
                continue;
            } else {
                LOG_INF("Ip address: %u.%u.%u.%u", addr3, addr2, addr1, addr0);
                break;
            }
        }

        if ((start_ts + 4000) > k_uptime_get()) {
            goto DONE;
        }
    } while (true);

    res = true;
DONE:
    LOG_INF("... gsm_modem_net_setup done res %u", res);
    return (res);
}

bool gsm_modem_mqtt_connect(const char *domain, uint32_t port) {
    bool res = false;

    // AT+CIPSTART="TCP","194.42.111.14","1883"
    // OK
    // CONNECT OK
    char send_data[64];
    size_t send_data_len;
    send_data_len = sprintf(send_data, "\nAT+CIPSTART=\"TCP\",\"%s\",\"%u\"\n",
                            domain, port);
    LOG_INF("> %s", send_data + 1);
    if (!gsm_modem_cmd_base(send_data, send_data_len, "CONNECT OK", 6000)) {
        LOG_ERR("Cipstart failed");
        goto DONE;
    }

    send_data[0] = 0x10;   // MQTT CONNECT packet type
    send_data[1] = 0x0C;   // Remaing length 10 bytes
    send_data[2] = 0x00;
    send_data[3] = 0x04;
    send_data[4] = 'M';
    send_data[5] = 'Q';
    send_data[6] = 'T';
    send_data[7] = 'T';    // Protocol name
    send_data[8] = 0x04;   // Protocol level (4 for MQTT v3.1.1)
    send_data[9] = 0x02;   // Connect flags (Clean session)
    send_data[10] = 0x00;
    send_data[11] = 0x3C;   // Keep - alive timer(60 seconds)
    send_data[12] = 0x00;
    send_data[13] = 0x00;   // No client id
    send_data[14] = 0x1A;   // End of data to send
    if (!gsm_modem_cipsend(send_data, 15, 4000)) {
        LOG_ERR("Cipsend failed");
        goto DONE;
    }

    uint8_t read_buffer[6];
    if (!uart_gsm_read_bytes(read_buffer, 5, 4000)) {
        LOG_ERR("Read broker answer failed");
        goto DONE;
    } else {
        // server answers: 0a 53 45 4e 44 20 4f 4b 0a 20 02 00 00(SEND OK, 20 02 00 00)
        // https: docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc385349251
        // x20 - mqtt control packet
        // x02 - remainging length
        // x00 - session not present, 01 - session present
        // x00 - connect return code, 0 - success
        const uint8_t expected[] = {'\n', 0x20, 0x02, 0x00, 0x00};
        if (0 != memcmp(read_buffer, expected, 5)) {
            LOG_ERR("Broker answer bad");
            goto DONE;
        }
    }

    LOG_INF("MQTT broker connection established");
    res = true;
DONE:
    return (res);
}

bool gsm_modem_mqtt_publish(const char *topic, uint8_t *data, size_t len) {
    // AT+CIPSEND Publish message: 123, topic /wlabdb/bix
    // 30 10 00 0B 2F 77 6C 61 62 64 62 2F 62 69 78 31 32 33 1A
    // uint8_t pubbix[] = {0x30, 0x10, 0x00, 0x0B, 0x2F, 0x77, 0x6C,
    //                     0x61, 0x62, 0x64, 0x62, 0x2F, 0x62, 0x69,
    //                     0x78, 0x31, 0x32, 0x33, 0x1A};
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

    if (!gsm_modem_cipsend(publish_buffer, total_len, 12000)) {
        goto DONE;
    }

    // There is no answer from server, maybe higher QoS needed
    // uint8_t read_buffer[6];
    // if (!uart_gsm_read_bytes(read_buffer, 4, 4000)) {
    //     goto DONE;
    // } else {
    //     // 40 - mqtt control packet
    //     // 02 - remainging length
    //     // XX - package identifier being acknowledged msb
    //     // XX - package identifier being acknowledged lsb
    //     //  - connect return code, 0 - success
    //     if ((0x40 != read_buffer[0]) || (0x02 != read_buffer[1])) {
    //         goto DONE;
    //     }
    // }

    res = true;
DONE:
    return (res);
}

bool gsm_modem_get_ts_utc(int64_t *ts) {
    bool res = false;
    char read_line[64];
    struct tm time_struct;

    // AT+CCLK?
    // +CCLK : "21/07/17,12:34:56+00"
    // +CCLK: "04/01/01,00:05:55+08" in case of failed
    // +08 multiply of 15 minutes
    char at_clk[] = "\nAT+CCLK?\n";
    uart_gsm_rx_clear();
    uart_gsm_send(at_clk, sizeof(at_clk));
    LOG_INF("> %s", at_clk + 1);
    if (uart_gsm_read_line(read_line, sizeof(read_line), 1000)) {
        LOG_INF("< %s", read_line);
        if (NULL == strstr(read_line, "+CCLK")) {
            LOG_ERR("Failed to get time");
            goto DONE;
        }

        if (NULL != strstr(read_line, "04/01/01")) {
            LOG_ERR("Time not synced properly");
            goto DONE;
        }

        int32_t year, month, day, hour, minute, second, utc_offset;
        char utc_offset_sign;
        if (8 == sscanf(read_line, "+CCLK: \"%d/%d/%d,%d:%d:%d%c%d\"", &year,
                        &month, &day, &hour, &minute, &second, &utc_offset_sign,
                        &utc_offset)) {
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
                if ('+' == utc_offset_sign) {
                    epoch_time -= (int64_t)(15 * 60 * utc_offset);
                } else {
                    epoch_time += (int64_t)(15 * 60 * utc_offset);
                }
                LOG_DBG("Epoch time: %lld", epoch_time);
                *ts = epoch_time;
            } else {
                LOG_ERR("Failed to convert to epoch time");
                goto DONE;
            }
        } else {
            LOG_ERR("Failed to scan timestamp");
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