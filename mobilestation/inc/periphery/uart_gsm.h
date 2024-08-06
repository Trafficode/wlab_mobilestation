/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: uart_gsm.h
 * --------------------------------------------------------------------------*/
#ifndef UART_GSM_H_
#define UART_GSM_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief uartport_init
 * 
 */
void uart_gsm_init(void);

/**
 * @brief uart_gsm_send
 * 
 * @param tx_data Pointer with data to send
 * @param tx_len Number of bytes to send
 */
void uart_gsm_send(uint8_t *tx_data, size_t tx_len);

/**
 * @brief uart_gsm_read_bytes
 * 
 * @param rx_data Pointer where received data will be saved
 * @param rx_len Number of bytes to read
 * @param timeout Milliseconds to wait for data
 * @return true Got given bytes number
 * @return false Timeout occured
 */
bool uart_gsm_read_bytes(uint8_t *rx_data, size_t exp_len, int32_t timeout);

/**
 * @brief uart_gsm_read_line
 * 
 * @param rx_line Pointer where received data will be saved
 * @param rx_len Length of received line
 * @param timeout Milliseconds to wait for line
 * @return true Got line properly
 * @return false Timeout occured
 */
bool uart_gsm_read_line(char *rx_line, size_t max_len, int32_t timeout);

/**
 * @brief uart_gsm_rx_clear
 * 
 */
void uart_gsm_rx_clear(void);

/**
 * @brief uart_gsm_getc
 * 
 * @param chr Pointer where received byte will be stored
 * @param timeout 
 * @return true Got char properly
 * @return false Timeout occured
 */
bool uart_gsm_getc(uint8_t *chr, int32_t timeout);

#endif /* UART_GSM_H_ */
/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/