/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: main.c
 * --------------------------------------------------------------------------*/
#include <stdint.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/kernel_version.h>
#include <zephyr/logging/log.h>

#include "nvs_data.h"
#include "version.h"
#include "wlab.h"

LOG_MODULE_REGISTER(MAIN, LOG_LEVEL_DBG);

// 1
// status led

// 2
// reset button

// 3
// rebooting when net_setup failed and try again

// 4
// document sim800l.h

// 5
// one sample timestamp, cnt for hum and dtemp

// 6
// disable sim800l onboard led

// 7
// [01:22:07.458,404] <inf> SIM800L: gsm_modem_net_setup start
// [01:22:07.489,196] <inf> SIM800L: < ERROR/OK
// [01:22:08.523,498] <inf> SIM800L: < ERROR/OK
// [01:22:09.554,504] <inf> SIM800L: < ERROR/OK
// [01:22:10.585,510] <inf> SIM800L: < ERROR/OK
// [01:22:11.616,516] <inf> SIM800L: < ERROR/OK
// [01:22:12.647,521] <inf> SIM800L: < ERROR/OK
// [01:22:13.678,527] <inf> SIM800L: < ERROR/OK
// [01:22:14.719,421] <inf> SIM800L: < ERROR/OK
// [01:22:15.749,206] <inf> SIM800L: < ERROR/OK
// [01:22:16.774,261] <inf> SIM800L: < OK/OK
// [01:22:16.807,189] <inf> SIM800L: < SHUT OK/SHUT OK
// [01:22:16.837,280] <inf> SIM800L: < OK/STATE: IP INITIAL
// [01:22:16.857,086] <inf> SIM800L: < STATE: IP INITIAL/STATE: IP INITIAL
// [01:22:16.936,187] <inf> SIM800L: < *PSUTTZ: 2024,8,5,19,40,22,"+8",1/OK
// [01:22:16.945,037] <inf> SIM800L: < DST: 1/OK
// [01:22:16.980,194] <inf> SIM800L: < +CIEV: 10,"26003","Orange","Orange", 0, 0/OK
// [01:22:16.980,194] <inf> SIM800L: ... gsm_modem_net_setup done res 0
// [01:22:16.980,194] <err> WLAB: Network up failed

// 8
// why issue with timestamp, two times taken

// 9
// Create python script to authorize new device

// 10
// Dump rpi image

// 11
// create uart logger for RPI

// 12
// MQTT Dashboard api create

// 13
// [00:08:21.657,012] <inf> SIM800L: gsm_modem_config start
// [00:08:21.674,285] <inf> SIM800L: < OK/OK
// [00:08:21.694,122] <inf> SIM800L: < OK/OK
// [00:08:21.720,367] <inf> SIM800L: < OK/OK
// [00:08:21.753,021] <inf> SIM800L: < OK/OK
// [00:08:21.776,092] <inf> SIM800L: < OK/OK
// [00:08:21.776,123] <inf> SIM800L: ... gsm_modem_config done res 1
// [00:08:21.776,153] <inf> SIM800L: gsm_modem_net_setup start
// [00:08:21.803,802] <inf> SIM800L: < OK/OK
// [00:08:23.906,738] <inf> SIM800L: ... gsm_modem_net_setup done res 0
// [00:08:23.906,768] <err> WLAB: Network up failed

int main(void) {
    uint32_t ver = sys_kernel_version_get();

    LOG_INF("Board: %s", CONFIG_BOARD);
    LOG_INF("Firmware: %s Zephyr: %u.%u.%u", FIRMWARE_VERSION,
            SYS_KERNEL_VER_MAJOR(ver), SYS_KERNEL_VER_MINOR(ver),
            SYS_KERNEL_VER_PATCHLEVEL(ver));

    nvs_data_init();

    wlab_init();

    for (;;) {
        wlab_proc();
    }
    return 0;
}

/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/