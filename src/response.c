#include "flare/response.h"
#include "flare/runtime.h"
#include "flare/env.h"

void flare_resp_set_status(uint16_t code) {
    resp_set_status((int32_t)code);
}

void flare_resp_set_header(const char *name, size_t name_len,
                           const char *value, size_t value_len) {
    if (name == NULL || name_len == 0) return;
    int32_t name_handle = flare_copy_to_arena(name, name_len);
    if (name_handle == 0) return;
    int32_t val_handle = 0;
    if (value != NULL && value_len > 0) {
        val_handle = flare_copy_to_arena(value, value_len);
        if (val_handle == 0) return;
    }
    resp_header_set(name_handle, (int32_t)name_len, val_handle, (int32_t)value_len);
}

void flare_resp_set_body(const uint8_t *body, size_t len) {
    if (len == 0 || body == NULL) {
        resp_body_set(0, 0);
        return;
    }
    int32_t handle = flare_copy_to_arena(body, len);
    if (handle == 0) return;
    resp_body_set(handle, (int32_t)len);
}

void flare_resp_set_body_str(const char *body, size_t len) {
    flare_resp_set_body((const uint8_t *)body, len);
}
