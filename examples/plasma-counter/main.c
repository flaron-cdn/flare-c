#include <flare.h>

/*
 * Cross-edge counter using Plasma (CRDT-replicated K/V). Every request
 * increments "global_hits" by 1 and returns the new value. Unlike Spark,
 * Plasma values are eventually-consistent across the entire fleet.
 *
 * Requires capability: writes_plasma_kv = true
 */

static char buf[64];

static size_t format_int(long long v, char *out) {
    int neg = 0;
    if (v < 0) { neg = 1; v = -v; }
    if (v == 0) { out[0] = '0'; return 1; }
    char tmp[24];
    size_t n = 0;
    while (v) { tmp[n++] = (char)('0' + (v % 10)); v /= 10; }
    size_t off = 0;
    if (neg) out[off++] = '-';
    for (size_t i = 0; i < n; i++) out[off++] = tmp[n - 1 - i];
    return off;
}

static flare_action_t handle_request(void) {
    int64_t total = 0;
    if (flare_plasma_increment("global_hits", 11, 1, &total) != FLARE_OK) {
        flare_resp_set_status(500);
        flare_resp_set_body((const uint8_t *)"plasma error", 12);
        return FLARE_ACTION_RESPOND;
    }

    static const char prefix[] = "global hits: ";
    size_t off = 0;
    for (size_t i = 0; i < sizeof(prefix) - 1; i++) buf[off++] = prefix[i];
    off += format_int(total, buf + off);

    flare_resp_set_status(200);
    flare_resp_set_header("content-type", 12, "text/plain", 10);
    flare_resp_set_body((const uint8_t *)buf, off);
    return FLARE_ACTION_RESPOND;
}

FLARE_EXPORT_HANDLE_REQUEST(handle_request)
