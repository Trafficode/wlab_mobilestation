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

static uint16_t StoredN = 0;
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
    LOG_INF("sample_storage_init");
}

uint16_t sample_storage_get_samples_num(void) {
    return (StoredN);
}

void sample_storage_mark_as_sent(uint16_t pull_idx) {
    PullIdx = (pull_idx + 1) % NVS_SAMPLE_MAX_NUM;
    StoredN--;
}

bool sample_storage_pull(void *sample, size_t pull_len, uint16_t *pull_idx) {
    bool res = false;
    uint8_t buffer[NVS_SAMPLE_SIZE];

    if (pull_len > NVS_SAMPLE_SIZE) {
        LOG_ERR("pull_len > NVS_SAMPLE_SIZE");
        goto DONE;
    }

    LOG_INF("PULL PushIdx %u PullIdx %u N %u", PushIdx, PullIdx, StoredN);

    if (StoredN > 0) {
        res = read_sample_idx(buffer, NVS_SAMPLE_SIZE, PullIdx);
        if (!res) {
            LOG_ERR("Failed to read sample_idx %u", PullIdx);
        } else {
            *pull_idx = PullIdx;
            memcpy(sample, buffer, pull_len);
            res = true;
        }
    } else {
        LOG_INF("No samples waiting to send");
    }

DONE:
    return (res);
}

bool sample_storage_push(void *sample, size_t push_len) {
    bool res = false;
    uint8_t buffer[NVS_SAMPLE_SIZE];

    if (push_len > NVS_SAMPLE_SIZE) {
        LOG_ERR("push_len > NVS_SAMPLE_SIZE");
        goto DONE;
    }

    LOG_INF("PUSH PushIdx %u PullIdx %u N %u", PushIdx, PullIdx, StoredN);
    if (NVS_SAMPLE_MAX_NUM == StoredN) {
        LOG_ERR("No free space to push sample in");
    } else {
        memset(buffer, 0x00, NVS_SAMPLE_SIZE);
        memcpy(buffer, sample, push_len);
        if (save_sample_idx(buffer, NVS_SAMPLE_SIZE, PushIdx)) {
            PushIdx = (PushIdx + 1) % NVS_SAMPLE_MAX_NUM;
            StoredN++;
            res = true;
        }
    }

DONE:
    return (res);
}

/* ---------------------------------------------------------------------------
 * end of file
 * --------------------------------------------------------------------------*/