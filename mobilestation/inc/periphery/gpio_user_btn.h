/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: gpio_user_btn.h
 * --------------------------------------------------------------------------*/
#ifndef GPIO_USER_BTN_H_
#define GPIO_USER_BTN_H_

#include <stdint.h>

/**
 * @brief gpio_user_btn_init
 * Initialise as pulled up.
 */
void gpio_user_btn_init(void);

/**
 * @brief 
 * 
 * @param cb Provided callback will be run for every edge occured on button pin.
 * No contact vibrations avoidance implemented. If state = 0 means button pressed.
 * Released otherwise.
 */
void gpio_user_btn_register_cb(void (*cb)(uint8_t state));

#endif /* GPIO_USER_BTN_H_ */
/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/