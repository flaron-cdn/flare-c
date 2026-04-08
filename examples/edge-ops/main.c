#include <flare.h>

/*
 * Demonstrates several edge ops in one flare:
 *   - SHA-256 hash of "hello"
 *   - UUID v7 generation
 *   - RFC3339 timestamp
 *
 * Builds a small text response showing each result. Useful as a smoke test
 * after deployment.
 */

static char buf[512];

static size_t append(char *dst, size_t off, const char *src, size_t n) {
    for (size_t i = 0; i < n; i++) dst[off++] = src[i];
    return off;
}

static size_t append_lit(char *dst, size_t off, const char *lit) {
    while (*lit) dst[off++] = *lit++;
    return off;
}

static flare_action_t handle_request(void) {
    size_t off = 0;

    const char *hash = NULL;
    size_t hash_len = 0;
    if (flare_crypto_hash("sha256", 6, "hello", 5, &hash, &hash_len) == FLARE_OK) {
        off = append_lit(buf, off, "sha256(\"hello\") = ");
        off = append(buf, off, hash, hash_len);
        off = append_lit(buf, off, "\n");
    }

    const char *uuid = NULL;
    size_t uuid_len = 0;
    if (flare_id_uuid_v7(&uuid, &uuid_len) == FLARE_OK) {
        off = append_lit(buf, off, "uuid v7        = ");
        off = append(buf, off, uuid, uuid_len);
        off = append_lit(buf, off, "\n");
    }

    const char *ts = NULL;
    size_t ts_len = 0;
    if (flare_timestamp(FLARE_TIME_RFC3339, &ts, &ts_len) == FLARE_OK) {
        off = append_lit(buf, off, "edge time      = ");
        off = append(buf, off, ts, ts_len);
        off = append_lit(buf, off, "\n");
    }

    flare_resp_set_status(200);
    flare_resp_set_header("content-type", 12, "text/plain", 10);
    flare_resp_set_body((const uint8_t *)buf, off);
    return FLARE_ACTION_RESPOND;
}

FLARE_EXPORT_HANDLE_REQUEST(handle_request)
