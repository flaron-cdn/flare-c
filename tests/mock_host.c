#include "mock_host.h"

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>

#include "flare/runtime.h"
#include "flare/env.h"

extern uint8_t flare_arena_buf[];

#define MOCK_MAX_HEADERS  32
#define MOCK_MAX_KV       64
#define MOCK_KEY_BUF      256
#define MOCK_VAL_BUF      4096
#define MOCK_BUFFER_SIZE  16384

typedef struct {
    char     name[128];
    char     value[1024];
    int      in_use;
} mock_header_t;

typedef struct {
    char    key[MOCK_KEY_BUF];
    uint8_t value[MOCK_VAL_BUF];
    size_t  value_len;
    uint32_t ttl_secs;
    int     in_use;
} mock_kv_entry_t;

typedef struct {
    char    key[MOCK_KEY_BUF];
    char    value[MOCK_VAL_BUF];
    int     in_use;
} mock_secret_t;

typedef struct {
    int     level;
    char    msg[1024];
} mock_log_t;

typedef struct {
    char     method[32];
    char     url[2048];
    mock_header_t req_headers[MOCK_MAX_HEADERS];
    uint8_t  req_body[MOCK_VAL_BUF];
    size_t   req_body_len;

    int      resp_status;
    mock_header_t resp_headers[MOCK_MAX_HEADERS];
    uint8_t  resp_body[MOCK_VAL_BUF];
    size_t   resp_body_len;

    mock_kv_entry_t spark[MOCK_MAX_KV];
    int32_t  spark_next_error;

    mock_kv_entry_t plasma[MOCK_MAX_KV];
    int64_t  plasma_counters[MOCK_MAX_KV];
    int      plasma_counters_used[MOCK_MAX_KV];

    mock_secret_t secrets[MOCK_MAX_KV];

    char     random_hex[1024];
    char     hash_hex[1024];
    char     hmac_hex[1024];
    char     jwt[1024];
    char     aes_encrypt_b64[1024];
    uint8_t  aes_decrypt[MOCK_VAL_BUF];
    size_t   aes_decrypt_len;
    char     uuid[64];
    char     ulid[64];
    char     nanoid[64];
    char     ksuid[64];
    char     snowflake[64];
    char     timestamp[64];
    char     beam_response[MOCK_VAL_BUF];
    char     b64_encode[MOCK_VAL_BUF];
    uint8_t  b64_decode[MOCK_VAL_BUF];
    size_t   b64_decode_len;
    char     hex_encode[MOCK_VAL_BUF];
    uint8_t  hex_decode[MOCK_VAL_BUF];
    size_t   hex_decode_len;
    char     url_encode[MOCK_VAL_BUF];
    char     url_decode[MOCK_VAL_BUF];

    mock_log_t logs[64];
    int        log_count;

    char     ws_event_type[32];
    uint8_t  ws_event_data[MOCK_VAL_BUF];
    size_t   ws_event_data_len;
    char     ws_conn_id[128];
    int      ws_close_code;
    uint8_t  ws_last_send[MOCK_VAL_BUF];
    size_t   ws_last_send_len;
    int      ws_send_count;
    int      ws_close_called;
    int      ws_close_code_seen;
    int      ws_send_error;
} mock_state_t;

static mock_state_t g_mock;

static int32_t pack_to_arena(const uint8_t *src, size_t len, uint64_t *out_packed) {
    if (len == 0) {
        *out_packed = 0;
        return 0;
    }
    int32_t handle = flare_alloc((int32_t)len);
    if (handle == 0) {
        *out_packed = 0;
        return -1;
    }
    memcpy(flare_arena_addr(handle), src, len);
    *out_packed = ((uint64_t)(uint32_t)handle << 32) | (uint64_t)(uint32_t)len;
    return 0;
}

