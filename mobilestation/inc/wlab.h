/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: wlab.h
 * --------------------------------------------------------------------------*/
#ifndef WLAB_H_
#define WLAB_H_

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

#endif /* WLAB_H_ */
/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/