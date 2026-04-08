#include <flare.h>

/*
 * Per-edge counter persisted in Spark. Each request increments the value at
 * the "hits" key and returns the running total. Spark gives every site a
 * private K/V namespace; the value is automatically expired after 24h here.
 *
 * Requires capability: writes_spark_kv = true
 */

static char buf[64];

static size_t format_uint(unsigned long long v, char *out) {
    if (v == 0) { out[0] = '0'; return 1; }
    char tmp[24];
    size_t n = 0;
    while (v) { tmp[n++] = (char)('0' + (v % 10)); v /= 10; }
    for (size_t i = 0; i < n; i++) out[i] = tmp[n - 1 - i];
    return n;
}

static unsigned long long parse_uint(const uint8_t *s, size_t len) {
    unsigned long long v = 0;
    for (size_t i = 0; i < len; i++) {
        if (s[i] < '0' || s[i] > '9') break;
        v = v * 10 + (unsigned long long)(s[i] - '0');
    }
    return v;
}

static flare_action_t handle_request(void) {
    unsigned long long hits = 0;

    flare_spark_entry_t entry = {0};
    if (flare_spark_get("hits", 4, &entry) == FLARE_OK) {
        hits = parse_uint(entry.value, entry.value_len);
    }
    hits++;

    char enc[32];
    size_t enc_len = format_uint(hits, enc);
    (void)flare_spark_set("hits", 4, (const uint8_t *)enc, enc_len, 86400);

    /* Body: "hits: NNN" */
    static const char prefix[] = "hits: ";
    size_t off = 0;
    for (size_t i = 0; i < sizeof(prefix) - 1; i++) buf[off++] = prefix[i];
    for (size_t i = 0; i < enc_len; i++) buf[off++] = enc[i];

    flare_resp_set_status(200);
    flare_resp_set_header("content-type", 12, "text/plain", 10);
    flare_resp_set_body((const uint8_t *)buf, off);
    return FLARE_ACTION_RESPOND;
}

FLARE_EXPORT_HANDLE_REQUEST(handle_request)
