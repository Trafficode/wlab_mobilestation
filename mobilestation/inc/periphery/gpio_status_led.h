/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: gpio_status_led.h
 * --------------------------------------------------------------------------*/
#ifndef GPIO_STATUS_LED_H_
#define GPIO_STATUS_LED_H_

#include <stdint.h>

/**
 * @brief gpio_status_led_init
 * Initialise as turned off.
 */
void gpio_status_led_init(void);

/**
 * @brief gpio_status_led_set_state
 * 
 * @param led_on 0 to turn led off, ~0 to turn on
 */
void gpio_status_led_set_state(uint8_t led_on);

#endif /* GPIO_STATUS_LED_H_ */
/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/