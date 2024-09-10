/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: sim800l.h
 * --------------------------------------------------------------------------*/
#ifndef SIM800L_H_
#define SIM800L_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "nvs_data.h"

/**
 * @brief gsm_modem_init
 * Initilise neccessary staff.
 */
void gsm_modem_init(void);

/**
 * @brief gsm_modem_test
 * Test communication with modem. Uart based.
 * @return true 
 * @return false 
 */
bool gsm_modem_test(void);

/**
 * @brief gsm_modem_reset
 * Perform modem reseting.
 * @return true 
 * @return false 
 */
bool gsm_modem_reset(void);

/**
 * @brief gsm_modem_config
 * Perform modem configuration. Time synchronization, led disabling etc.
 * @return true 
 * @return false 
 */
bool gsm_modem_config(void);

/**
 * @brief gsm_modem_get_ts_utc
 * Get timestamp from gsm network. Local time is taken.
 * @param ts Pointer where timestamp will be stored. Epoch time in seconds.
 * @return true 
 * @return false 
 */
bool gsm_modem_get_ts_utc(int64_t *ts);

/**
 * @brief gsm_modem_net_setup
 * Setup gsm connection with broker. Doing this action Ip address is assigned and
 * TCP connection can be established.
 * @param apn Provide pointer to structure apn_config 
 * @return true Success
 * @return false Failed
 */
bool gsm_modem_net_setup(struct apn_config *apn);

/**
 * @brief gsm_modem_mqtt_connect
 * Establish mqtt connection with broker.
 * 
 * @param domain 
 * @param port 
 * @return true Success
 * @return false Failed
 */
bool gsm_modem_mqtt_connect(const char *domain, uint32_t port);

/**
 * @brief gsm_modem_mqtt_publish
 * 
 * @param topic Topic to publish, as string.
 * @param data Data bytes to publish.
 * @param len Length of data.
 * @return true Success
 * @return false Failed
 */
bool gsm_modem_mqtt_publish(const char *topic, uint8_t *data, size_t len,
                            uint8_t retires);

/**
 * @brief gsm_modem_mqtt_close
 * Close mqtt connection, socket.
 */
void gsm_modem_mqtt_close(void);

/**
 * @brief gsm_modem_wakeup
 * Wakeup from sleep mode.
 * @return true Success
 * @return false Failed
 */
bool gsm_modem_wakeup(void);

/**
 * @brief gsm_modem_sleep
 * Enter sleep mode.
 * @return true Success
 * @return false Failed
 */
bool gsm_modem_sleep(void);

#endif /* SIM800L_H_ */
/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/