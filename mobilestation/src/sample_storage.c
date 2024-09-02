/* ---------------------------------------------------------------------------
 *  mobilestation
 * ---------------------------------------------------------------------------
 *  Name: sample_storage.c
 * --------------------------------------------------------------------------*/
#include "sample_storage.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(SS, LOG_LEVEL_DBG);

static uint16_t PullIdx = 0;
static uint16_t PushIdx = 0;
static uint8_t SampleStorage[NVS_SAMPLE_MAX_NUM][NVS_SAMPLE_SIZE];

static bool save_sample_idx(void *sample, size_t len, uint16_t idx) {
    memcpy(SampleStorage[idx], sample, len);
    return (true);
}

static bool read_sample_idx(void *sample, size_t len, uint16_t idx) {
    memcpy(sample, SampleStorage[idx], len);
    return (true);
}

void sample_storage_init(void) {
    ;
}

void sample_storage_mark_as_sent(uint16_t pull_idx) {
    PullIdx = (pull_idx + 1) % NVS_SAMPLE_MAX_NUM;
}

bool sample_storage_pull(void *sample, size_t len, uint16_t *pull_idx) {
    bool res = false;
    uint8_t buffer[NVS_SAMPLE_SIZE];

    if (PushIdx != PullIdx) {
        res = read_sample_idx(buffer, NVS_SAMPLE_SIZE, PullIdx);
        if (!res) {
            LOG_ERR("Failed to read sample_idx %u", PullIdx);
        } else {
            *pull_idx = PullIdx;
            memcpy(sample, buffer, len);
            res = true;
        }
    } else {
        LOG_INF("No samples waiting to send");
    }

    return (res);
}

bool sample_storage_push(void *sample, size_t len) {
    bool res = false;
    uint8_t buffer[NVS_SAMPLE_SIZE];

    LOG_INF("PushIdx %u PullIdx %u", PushIdx, PullIdx);
    if ((PushIdx == PullIdx - 1) ||
        ((PushIdx == NVS_SAMPLE_MAX_NUM - 1) && (0 == PullIdx))) {
        LOG_ERR("No free space to push sample in");
    } else {
        memset(buffer, 0x00, NVS_SAMPLE_SIZE);
        memcpy(buffer, sample, len);
        if (save_sample_idx(buffer, NVS_SAMPLE_SIZE, PushIdx)) {
            PushIdx = (PushIdx + 1) % NVS_SAMPLE_MAX_NUM;
            res = true;
        }
    }

    return (res);
}

/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/