/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: gpio_sim800l.h
 * --------------------------------------------------------------------------*/
#ifndef GPIO_SIM800L_H_
#define GPIO_SIM800L_H_

/**
 * @brief gpio_sim800l_init
 * DT defined as gsmreset_gpios. Initialise and configure peripherals.
 */
void gpio_sim800l_init(void);

/**
 * @brief gpio_sim800l_rst_up
 * Go to logical pin high.
 */
void gpio_sim800l_rst_up(void);

/**
 * @brief gpio_sim800l_rst_down
 * Go to logical pin low.
 */
void gpio_sim800l_rst_down(void);

/**
 * @brief gpio_sim800l_dtr_down
 * Go to logical pin low.
 */
void gpio_sim800l_dtr_down(void);

/**
 * @brief gpio_sim800l_dtr_up
 * Go to logical pin high.
 */
void gpio_sim800l_dtr_up(void);

#endif /* GPIO_SIM800L_H_ */
/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/