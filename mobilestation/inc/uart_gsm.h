/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: uart_gsm.h
 * --------------------------------------------------------------------------*/
#include <stdint.h>

/**
 * @brief uartport_init
 * 
 */
void uart_gsm_init(void);

/**
 * @brief uartport_send_receive
 * 
 * @param tx_data 
 * @param tx_len 
 * @param rx_data 
 * @param rx_patterns 
 * @param timeout 
 */
void uart_gsm_send_receive(uint8_t *tx_data, size_t tx_len, char *rx_data,
                           const char *rx_patterns[], int64_t timeout);

/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/