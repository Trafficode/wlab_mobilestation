/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: sample_storage.h
 * --------------------------------------------------------------------------*/
#ifndef SAMPLE_STORAGE_H_
#define SAMPLE_STORAGE_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define NVS_SAMPLE_MAX_NUM (32)
#define NVS_SAMPLE_SIZE    (64)

/**
 * @brief Pull sample from storage that failed to send
 * 
 * @param sample Pointer where sample will be send
 * @param len Not verify if exceed max sample length equals NVS_SAMPLE_SIZE
 * @param pull_idx Return index of sample that was stored in sample pointer
 * @return true Sample successfully taken
 * @return false No waiting sample in storage
 */
bool sample_storage_pull(void *sample, size_t len, uint16_t *pull_idx);

/**
 * @brief Mark giver sample index as send succesfully.
 * 
 * @param pull_idx 
 */
void sample_storage_mark_as_sent(uint16_t pull_idx);

/**
 * @brief Save sample that fails send. 
 * 
 * @param sample Pointer with sample to store
 * @param len Length of data to send
 * @return true 
 * @return false 
 */
bool sample_storage_push(void *sample, size_t len);

#endif /* SAMPLE_STORAGE_H_ */
/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/