static int find_header(const mock_header_t *headers, const char *name) {
    for (int i = 0; i < MOCK_MAX_HEADERS; i++) {
        if (headers[i].in_use && strcasecmp(headers[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static int put_header(mock_header_t *headers, const char *name, const char *value) {
    int idx = find_header(headers, name);
    if (idx < 0) {
        for (int i = 0; i < MOCK_MAX_HEADERS; i++) {
            if (!headers[i].in_use) { idx = i; break; }
        }
    }
    if (idx < 0) return -1;
    headers[idx].in_use = 1;
    snprintf(headers[idx].name, sizeof(headers[idx].name), "%s", name);
    snprintf(headers[idx].value, sizeof(headers[idx].value), "%s", value);
    return 0;
}

static int find_kv(mock_kv_entry_t *table, const char *key) {
    for (int i = 0; i < MOCK_MAX_KV; i++) {
        if (table[i].in_use && strcmp(table[i].key, key) == 0) {
            return i;
        }
    }
    return -1;
}

static int put_kv(mock_kv_entry_t *table, const char *key,
                  const uint8_t *value, size_t len, uint32_t ttl) {
    int idx = find_kv(table, key);
    if (idx < 0) {
        for (int i = 0; i < MOCK_MAX_KV; i++) {
            if (!table[i].in_use) { idx = i; break; }
        }
    }
    if (idx < 0) return -1;
    if (len > MOCK_VAL_BUF) len = MOCK_VAL_BUF;
    table[idx].in_use = 1;
    snprintf(table[idx].key, sizeof(table[idx].key), "%s", key);
    memcpy(table[idx].value, value, len);
    table[idx].value_len = len;
    table[idx].ttl_secs = ttl;
    return idx;
}

void flare_mock_reset(void) {
    memset(&g_mock, 0, sizeof(g_mock));
    g_mock.resp_status = 200;
    flare_reset_arena();
}

void flare_mock_set_method(const char *method) {
    snprintf(g_mock.method, sizeof(g_mock.method), "%s", method);
}

void flare_mock_set_url(const char *url) {
    snprintf(g_mock.url, sizeof(g_mock.url), "%s", url);
}

void flare_mock_set_request_header(const char *name, const char *value) {
    put_header(g_mock.req_headers, name, value);
}

void flare_mock_set_request_body(const uint8_t *body, size_t len) {
    if (len > MOCK_VAL_BUF) len = MOCK_VAL_BUF;
    memcpy(g_mock.req_body, body, len);
    g_mock.req_body_len = len;
}

int flare_mock_resp_status(void) { return g_mock.resp_status; }

const char *flare_mock_resp_header(const char *name) {
    int idx = find_header(g_mock.resp_headers, name);
    return idx < 0 ? NULL : g_mock.resp_headers[idx].value;
}

const uint8_t *flare_mock_resp_body(size_t *out_len) {
    if (out_len) *out_len = g_mock.resp_body_len;
    return g_mock.resp_body;
}

void flare_mock_spark_seed(const char *key, const uint8_t *value, size_t len, uint32_t ttl) {
    put_kv(g_mock.spark, key, value, len, ttl);
}

int flare_mock_spark_has(const char *key) {
    return find_kv(g_mock.spark, key) >= 0 ? 1 : 0;
}

void flare_mock_spark_set_next_error(int32_t code) {
    g_mock.spark_next_error = code;
}

void flare_mock_plasma_seed(const char *key, const uint8_t *value, size_t len) {
    put_kv(g_mock.plasma, key, value, len, 0);
}

int flare_mock_plasma_has(const char *key) {
    return find_kv(g_mock.plasma, key) >= 0 ? 1 : 0;
}

int64_t flare_mock_plasma_counter(const char *key) {
    int idx = find_kv(g_mock.plasma, key);
    if (idx < 0) return 0;
    return g_mock.plasma_counters[idx];
}

void flare_mock_secret_seed(const char *key, const char *value) {
    for (int i = 0; i < MOCK_MAX_KV; i++) {
        if (g_mock.secrets[i].in_use && strcmp(g_mock.secrets[i].key, key) == 0) {
            snprintf(g_mock.secrets[i].value, sizeof(g_mock.secrets[i].value), "%s", value);
            return;
        }
    }
    for (int i = 0; i < MOCK_MAX_KV; i++) {
        if (!g_mock.secrets[i].in_use) {
            g_mock.secrets[i].in_use = 1;
            snprintf(g_mock.secrets[i].key, sizeof(g_mock.secrets[i].key), "%s", key);
            snprintf(g_mock.secrets[i].value, sizeof(g_mock.secrets[i].value), "%s", value);
            return;
        }
    }
}

#define DEFINE_MOCK_STR_SETTER(field, fn_name)                                   \
    void fn_name(const char *v) {                                                \
        snprintf(g_mock.field, sizeof(g_mock.field), "%s", v);                   \
    }

DEFINE_MOCK_STR_SETTER(random_hex,       flare_mock_set_random_bytes_hex)
DEFINE_MOCK_STR_SETTER(hash_hex,         flare_mock_set_hash_result)
DEFINE_MOCK_STR_SETTER(hmac_hex,         flare_mock_set_hmac_result)
DEFINE_MOCK_STR_SETTER(jwt,              flare_mock_set_jwt_result)
DEFINE_MOCK_STR_SETTER(aes_encrypt_b64,  flare_mock_set_aes_encrypt_result)
DEFINE_MOCK_STR_SETTER(uuid,             flare_mock_set_uuid_result)
DEFINE_MOCK_STR_SETTER(ulid,             flare_mock_set_ulid_result)
DEFINE_MOCK_STR_SETTER(nanoid,           flare_mock_set_nanoid_result)
DEFINE_MOCK_STR_SETTER(ksuid,            flare_mock_set_ksuid_result)
DEFINE_MOCK_STR_SETTER(snowflake,        flare_mock_set_snowflake_result)
DEFINE_MOCK_STR_SETTER(timestamp,        flare_mock_set_timestamp_result)
DEFINE_MOCK_STR_SETTER(beam_response,    flare_mock_set_beam_response)
DEFINE_MOCK_STR_SETTER(b64_encode,       flare_mock_set_b64_encode_result)
DEFINE_MOCK_STR_SETTER(hex_encode,       flare_mock_set_hex_encode_result)
DEFINE_MOCK_STR_SETTER(url_encode,       flare_mock_set_url_encode_result)
DEFINE_MOCK_STR_SETTER(url_decode,       flare_mock_set_url_decode_result)

void flare_mock_set_aes_decrypt_result(const uint8_t *plain, size_t len) {
    if (len > MOCK_VAL_BUF) len = MOCK_VAL_BUF;
    memcpy(g_mock.aes_decrypt, plain, len);
    g_mock.aes_decrypt_len = len;
}

void flare_mock_set_b64_decode_result(const uint8_t *bytes, size_t len) {
    if (len > MOCK_VAL_BUF) len = MOCK_VAL_BUF;
    memcpy(g_mock.b64_decode, bytes, len);
    g_mock.b64_decode_len = len;
}

void flare_mock_set_hex_decode_result(const uint8_t *bytes, size_t len) {
    if (len > MOCK_VAL_BUF) len = MOCK_VAL_BUF;
    memcpy(g_mock.hex_decode, bytes, len);
    g_mock.hex_decode_len = len;
}

const char *flare_mock_last_log(int level) {
    for (int i = g_mock.log_count - 1; i >= 0; i--) {
        if (g_mock.logs[i].level == level) {
            return g_mock.logs[i].msg;
        }
    }
    return NULL;
}

int flare_mock_log_count(int level) {
    int n = 0;
    for (int i = 0; i < g_mock.log_count; i++) {
        if (g_mock.logs[i].level == level) n++;
    }
    return n;
}

void flare_mock_set_ws_event_type(const char *type) {
    snprintf(g_mock.ws_event_type, sizeof(g_mock.ws_event_type), "%s", type);
}

void flare_mock_set_ws_event_data(const uint8_t *data, size_t len) {
    if (len > MOCK_VAL_BUF) len = MOCK_VAL_BUF;
    memcpy(g_mock.ws_event_data, data, len);
    g_mock.ws_event_data_len = len;
}

void flare_mock_set_ws_conn_id(const char *id) {
    snprintf(g_mock.ws_conn_id, sizeof(g_mock.ws_conn_id), "%s", id);
}

void flare_mock_set_ws_close_code(int code) {
    g_mock.ws_close_code = code;
}

const uint8_t *flare_mock_ws_last_send(size_t *out_len) {
    if (out_len) *out_len = g_mock.ws_last_send_len;
    return g_mock.ws_last_send;
}

int flare_mock_ws_send_count(void)       { return g_mock.ws_send_count; }
int flare_mock_ws_close_called(void)     { return g_mock.ws_close_called; }
int flare_mock_ws_close_code_seen(void)  { return g_mock.ws_close_code_seen; }

void flare_mock_ws_set_send_error(int code) { g_mock.ws_send_error = code; }

/* ===== Host function implementations the SDK links against in tests ===== */

static const void *guest_addr(int32_t ptr) {
    return flare_arena_addr(ptr);
}

int64_t req_method(void) {
    if (g_mock.method[0] == '\0') return 0;
    uint64_t packed = 0;
    pack_to_arena((const uint8_t *)g_mock.method, strlen(g_mock.method), &packed);
    return (int64_t)packed;
}

int64_t req_url(void) {
    if (g_mock.url[0] == '\0') return 0;
    uint64_t packed = 0;
    pack_to_arena((const uint8_t *)g_mock.url, strlen(g_mock.url), &packed);
    return (int64_t)packed;
}

int64_t req_header_get(int32_t name_ptr, int32_t name_len) {
    char name[256];
    if (name_len <= 0 || (size_t)name_len >= sizeof(name)) return 0;
    memcpy(name, guest_addr(name_ptr), (size_t)name_len);
    name[name_len] = '\0';
    int idx = find_header(g_mock.req_headers, name);
    if (idx < 0) return 0;
    uint64_t packed = 0;
    pack_to_arena((const uint8_t *)g_mock.req_headers[idx].value,
                  strlen(g_mock.req_headers[idx].value), &packed);
    return (int64_t)packed;
}

int64_t req_body(void) {
    if (g_mock.req_body_len == 0) return 0;
    uint64_t packed = 0;
    pack_to_arena(g_mock.req_body, g_mock.req_body_len, &packed);
    return (int64_t)packed;
}

void resp_set_status(int32_t status) {
    g_mock.resp_status = status;
}

void resp_header_set(int32_t name_ptr, int32_t name_len,
                     int32_t val_ptr, int32_t val_len) {
    char name[256], value[1024];
    if (name_len <= 0 || (size_t)name_len >= sizeof(name)) return;
    if (val_len < 0 || (size_t)val_len >= sizeof(value)) return;
    memcpy(name, guest_addr(name_ptr), (size_t)name_len);
    name[name_len] = '\0';
    memcpy(value, guest_addr(val_ptr), (size_t)val_len);
    value[val_len] = '\0';
    put_header(g_mock.resp_headers, name, value);
}

void resp_body_set(int32_t body_ptr, int32_t body_len) {
    if (body_len < 0) body_len = 0;
    if ((size_t)body_len > MOCK_VAL_BUF) body_len = MOCK_VAL_BUF;
    if (body_len > 0) {
        memcpy(g_mock.resp_body, guest_addr(body_ptr), (size_t)body_len);
    }
    g_mock.resp_body_len = (size_t)body_len;
}

int64_t beam_fetch(int32_t url_ptr, int32_t url_len,
                   int32_t opts_ptr, int32_t opts_len) {
    (void)url_ptr; (void)url_len; (void)opts_ptr; (void)opts_len;
    if (g_mock.beam_response[0] == '\0') return 0;
    uint64_t packed = 0;
    pack_to_arena((const uint8_t *)g_mock.beam_response,
                  strlen(g_mock.beam_response), &packed);
    return (int64_t)packed;
}

static void log_record(int level, int32_t msg_ptr, int32_t msg_len) {
    if (g_mock.log_count >= 64) return;
    if (msg_len < 0) msg_len = 0;
    size_t copy = (size_t)msg_len < sizeof(g_mock.logs[0].msg) - 1
                      ? (size_t)msg_len : sizeof(g_mock.logs[0].msg) - 1;
    memcpy(g_mock.logs[g_mock.log_count].msg, guest_addr(msg_ptr), copy);
    g_mock.logs[g_mock.log_count].msg[copy] = '\0';
    g_mock.logs[g_mock.log_count].level = level;
    g_mock.log_count++;
}

void log_info(int32_t msg_ptr, int32_t msg_len)  { log_record(0, msg_ptr, msg_len); }
void log_warn(int32_t msg_ptr, int32_t msg_len)  { log_record(1, msg_ptr, msg_len); }
void log_error(int32_t msg_ptr, int32_t msg_len) { log_record(2, msg_ptr, msg_len); }

#define DEFINE_HOST_STR_GETTER(host_fn, field)                              \
    int64_t host_fn(int32_t args_ptr, int32_t args_len) {                   \
        (void)args_ptr; (void)args_len;                                     \
        if (g_mock.field[0] == '\0') return 0;                              \
        uint64_t packed = 0;                                                \
        pack_to_arena((const uint8_t *)g_mock.field,                        \
                      strlen(g_mock.field), &packed);                       \
        return (int64_t)packed;                                             \
    }

DEFINE_HOST_STR_GETTER(crypto_hash,     hash_hex)
DEFINE_HOST_STR_GETTER(crypto_hmac,     hmac_hex)
DEFINE_HOST_STR_GETTER(crypto_sign_jwt, jwt)
DEFINE_HOST_STR_GETTER(crypto_encrypt_aes, aes_encrypt_b64)
DEFINE_HOST_STR_GETTER(id_uuid,         uuid)
DEFINE_HOST_STR_GETTER(timestamp,       timestamp)

int64_t crypto_decrypt_aes(int32_t args_ptr, int32_t args_len) {
    (void)args_ptr; (void)args_len;
    if (g_mock.aes_decrypt_len == 0) return 0;
    uint64_t packed = 0;
    pack_to_arena(g_mock.aes_decrypt, g_mock.aes_decrypt_len, &packed);
    return (int64_t)packed;
}

int64_t crypto_random_bytes(int32_t length) {
    (void)length;
    if (g_mock.random_hex[0] == '\0') return 0;
    uint64_t packed = 0;
    pack_to_arena((const uint8_t *)g_mock.random_hex,
                  strlen(g_mock.random_hex), &packed);
    return (int64_t)packed;
}

#define DEFINE_HOST_BYTES_TRANSFORM(host_fn, str_field, bin_field, bin_len)  \
    int64_t host_fn(int32_t data_ptr, int32_t data_len) {                    \
        (void)data_ptr; (void)data_len;                                      \
        uint64_t packed = 0;                                                 \
        if (g_mock.str_field[0] != '\0') {                                   \
            pack_to_arena((const uint8_t *)g_mock.str_field,                 \
                          strlen(g_mock.str_field), &packed);                \
        } else if (g_mock.bin_len > 0) {                                     \
            pack_to_arena(g_mock.bin_field, g_mock.bin_len, &packed);        \
        }                                                                    \
        return (int64_t)packed;                                              \
    }

DEFINE_HOST_BYTES_TRANSFORM(encoding_base64_encode, b64_encode, b64_decode, b64_decode_len)
DEFINE_HOST_BYTES_TRANSFORM(encoding_base64_decode, b64_encode, b64_decode, b64_decode_len)
DEFINE_HOST_BYTES_TRANSFORM(encoding_hex_encode,    hex_encode, hex_decode, hex_decode_len)
DEFINE_HOST_BYTES_TRANSFORM(encoding_hex_decode,    hex_encode, hex_decode, hex_decode_len)
DEFINE_HOST_BYTES_TRANSFORM(encoding_url_encode,    url_encode, hex_decode, hex_decode_len)
DEFINE_HOST_BYTES_TRANSFORM(encoding_url_decode,    url_decode, hex_decode, hex_decode_len)

int64_t id_ulid(void) {
    if (g_mock.ulid[0] == '\0') return 0;
    uint64_t packed = 0;
    pack_to_arena((const uint8_t *)g_mock.ulid, strlen(g_mock.ulid), &packed);
    return (int64_t)packed;
}

int64_t id_nanoid(int32_t length) {
    (void)length;
    if (g_mock.nanoid[0] == '\0') return 0;
    uint64_t packed = 0;
    pack_to_arena((const uint8_t *)g_mock.nanoid, strlen(g_mock.nanoid), &packed);
    return (int64_t)packed;
}

int64_t id_ksuid(void) {
    if (g_mock.ksuid[0] == '\0') return 0;
    uint64_t packed = 0;
    pack_to_arena((const uint8_t *)g_mock.ksuid, strlen(g_mock.ksuid), &packed);
    return (int64_t)packed;
}

int64_t id_snowflake(void) {
    if (g_mock.snowflake[0] == '\0') return 0;
    uint64_t packed = 0;
    pack_to_arena((const uint8_t *)g_mock.snowflake, strlen(g_mock.snowflake), &packed);
    return (int64_t)packed;
}

int64_t snowflake_id(void) {
    return id_snowflake();
}

int64_t spark_get(int32_t key_ptr, int32_t key_len) {
    char key[MOCK_KEY_BUF];
    if (key_len <= 0 || (size_t)key_len >= sizeof(key)) return 0;
    memcpy(key, guest_addr(key_ptr), (size_t)key_len);
    key[key_len] = '\0';
    int idx = find_kv(g_mock.spark, key);
    if (idx < 0) return 0;
    uint8_t buf[MOCK_VAL_BUF + 4];
    uint32_t ttl = g_mock.spark[idx].ttl_secs;
    buf[0] = (uint8_t)(ttl & 0xff);
    buf[1] = (uint8_t)((ttl >> 8) & 0xff);
    buf[2] = (uint8_t)((ttl >> 16) & 0xff);
    buf[3] = (uint8_t)((ttl >> 24) & 0xff);
    memcpy(buf + 4, g_mock.spark[idx].value, g_mock.spark[idx].value_len);
    uint64_t packed = 0;
    pack_to_arena(buf, g_mock.spark[idx].value_len + 4, &packed);
    return (int64_t)packed;
}

int32_t spark_set(int32_t key_ptr, int32_t key_len,
                  int32_t val_ptr, int32_t val_len, int32_t ttl_secs) {
    if (g_mock.spark_next_error != 0) {
        int32_t code = g_mock.spark_next_error;
        g_mock.spark_next_error = 0;
        return code;
    }
    char key[MOCK_KEY_BUF];
    if (key_len <= 0 || (size_t)key_len >= sizeof(key)) return 8; /* bad key */
    memcpy(key, guest_addr(key_ptr), (size_t)key_len);
    key[key_len] = '\0';
    if (val_len < 0) val_len = 0;
    put_kv(g_mock.spark, key, (const uint8_t *)guest_addr(val_ptr),
           (size_t)val_len, (uint32_t)ttl_secs);
    return 0;
}

void spark_delete(int32_t key_ptr, int32_t key_len) {
    char key[MOCK_KEY_BUF];
    if (key_len <= 0 || (size_t)key_len >= sizeof(key)) return;
    memcpy(key, guest_addr(key_ptr), (size_t)key_len);
    key[key_len] = '\0';
    int idx = find_kv(g_mock.spark, key);
    if (idx >= 0) {
        memset(&g_mock.spark[idx], 0, sizeof(g_mock.spark[idx]));
    }
}

int64_t spark_list(void) {
    char buf[MOCK_VAL_BUF];
    size_t off = 0;
    buf[off++] = '[';
    int first = 1;
    for (int i = 0; i < MOCK_MAX_KV; i++) {
        if (!g_mock.spark[i].in_use) continue;
        if (!first) { buf[off++] = ','; }
        first = 0;
        buf[off++] = '"';
        size_t klen = strlen(g_mock.spark[i].key);
        memcpy(buf + off, g_mock.spark[i].key, klen);
        off += klen;
        buf[off++] = '"';
    }
    buf[off++] = ']';
    uint64_t packed = 0;
    pack_to_arena((const uint8_t *)buf, off, &packed);
    return (int64_t)packed;
}

int32_t spark_pull(int32_t origin_ptr, int32_t origin_len,
                   int32_t keys_ptr, int32_t keys_len) {
    (void)origin_ptr; (void)origin_len; (void)keys_ptr; (void)keys_len;
    return 0;
}

int64_t plasma_get(int32_t key_ptr, int32_t key_len) {
    char key[MOCK_KEY_BUF];
    if (key_len <= 0 || (size_t)key_len >= sizeof(key)) return 0;
    memcpy(key, guest_addr(key_ptr), (size_t)key_len);
    key[key_len] = '\0';
    int idx = find_kv(g_mock.plasma, key);
    if (idx < 0) return 0;
    uint64_t packed = 0;
    pack_to_arena(g_mock.plasma[idx].value, g_mock.plasma[idx].value_len, &packed);
    return (int64_t)packed;
}

int32_t plasma_set(int32_t key_ptr, int32_t key_len,
                   int32_t val_ptr, int32_t val_len) {
    char key[MOCK_KEY_BUF];
    if (key_len <= 0 || (size_t)key_len >= sizeof(key)) return 4; /* bad key */
    memcpy(key, guest_addr(key_ptr), (size_t)key_len);
    key[key_len] = '\0';
    if (val_len < 0) val_len = 0;
    put_kv(g_mock.plasma, key, (const uint8_t *)guest_addr(val_ptr), (size_t)val_len, 0);
    return 0;
}

int32_t plasma_delete(int32_t key_ptr, int32_t key_len) {
    char key[MOCK_KEY_BUF];
    if (key_len <= 0 || (size_t)key_len >= sizeof(key)) return 4;
    memcpy(key, guest_addr(key_ptr), (size_t)key_len);
    key[key_len] = '\0';
    int idx = find_kv(g_mock.plasma, key);
    if (idx >= 0) {
        memset(&g_mock.plasma[idx], 0, sizeof(g_mock.plasma[idx]));
        g_mock.plasma_counters[idx] = 0;
        g_mock.plasma_counters_used[idx] = 0;
    }
    return 0;
}

static int64_t plasma_apply_delta(int32_t key_ptr, int32_t key_len, int64_t delta) {
    char key[MOCK_KEY_BUF];
    if (key_len <= 0 || (size_t)key_len >= sizeof(key)) return 0;
    memcpy(key, guest_addr(key_ptr), (size_t)key_len);
    key[key_len] = '\0';
    int idx = find_kv(g_mock.plasma, key);
    if (idx < 0) {
        idx = put_kv(g_mock.plasma, key, NULL, 0, 0);
        if (idx < 0) return 0;
    }
    g_mock.plasma_counters_used[idx] = 1;
    g_mock.plasma_counters[idx] += delta;
    int64_t newv = g_mock.plasma_counters[idx];
    uint8_t buf[8];
    for (int i = 0; i < 8; i++) {
        buf[i] = (uint8_t)((uint64_t)newv >> (i * 8));
    }
    uint64_t packed = 0;
    pack_to_arena(buf, 8, &packed);
    return (int64_t)packed;
}

int64_t plasma_increment(int32_t key_ptr, int32_t key_len, int64_t delta) {
    return plasma_apply_delta(key_ptr, key_len, delta);
}

int64_t plasma_decrement(int32_t key_ptr, int32_t key_len, int64_t delta) {
    return plasma_apply_delta(key_ptr, key_len, -delta);
}

int64_t plasma_list(void) {
    char buf[MOCK_VAL_BUF];
    size_t off = 0;
    buf[off++] = '[';
    int first = 1;
    for (int i = 0; i < MOCK_MAX_KV; i++) {
        if (!g_mock.plasma[i].in_use) continue;
        if (!first) { buf[off++] = ','; }
        first = 0;
        buf[off++] = '"';
        size_t klen = strlen(g_mock.plasma[i].key);
        memcpy(buf + off, g_mock.plasma[i].key, klen);
        off += klen;
        buf[off++] = '"';
    }
    buf[off++] = ']';
    uint64_t packed = 0;
    pack_to_arena((const uint8_t *)buf, off, &packed);
    return (int64_t)packed;
}

int64_t secret_get(int32_t key_ptr, int32_t key_len) {
    char key[MOCK_KEY_BUF];
    if (key_len <= 0 || (size_t)key_len >= sizeof(key)) return 0;
    memcpy(key, guest_addr(key_ptr), (size_t)key_len);
    key[key_len] = '\0';
    for (int i = 0; i < MOCK_MAX_KV; i++) {
        if (g_mock.secrets[i].in_use && strcmp(g_mock.secrets[i].key, key) == 0) {
            uint64_t packed = 0;
            pack_to_arena((const uint8_t *)g_mock.secrets[i].value,
                          strlen(g_mock.secrets[i].value), &packed);
            return (int64_t)packed;
        }
    }
    return 0;
}

int32_t ws_send(int32_t data_ptr, int32_t data_len) {
    if (g_mock.ws_send_error != 0) {
        return g_mock.ws_send_error;
    }
    if (data_len < 0) data_len = 0;
    if ((size_t)data_len > MOCK_VAL_BUF) data_len = MOCK_VAL_BUF;
    memcpy(g_mock.ws_last_send, guest_addr(data_ptr), (size_t)data_len);
    g_mock.ws_last_send_len = (size_t)data_len;
    g_mock.ws_send_count++;
    return 0;
}

void ws_close_conn(int32_t code) {
    g_mock.ws_close_called = 1;
    g_mock.ws_close_code_seen = code;
}

int64_t ws_conn_id(void) {
    if (g_mock.ws_conn_id[0] == '\0') return 0;
    uint64_t packed = 0;
    pack_to_arena((const uint8_t *)g_mock.ws_conn_id, strlen(g_mock.ws_conn_id), &packed);
    return (int64_t)packed;
}

int64_t ws_event_type(void) {
    if (g_mock.ws_event_type[0] == '\0') return 0;
    uint64_t packed = 0;
    pack_to_arena((const uint8_t *)g_mock.ws_event_type, strlen(g_mock.ws_event_type), &packed);
    return (int64_t)packed;
}

int64_t ws_event_data(void) {
    if (g_mock.ws_event_data_len == 0) return 0;
    uint64_t packed = 0;
    pack_to_arena(g_mock.ws_event_data, g_mock.ws_event_data_len, &packed);
    return (int64_t)packed;
}

int32_t ws_close_code(void) {
    return g_mock.ws_close_code;
}
