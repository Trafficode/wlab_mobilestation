/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: ble_worker.h
 * --------------------------------------------------------------------------*/
#ifndef BLE_WORKER_H_
#define BLE_WORKER_H_

#include <stdint.h>

/**
 * @brief Initialize periphery, and configure advertising name.
 * BLE OTA and BLE NUS services enabled.
 * @param adv_time_sec Bluetooth advertising will be disabled after this time.
 */
void ble_worker_init(int32_t adv_time_sec);

#endif /* BLE_WORKER_H_ */
/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/