/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: sim800l.h
 * --------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SIM800L_NET_WAIT_SEC (2 * 60)

void gsm_modem_init(void);
bool gsm_modem_reset(void);
bool gsm_modem_config(void);
bool gsm_modem_get_timestamp(int64_t *ts);
bool gsm_modem_test(void);
bool gsm_modem_mqtt_connect(const char *domain, uint32_t port);
bool gsm_modem_mqtt_publish(const char *topic, uint8_t *data, size_t len);
void gsm_modem_mqtt_close(void);
bool gsm_modem_wakeup(void);
bool gsm_modem_sleep(void);
bool gsm_modem_net_setup(void);

/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/