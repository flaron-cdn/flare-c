#include "flare/plasma.h"
#include "flare/runtime.h"
#include "flare/env.h"

static flare_status_t map_plasma_error(int32_t code) {
    switch (code) {
    case 0:  return FLARE_OK;
    case 1:  return FLARE_ERR_PLASMA_NOT_AVAIL;
    case 2:  return FLARE_ERR_PLASMA_WRITE_LIMIT;
    case 3:  return FLARE_ERR_PLASMA_TOO_LARGE;
    case 4:  return FLARE_ERR_PLASMA_BAD_KEY;
    case 5:  return FLARE_ERR_PLASMA_NO_CAPAB;
    case 6:  return FLARE_ERR_PLASMA_INTERNAL;
    default: return FLARE_ERR_HOST;
    }
}

flare_status_t flare_plasma_get(const char *key, size_t key_len,
                                const uint8_t **out_value, size_t *out_len) {
    if (out_value) *out_value = NULL;
    if (out_len)   *out_len = 0;
    if (key == NULL || key_len == 0) return FLARE_ERR_INVALID_ARG;

    int32_t key_h = flare_copy_to_arena(key, key_len);
    if (key_h == 0) return FLARE_ERR_OUT_OF_MEMORY;

    int64_t packed = plasma_get(key_h, (int32_t)key_len);
    if (packed == 0) return FLARE_ERR_NOT_FOUND;

    int32_t handle = 0, len = 0;
    flare_decode_ptr_len(packed, &handle, &len);
    if (handle == 0 || len < 0) return FLARE_ERR_HOST;
    if (out_value) *out_value = flare_arena_addr(handle);
    if (out_len)   *out_len = (size_t)len;
    return FLARE_OK;
}

flare_status_t flare_plasma_set(const char *key, size_t key_len,
                                const uint8_t *value, size_t value_len) {
    if (key == NULL || key_len == 0) return FLARE_ERR_INVALID_ARG;
    int32_t key_h = flare_copy_to_arena(key, key_len);
    if (key_h == 0) return FLARE_ERR_OUT_OF_MEMORY;
    int32_t val_h = 0;
    if (value_len > 0 && value != NULL) {
        val_h = flare_copy_to_arena(value, value_len);
        if (val_h == 0) return FLARE_ERR_OUT_OF_MEMORY;
    }
    return map_plasma_error(plasma_set(key_h, (int32_t)key_len,
                                       val_h, (int32_t)value_len));
}

flare_status_t flare_plasma_delete(const char *key, size_t key_len) {
    if (key == NULL || key_len == 0) return FLARE_ERR_INVALID_ARG;
    int32_t key_h = flare_copy_to_arena(key, key_len);
    if (key_h == 0) return FLARE_ERR_OUT_OF_MEMORY;
    return map_plasma_error(plasma_delete(key_h, (int32_t)key_len));
}

static flare_status_t plasma_apply(int64_t (*op)(int32_t, int32_t, int64_t),
                                   const char *key, size_t key_len,
                                   int64_t delta, int64_t *out_value) {
    if (out_value) *out_value = 0;
    if (key == NULL || key_len == 0) return FLARE_ERR_INVALID_ARG;
    int32_t key_h = flare_copy_to_arena(key, key_len);
    if (key_h == 0) return FLARE_ERR_OUT_OF_MEMORY;

    int64_t packed = op(key_h, (int32_t)key_len, delta);
    if (packed == 0) return FLARE_ERR_HOST;

    int32_t handle = 0, len = 0;
    flare_decode_ptr_len(packed, &handle, &len);
    if (handle == 0 || len < 8) return FLARE_ERR_HOST;

    const uint8_t *raw = flare_arena_addr(handle);
    if (raw == NULL) return FLARE_ERR_HOST;

    uint64_t v = 0;
    for (int i = 0; i < 8; i++) {
        v |= ((uint64_t)raw[i]) << (i * 8);
    }
    if (out_value) *out_value = (int64_t)v;
    return FLARE_OK;
}

flare_status_t flare_plasma_increment(const char *key, size_t key_len,
                                      int64_t delta, int64_t *out_value) {
    return plasma_apply(plasma_increment, key, key_len, delta, out_value);
}

flare_status_t flare_plasma_decrement(const char *key, size_t key_len,
                                      int64_t delta, int64_t *out_value) {
    return plasma_apply(plasma_decrement, key, key_len, delta, out_value);
}

flare_status_t flare_plasma_list(const uint8_t **out_json, size_t *out_len) {
    if (out_json) *out_json = NULL;
    if (out_len)  *out_len = 0;
    int64_t packed = plasma_list();
    if (packed == 0) return FLARE_ERR_NOT_FOUND;
    int32_t handle = 0, len = 0;
    flare_decode_ptr_len(packed, &handle, &len);
    if (handle == 0 || len < 0) return FLARE_ERR_HOST;
    if (out_json) *out_json = flare_arena_addr(handle);
    if (out_len)  *out_len = (size_t)len;
    return FLARE_OK;
}
