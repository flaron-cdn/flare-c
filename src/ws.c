#include "flare/ws.h"
#include "flare/runtime.h"
#include "flare/env.h"

flare_status_t flare_ws_send(const uint8_t *data, size_t len) {
    int32_t handle = 0;
    if (data != NULL && len > 0) {
        handle = flare_copy_to_arena(data, len);
        if (handle == 0) return FLARE_ERR_OUT_OF_MEMORY;
    }
    int32_t code = ws_send(handle, (int32_t)len);
    return code == 0 ? FLARE_OK : FLARE_ERR_WS_SEND;
}

flare_status_t flare_ws_send_text(const char *text, size_t len) {
    return flare_ws_send((const uint8_t *)text, len);
}

void flare_ws_close(uint16_t code) {
    ws_close_conn((int32_t)code);
}

static flare_status_t read_packed(int64_t packed,
                                  const void **out_ptr, size_t *out_len) {
    if (out_ptr) *out_ptr = NULL;
    if (out_len) *out_len = 0;
    if (packed == 0) return FLARE_ERR_NOT_FOUND;
    int32_t handle = 0, len = 0;
    flare_decode_ptr_len(packed, &handle, &len);
    if (handle == 0 || len < 0) return FLARE_ERR_HOST;
    if (out_ptr) *out_ptr = flare_arena_addr(handle);
    if (out_len) *out_len = (size_t)len;
    return FLARE_OK;
}

flare_status_t flare_ws_conn_id(const char **out_id, size_t *out_len) {
    return read_packed(ws_conn_id(), (const void **)out_id, out_len);
}

flare_status_t flare_ws_event_type(const char **out_type, size_t *out_len) {
    return read_packed(ws_event_type(), (const void **)out_type, out_len);
}

flare_status_t flare_ws_event_data(const uint8_t **out_data, size_t *out_len) {
    return read_packed(ws_event_data(), (const void **)out_data, out_len);
}

uint16_t flare_ws_close_code(void) {
    return (uint16_t)ws_close_code();
}
