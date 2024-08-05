/* ---------------------------------------------------------------------------
 *  wlab_station
 * ---------------------------------------------------------------------------
 *  Name: shell_commands.c
 * --------------------------------------------------------------------------*/
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/sys/util.h>

#include "nvs_data.h"
#include "uart_gsm.h"

// $ mqttconf <hostname/ip> <port> <ping_period_secs> <max_ping_no_ans_mins>
static int cmd_mqtt_config(const struct shell *shell, size_t argc,
                           char *argv[]) {
    // if (argc != 5) {
    //     shell_fprintf(shell, SHELL_NORMAL, "\tBad command usage!");
    //     return (0);
    // }

    // struct mqtt_config mqttcfg = {0};
    // strncpy(mqttcfg.mqtt_broker, argv[1], CONFIG_BUFF_MAX_STRING_LEN);
    // mqttcfg.mqtt_port = strtoul(argv[2], NULL, 10);
    // mqttcfg.mqtt_ping_period = strtoul(argv[3], NULL, 10);
    // mqttcfg.mqtt_max_ping_no_answer = strtoul(argv[4], NULL, 10);
    // if (0 == nvs_data_mqtt_config_set(&mqttcfg)) {
    //     shell_fprintf(shell, SHELL_NORMAL, "\tmqtt_broker: <%s>\n",
    //                   mqttcfg.mqtt_broker);
    //     shell_fprintf(shell, SHELL_NORMAL, "\tmqtt_port: %u\n",
    //                   mqttcfg.mqtt_port);
    //     shell_fprintf(shell, SHELL_NORMAL, "\tmqtt_ping_period: %u [secs]\n",
    //                   mqttcfg.mqtt_ping_period);
    //     shell_fprintf(shell, SHELL_NORMAL,
    //                   "\tmqtt_max_ping_no_answer: %u [mins]\n",
    //                   mqttcfg.mqtt_max_ping_no_answer);
    //     shell_fprintf(shell, SHELL_NORMAL, "\tOK!\n");
    // } else {
    //     shell_fprintf(shell, SHELL_NORMAL, "\tFailed!\n");
    // }
    return (0);
}

// (set custom wlab id)             $ wlabid <str_hex_id>
// (restore taking id as mac addr)  $ wlabid
static int cmd_wlab_id(const struct shell *shell, size_t argc, char *argv[]) {
    uint64_t device_id = 0;

    if (argc != 2) {
        shell_fprintf(shell, SHELL_NORMAL, "\tBad command usage!");
        return (0);
    }

    if (2 == argc) {
        device_id = strtoull(argv[1], NULL, 16);
    }

    if (0 == nvs_data_wlab_device_id_set(&device_id)) {
        shell_fprintf(shell, SHELL_NORMAL, "device_id: %" PRIX64 "\n",
                      device_id);
        shell_fprintf(shell, SHELL_NORMAL, "\tOK!\n");
    } else {
        shell_fprintf(shell, SHELL_NORMAL, "\tFailed!\n");
    }
    return (0);
}

// $ wlabpubp <publish_period_mins>
static int cmd_wlab_publish_period(const struct shell *shell, size_t argc,
                                   char *argv[]) {
    if (argc != 2) {
        shell_fprintf(shell, SHELL_NORMAL, "\tBad command usage!");
        return (0);
    }

    int64_t pub_period = INT64_C(60) * (int64_t)strtoll(argv[1], NULL, 10);
    if (0 == nvs_data_wlab_pub_period_set(&pub_period)) {
        shell_fprintf(shell, SHELL_NORMAL, "pub_period: %" PRIi64 " [min]\n",
                      pub_period);
        shell_fprintf(shell, SHELL_NORMAL, "\tOK!\n");
    } else {
        shell_fprintf(shell, SHELL_NORMAL, "\tFailed!\n");
    }
    return (0);
}

// $ pconfig
static int cmd_pconfig(const struct shell *shell, size_t argc, char *argv[]) {
    struct mqtt_config mqttcfg = {};
    uint64_t device_id = 0;
    uint64_t pub_period = 0;

    nvs_data_mqtt_config_get(&mqttcfg);
    nvs_data_wlab_device_id_get(&device_id);
    nvs_data_wlab_pub_period_get(&pub_period);

    shell_fprintf(shell, SHELL_NORMAL, "mqtt_broker: <%s>\n", mqttcfg.broker);
    shell_fprintf(shell, SHELL_NORMAL, "mqtt_port: %u\n", mqttcfg.port);
    shell_fprintf(shell, SHELL_NORMAL, "device_id: %" PRIX64 "\n", device_id);
    shell_fprintf(shell, SHELL_NORMAL, "publish_period: %" PRIi64 " [mins]\n",
                  pub_period);
    return (0);
}

// reboot
static int cmd_reboot(const struct shell *shell, size_t argc, char *argv[]) {
    // Reboot the system
    sys_reboot(SYS_REBOOT_COLD);
    return (0);
}

// at
static int cmd_at(const struct shell *shell, size_t argc, char *argv[]) {
    if (argc > 1) {
        uart_gsm_send("\n", 1);
        for (int idx = 1; idx < argc; idx++) {
            size_t len = strlen(argv[idx]);
            uart_gsm_send(argv[idx], len);
        }
        uart_gsm_send("\n", 1);
    } else {
        shell_fprintf(shell, SHELL_NORMAL, "\tBad command usage!");
    }

    return (0);
}

// test
static int cmd_test(const struct shell *shell, size_t argc, char *argv[]) {
    shell_fprintf(shell, SHELL_NORMAL, "Test command enter...");

    shell_fprintf(shell, SHELL_NORMAL, "... done");
    return (0);
}

SHELL_CMD_REGISTER(test, NULL,
                   "Test command                \n"
                   "Usage:                      \n"
                   "$ test                        ",
                   cmd_test);

SHELL_CMD_REGISTER(at, NULL,
                   "Send command to modem       \n"
                   "Usage:                      \n"
                   "$ at AT                       ",
                   cmd_at);

SHELL_CMD_REGISTER(reboot, NULL,
                   "Restart uC                  \n"
                   "Usage:                      \n"
                   "$ reboot                     ",
                   cmd_reboot);

SHELL_CMD_REGISTER(pconfig, NULL,
                   "Print all custom user config\n"
                   "Usage:                      \n"
                   "$ pconfig                     ",
                   cmd_pconfig);

SHELL_CMD_REGISTER(mqttconf, NULL,
                   "Configure mqtt worker settings\n"
                   "Usage:\n"
                   "$ mqttconf <hostname/ip> <port> <ping_period_secs> "
                   "<max_ping_no_ans_mins>\n"
                   "$ mqttconf 192.168.1.10 1883 60 10",
                   cmd_mqtt_config);

SHELL_CMD_REGISTER(wlabid, NULL,
                   "Set custom wlab device id\n"
                   "Usage:\n"
                   "(set custom wlab id)             $ wlabid <str_hex_id>\n"
                   "(set custom wlab id)             $ wlabid 1122EEFF22AA\n"
                   "(restore taking id as mac addr)  $ wlabid",
                   cmd_wlab_id);

SHELL_CMD_REGISTER(wlabpubp, NULL,
                   "Set wlab publish period in seconds\n"
                   "Usage:\n"
                   "$ wlabpubp <publish_period_mins>\n"
                   "$ wlabpubp 10                     ",
                   cmd_wlab_publish_period);

/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/