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

#include "nvs_data.h"
#include "periphery/gpio_sim800l.h"
#include "periphery/uart_gsm.h"

#define GSM_MODEM_WAKEUP_ATTEMPS (8)

LOG_MODULE_REGISTER(SIM800L, LOG_LEVEL_DBG);

static bool gsm_modem_cmd_base(uint8_t *data, size_t len, const char *expected,
                               int32_t timeout) {
    bool res = false;
    char read_line[64];

    uart_gsm_rx_clear();
    uart_gsm_send(data, len);

    int64_t start_ts = k_uptime_get();
    int64_t rl_timeout = (int64_t)timeout;
    do {
        if (uart_gsm_read_line(read_line, sizeof(read_line), timeout)) {
            LOG_INF("< %s/%s", read_line, expected);
            if (strstr(read_line, expected) != NULL) {
                res = true;
                break;
            } else if (strstr(read_line, "ERROR") != NULL) {
                break;
            } else if (strstr(read_line, "SEND FAIL") != NULL) {
                break;
            } else if (strstr(read_line, "CLOSED") != NULL) {
                break;
            } else {
                // No action, unexpected line
            }
        }
    } while ((start_ts + rl_timeout) > k_uptime_get());

    return (res);
}

static bool gsm_modem_cmd_base_str(uint8_t *str_cmd, const char *expected,
                                   int32_t timeout, uint8_t tries,
                                   uint32_t delay_between) {
    uint8_t try = 0;
    LOG_INF("> %s", str_cmd);

    char cmd_buffer[128];
    size_t str_cmd_len = snprintf(cmd_buffer, 64, "\n%s\n", str_cmd);
    for (try = 0; try < tries; try++) {
        if (gsm_modem_cmd_base(str_cmd, str_cmd_len, expected, timeout)) {
            break;
        } else {
            LOG_WRN("Cmd %s try %u/%u failed", str_cmd, try + 1, tries);
            if (delay_between > 0) {
                k_sleep(K_MSEC(delay_between));
            }
        }
    }

    bool res = (try < tries) ? true : false;
    if (false == res) {
        LOG_ERR("Cmd %s failed", str_cmd);
    } else {
        LOG_INF("Cmd %s success", str_cmd);
    }

    return (res);
}

void gsm_modem_init(void) {
    gpio_sim800l_init();
    uart_gsm_init();

    if (gsm_modem_test()) {
        LOG_INF("Push modem into sleep mode");
        gsm_modem_sleep();
    } else {
        LOG_ERR("Modem doesn't react to AT command");
    }
}

bool gsm_modem_test(void) {
    return gsm_modem_cmd_base_str("AT", "OK", 1000, 16, 100);
}

bool gsm_modem_reset(void) {
    return gsm_modem_cmd_base_str("AT+CFUN=1,1", "OK", 1000, 3, 100);
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

bool gsm_modem_config(void) {
    bool res = false;

    LOG_INF("gsm_modem_config start");

    // # AT&F                  Factory settings
    // "OK"
    if (!gsm_modem_cmd_base_str("AT&F", "OK", 250, 2, 100)) {
        goto DONE;
    }

    // # ATE0                  Echo OFF
    // "OK"
    if (!gsm_modem_cmd_base_str("ATE0", "OK", 250, 2, 100)) {
        goto DONE;
    }

    // AT+CLTS=1  Enable the network time synchronization
    // OK
    if (!gsm_modem_cmd_base_str("AT+CLTS=1", "OK", 250, 2, 100)) {
        goto DONE;
    }

    // AT+CNETLIGHT=0  Disable blinking led
    // OK
    if (!gsm_modem_cmd_base_str("AT+CNETLIGHT=0", "OK", 250, 2, 100)) {
        goto DONE;
    }

    // AT&W  Save configuration
    // OK
    if (!gsm_modem_cmd_base_str("AT&W", "OK", 250, 2, 100)) {
        goto DONE;
    }

    res = true;
DONE:
    LOG_INF("... gsm_modem_config done res %u", res);
    return (res);
}

bool gsm_modem_wakeup(void) {
    char at[] = "\nAT\n";
    uart_gsm_rx_clear();
    uart_gsm_send(at, sizeof(at) - 1);
    return gsm_modem_cmd_base_str("AT+CSCLK=0", "OK", 1000,
                                  GSM_MODEM_WAKEUP_ATTEMPS, 100);
}

bool gsm_modem_sleep(void) {
    return gsm_modem_cmd_base_str("AT+CSCLK=2", "OK", 250, 2, 100);
}

bool gsm_modem_net_setup(struct apn_config *apn) {
    bool res = false;

    LOG_INF("gsm_modem_net_setup start");

    // # AT+CGATT=1            Attach to GPRS service
    // OK
    if (!gsm_modem_cmd_base_str("AT+CGATT=1", "OK", 2000,
                                CONFIG_SIM800L_NET_WAIT_MAX_SEC, 1000)) {
        goto DONE;
    }

    // # AT+CIPSHUT            Reset IP session
    // SHUT OK
    if (!gsm_modem_cmd_base_str("AT+CIPSHUT", "SHUT OK", 2000, 2, 1000)) {
        goto DONE;
    }

    // # AT+CIPSTATUS          Check IP status
    // STATE: IP INITIAL
    if (!gsm_modem_cmd_base_str("AT+CIPSTATUS", "STATE: IP INITIAL", 2000, 3,
                                1000)) {
        goto DONE;
    }

    // # AT+CIPMUX=0           Set single connection mode
    // OK
    if (!gsm_modem_cmd_base_str("AT+CIPMUX=0", "OK", 2000, 2, 1000)) {
        goto DONE;
    }

    // # AT+CSTT="TM","",""    Set APN's
    // AT+CSTT="your_apn","your_user","your_password"
    // OK
    char at_cstt[32 + 3 * NVS_ID_APN_MAX_LEN];
    int at_cstt_len = sprintf(at_cstt, "AT+CSTT=\"%s\",\"%s\",\"%s\"", apn->apn,
                              apn->user, apn->password);
    if (!gsm_modem_cmd_base(at_cstt, at_cstt_len, "OK", 1000)) {
        goto DONE;
    }

    // # AT+CIICR              Bring up wireless connection
    // OK
    if (!gsm_modem_cmd_base_str("AT+CIICR", "OK", 4000, 3, 1000)) {
        goto DONE;
    }

    // # AT+CREG?              Check gprs conection expected response:
    // home: "+CREG: 0,1", roaming: "+CREG: 0,5"
    if (!gsm_modem_cmd_base_str("AT+CREG?", "+CREG: 0,", 2000, 4, 1000)) {
        goto DONE;
    }

    // # AT+CIFSR              Get IP address (expecting a non-empty response)
    // OK
    // 10.0.0.0
    char at_cifsr[] = "\nAT+CIFSR\n";
    char read_line[32];
    uart_gsm_rx_clear();
    uart_gsm_send(at_cifsr, sizeof(at_cifsr) - 1);
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
    uart_gsm_send(at_clk, sizeof(at_clk) - 1);
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
    gsm_modem_cmd_base(at_cipclose, sizeof(at_cipclose) - 1, "OK", 1000);
}

/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/