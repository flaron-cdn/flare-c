#ifndef FLARE_SPARK_H
#define FLARE_SPARK_H

#include <stdint.h>
#include <stddef.h>

#include "flare/status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const uint8_t *value;
    size_t         value_len;
    uint32_t       ttl_secs;
} flare_spark_entry_t;

flare_status_t flare_spark_get(const char *key, size_t key_len,
                               flare_spark_entry_t *out_entry);

flare_status_t flare_spark_set(const char *key, size_t key_len,
                               const uint8_t *value, size_t value_len,
                               uint32_t ttl_secs);

void flare_spark_delete(const char *key, size_t key_len);

flare_status_t flare_spark_list(const uint8_t **out_json, size_t *out_len);

flare_status_t flare_spark_pull(const char *origin_node, size_t origin_len,
                                const char *keys_json, size_t keys_json_len,
                                uint32_t *out_count);

#ifdef __cplusplus
}
#endif

#endif
