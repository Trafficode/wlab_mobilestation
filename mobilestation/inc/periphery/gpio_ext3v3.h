/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: gpio_ext3v3.h
 * --------------------------------------------------------------------------*/
#ifndef GPIO_EXT3V3_H_
#define GPIO_EXT3V3_H_

/**
 * @brief gpio_ext3v3_init
 * DT defined as external3v3_gpios. Initialise and configure peripherals.
 */
void gpio_ext3v3_init(void);

/**
 * @brief gpio_ext3v3_enable
 * Go to logical pin high.
 */
void gpio_ext3v3_enable(void);

/**
 * @brief gpio_ext3v3_disable
 * Go to logical pin low.
 */
void gpio_ext3v3_disable(void);

#endif /* GPIO_EXT3V3_H_ */
/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/