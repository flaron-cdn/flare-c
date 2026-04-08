#include "flare/spark.h"
#include "flare/runtime.h"
#include "flare/env.h"

static flare_status_t map_spark_set_error(int32_t code) {
    switch (code) {
    case 0:  return FLARE_OK;
    case 1:  return FLARE_ERR_SPARK_INVALID_TTL;
    case 2:  return FLARE_ERR_SPARK_TOO_LARGE;
    case 3:  return FLARE_ERR_SPARK_WRITE_LIMIT;
    case 4:  return FLARE_ERR_SPARK_QUOTA;
    case 5:  return FLARE_ERR_SPARK_NOT_AVAIL;
    case 6:  return FLARE_ERR_SPARK_INTERNAL;
    case 7:  return FLARE_ERR_SPARK_READ_LIMIT;
    case 8:  return FLARE_ERR_SPARK_BAD_KEY;
    case 9:  return FLARE_ERR_SPARK_NO_CAPAB;
    default: return FLARE_ERR_HOST;
    }
}

flare_status_t flare_spark_get(const char *key, size_t key_len,
                               flare_spark_entry_t *out_entry) {
    if (out_entry == NULL) return FLARE_ERR_INVALID_ARG;
    out_entry->value = NULL;
    out_entry->value_len = 0;
    out_entry->ttl_secs = 0;

    if (key == NULL || key_len == 0) return FLARE_ERR_INVALID_ARG;

    int32_t key_handle = flare_copy_to_arena(key, key_len);
    if (key_handle == 0) return FLARE_ERR_OUT_OF_MEMORY;

    int64_t packed = spark_get(key_handle, (int32_t)key_len);
    if (packed == 0) return FLARE_ERR_NOT_FOUND;

    int32_t handle = 0, len = 0;
    flare_decode_ptr_len(packed, &handle, &len);
    if (handle == 0 || len < 4) {
        return FLARE_ERR_HOST;
    }

    const uint8_t *raw = flare_arena_addr(handle);
    if (raw == NULL) return FLARE_ERR_HOST;

    /* Wire format: 4-byte LE u32 TTL prefix + value bytes. ALWAYS strip. */
    uint32_t ttl = (uint32_t)raw[0]
                 | ((uint32_t)raw[1] << 8)
                 | ((uint32_t)raw[2] << 16)
                 | ((uint32_t)raw[3] << 24);

    out_entry->value     = raw + 4;
    out_entry->value_len = (size_t)(len - 4);
    out_entry->ttl_secs  = ttl;
    return FLARE_OK;
}

flare_status_t flare_spark_set(const char *key, size_t key_len,
                               const uint8_t *value, size_t value_len,
                               uint32_t ttl_secs) {
    if (key == NULL || key_len == 0) return FLARE_ERR_INVALID_ARG;
    int32_t key_handle = flare_copy_to_arena(key, key_len);
    if (key_handle == 0) return FLARE_ERR_OUT_OF_MEMORY;

    int32_t val_handle = 0;
    if (value_len > 0 && value != NULL) {
        val_handle = flare_copy_to_arena(value, value_len);
        if (val_handle == 0) return FLARE_ERR_OUT_OF_MEMORY;
    }

    int32_t code = spark_set(key_handle, (int32_t)key_len,
                             val_handle, (int32_t)value_len,
                             (int32_t)ttl_secs);
    return map_spark_set_error(code);
}

void flare_spark_delete(const char *key, size_t key_len) {
    if (key == NULL || key_len == 0) return;
    int32_t key_handle = flare_copy_to_arena(key, key_len);
    if (key_handle == 0) return;
    spark_delete(key_handle, (int32_t)key_len);
}

flare_status_t flare_spark_list(const uint8_t **out_json, size_t *out_len) {
    if (out_json) *out_json = NULL;
    if (out_len)  *out_len = 0;

    int64_t packed = spark_list();
    if (packed == 0) return FLARE_ERR_NOT_FOUND;

    int32_t handle = 0, len = 0;
    flare_decode_ptr_len(packed, &handle, &len);
    if (handle == 0 || len < 0) return FLARE_ERR_HOST;

    if (out_json) *out_json = flare_arena_addr(handle);
    if (out_len)  *out_len = (size_t)len;
    return FLARE_OK;
}

flare_status_t flare_spark_pull(const char *origin_node, size_t origin_len,
                                const char *keys_json, size_t keys_json_len,
                                uint32_t *out_count) {
    if (out_count) *out_count = 0;
    if (origin_node == NULL || origin_len == 0) return FLARE_ERR_INVALID_ARG;
    if (keys_json == NULL || keys_json_len == 0) return FLARE_ERR_INVALID_ARG;

    int32_t origin_h = flare_copy_to_arena(origin_node, origin_len);
    int32_t keys_h   = flare_copy_to_arena(keys_json, keys_json_len);
    if (origin_h == 0 || keys_h == 0) return FLARE_ERR_OUT_OF_MEMORY;

    int32_t code = spark_pull(origin_h, (int32_t)origin_len,
                              keys_h, (int32_t)keys_json_len);
    if (code < 0) {
        return map_spark_set_error(-code);
    }
    if (out_count) *out_count = (uint32_t)code;
    return FLARE_OK;
}
