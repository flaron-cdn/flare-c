#ifndef FLARE_MOCK_HOST_H
#define FLARE_MOCK_HOST_H

/*
 * Test-only mock for the flaron/v1 host module.
 *
 * The real host functions live in the flaron Wasm runtime — they are not
 * available when tests are compiled as a native binary. This header (with the
 * paired mock_host.c) provides:
 *
 *   1. Definitions for every extern host symbol declared in <flare/env.h>.
 *      These call into the mock state instead of trapping.
 *   2. A small fixture API tests use to drive the mocks: stage request data,
 *      seed kv stores, drain captured response state, etc.
 *
 * The mock owns its own static buffers — call flare_mock_reset() between tests.
 *
 * The mock is *not* a faithful reimplementation of corona. It only models the
 * behaviour the C SDK relies on: ptr/len ABI, the spark wire format, error
 * codes, and per-key not-found semantics. Anything else is undefined.
 */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void flare_mock_reset(void);

void flare_mock_set_method(const char *method);
void flare_mock_set_url(const char *url);
void flare_mock_set_request_header(const char *name, const char *value);
void flare_mock_set_request_body(const uint8_t *body, size_t len);

int                flare_mock_resp_status(void);
const char        *flare_mock_resp_header(const char *name);
const uint8_t     *flare_mock_resp_body(size_t *out_len);

void flare_mock_spark_seed(const char *key, const uint8_t *value, size_t len, uint32_t ttl);
int  flare_mock_spark_has(const char *key);
void flare_mock_spark_set_next_error(int32_t code);
void flare_mock_spark_pull_set_next_return(int32_t code);

void flare_mock_plasma_seed(const char *key, const uint8_t *value, size_t len);
int  flare_mock_plasma_has(const char *key);
int64_t flare_mock_plasma_counter(const char *key);

void flare_mock_secret_seed(const char *key, const char *value);

void flare_mock_set_random_bytes_hex(const char *hex);
void flare_mock_set_hash_result(const char *hex);
void flare_mock_set_hmac_result(const char *hex);
void flare_mock_set_jwt_result(const char *jwt);
void flare_mock_set_aes_encrypt_result(const char *b64);
void flare_mock_set_aes_decrypt_result(const uint8_t *plain, size_t len);
void flare_mock_set_uuid_result(const char *uuid);
void flare_mock_set_ulid_result(const char *ulid);
void flare_mock_set_nanoid_result(const char *id);
void flare_mock_set_ksuid_result(const char *id);
void flare_mock_set_snowflake_result(const char *id);
void flare_mock_set_timestamp_result(const char *ts);
void flare_mock_set_beam_response(const char *json);
void flare_mock_set_b64_encode_result(const char *b64);
void flare_mock_set_b64_decode_result(const uint8_t *bytes, size_t len);
void flare_mock_set_hex_encode_result(const char *hex);
void flare_mock_set_hex_decode_result(const uint8_t *bytes, size_t len);
void flare_mock_set_url_encode_result(const char *enc);
void flare_mock_set_url_decode_result(const char *dec);

const char *flare_mock_last_log(int level);
int         flare_mock_log_count(int level);

void flare_mock_set_ws_event_type(const char *type);
void flare_mock_set_ws_event_data(const uint8_t *data, size_t len);
void flare_mock_set_ws_conn_id(const char *id);
void flare_mock_set_ws_close_code(int code);
const uint8_t *flare_mock_ws_last_send(size_t *out_len);
int            flare_mock_ws_send_count(void);
int            flare_mock_ws_close_called(void);
int            flare_mock_ws_close_code_seen(void);
void           flare_mock_ws_set_send_error(int code);

#ifdef __cplusplus
}
#endif

#endif
