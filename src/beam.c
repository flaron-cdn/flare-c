#include "flare/beam.h"
#include "flare/runtime.h"
#include "flare/env.h"

flare_status_t flare_beam_fetch(const char *url, size_t url_len,
                                const char *opts_json, size_t opts_json_len,
                                const uint8_t **out_response_json, size_t *out_len) {
    if (out_response_json) *out_response_json = NULL;
    if (out_len)           *out_len = 0;

    if (url == NULL || url_len == 0) return FLARE_ERR_INVALID_ARG;

    int32_t url_h = flare_copy_to_arena(url, url_len);
    if (url_h == 0) return FLARE_ERR_OUT_OF_MEMORY;

    int32_t opts_h = 0;
    if (opts_json != NULL && opts_json_len > 0) {
        opts_h = flare_copy_to_arena(opts_json, opts_json_len);
        if (opts_h == 0) return FLARE_ERR_OUT_OF_MEMORY;
    }

    int64_t packed = beam_fetch(url_h, (int32_t)url_len,
                                opts_h, (int32_t)opts_json_len);
    if (packed == 0) return FLARE_ERR_NO_RESPONSE;

    int32_t handle = 0, len = 0;
    flare_decode_ptr_len(packed, &handle, &len);
    if (handle == 0 || len < 0) return FLARE_ERR_HOST;
    if (out_response_json) *out_response_json = flare_arena_addr(handle);
    if (out_len)           *out_len = (size_t)len;
    return FLARE_OK;
}
