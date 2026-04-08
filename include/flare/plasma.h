#ifndef FLARE_PLASMA_H
#define FLARE_PLASMA_H

#include <stdint.h>
#include <stddef.h>

#include "flare/status.h"

#ifdef __cplusplus
extern "C" {
#endif

flare_status_t flare_plasma_get(const char *key, size_t key_len,
                                const uint8_t **out_value, size_t *out_len);

flare_status_t flare_plasma_set(const char *key, size_t key_len,
                                const uint8_t *value, size_t value_len);

flare_status_t flare_plasma_delete(const char *key, size_t key_len);

flare_status_t flare_plasma_increment(const char *key, size_t key_len,
                                      int64_t delta, int64_t *out_value);

flare_status_t flare_plasma_decrement(const char *key, size_t key_len,
                                      int64_t delta, int64_t *out_value);

flare_status_t flare_plasma_list(const uint8_t **out_json, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif
