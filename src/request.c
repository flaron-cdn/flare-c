#include "flare/request.h"
#include "flare/runtime.h"
#include "flare/env.h"

static flare_status_t read_packed_string(int64_t packed,
                                         const char **out_str, size_t *out_len) {
    if (out_str) *out_str = NULL;
    if (out_len) *out_len = 0;
    if (packed == 0) {
        return FLARE_ERR_NOT_FOUND;
    }
    int32_t handle = 0, len = 0;
    flare_decode_ptr_len(packed, &handle, &len);
    if (handle == 0 || len < 0) {
        return FLARE_ERR_HOST;
    }
    if (out_str) *out_str = (const char *)flare_arena_addr(handle);
    if (out_len) *out_len = (size_t)len;
    return FLARE_OK;
}

static flare_status_t read_packed_bytes(int64_t packed,
                                        const uint8_t **out_bytes, size_t *out_len) {
    if (out_bytes) *out_bytes = NULL;
    if (out_len)   *out_len = 0;
    if (packed == 0) {
        return FLARE_ERR_NOT_FOUND;
    }
    int32_t handle = 0, len = 0;
    flare_decode_ptr_len(packed, &handle, &len);
    if (handle == 0 || len < 0) {
        return FLARE_ERR_HOST;
    }
    if (out_bytes) *out_bytes = flare_arena_addr(handle);
    if (out_len)   *out_len = (size_t)len;
    return FLARE_OK;
}

flare_status_t flare_req_method(const char **out_str, size_t *out_len) {
    return read_packed_string(req_method(), out_str, out_len);
}

flare_status_t flare_req_url(const char **out_str, size_t *out_len) {
    return read_packed_string(req_url(), out_str, out_len);
}

flare_status_t flare_req_header(const char *name, size_t name_len,
                                const char **out_value, size_t *out_value_len) {
    if (name == NULL || name_len == 0) {
        if (out_value)     *out_value = NULL;
        if (out_value_len) *out_value_len = 0;
        return FLARE_ERR_INVALID_ARG;
    }
    int32_t name_handle = flare_copy_to_arena(name, name_len);
    if (name_handle == 0) return FLARE_ERR_OUT_OF_MEMORY;
    int64_t packed = req_header_get(name_handle, (int32_t)name_len);
    return read_packed_string(packed, out_value, out_value_len);
}

flare_status_t flare_req_body(const uint8_t **out_body, size_t *out_len) {
    return read_packed_bytes(req_body(), out_body, out_len);
}
