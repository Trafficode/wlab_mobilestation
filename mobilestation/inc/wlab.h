/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: wlab.h
 * --------------------------------------------------------------------------*/
#ifndef WLAB_H_
#define WLAB_H_

#include <stdbool.h>

#define WLAB_DEFAULT_PUB_PERIOD_MIN INT64_C(10)
#define WLAB_DEFAULT_SAMPLE_TOPIC   ("/wlabdb/bin")
#define WLAB_DEFAULT_ARCH_PUB_NUM   UINT8_C(4)

/**
 * @brief wlab_init
 * 
 */
void wlab_init(void);

/**
 * @brief wlab_proc
 * 
 */
void wlab_proc(void);

/**
 * @brief Publish wlab sample or recover sample that failed
 * previously.
 * 
 * @param resend true to recover sample, false to send actual sample
 * @return true Success
 * @return false Failed
 */
bool wlab_publish2(bool resend);

#endif /* WLAB_H_ */
/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/