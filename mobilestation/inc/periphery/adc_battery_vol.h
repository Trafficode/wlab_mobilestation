/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: adc_battery_vol.h
 * --------------------------------------------------------------------------*/
#ifndef ADC_BATTERY_VOL_H_
#define ADC_BATTERY_VOL_H_

#include <stdint.h>

/**
 * @brief adc_battery_vol_init
 * 
 */
void adc_battery_vol_init(void);

/**
 * @brief adc_battery_vol_get_milliv
 * 
 * @return int32_t Battery voltage in millivolts
 */
int32_t adc_battery_vol_get_milliv(void);

#endif /* ADC_BATTERY_VOL_H_ */
/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/