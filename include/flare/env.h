#ifndef FLARE_ENV_H
#define FLARE_ENV_H

/*
 * Private header — extern declarations for the flaron/v1 host module the
 * flaron edge runtime imports into every flare. Everything here is unsafe and
 * unchecked. Use the wrappers in <flare/spark.h>, <flare/plasma.h>, etc.
 *
 * The host writes return data into guest memory by calling the guest-exported
 * `flare_alloc` function. Each invocation must reset the bump arena before
 * the host hands control back to your handler — see <flare/runtime.h>.
 */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FLARE_HOST_IMPORT
#  if defined(__wasm__) || defined(__wasm32__)
#    define FLARE_HOST_IMPORT __attribute__((import_module("flaron/v1")))
#  else
#    define FLARE_HOST_IMPORT
#  endif
#endif

FLARE_HOST_IMPORT int64_t req_method(void);
FLARE_HOST_IMPORT int64_t req_url(void);
FLARE_HOST_IMPORT int64_t req_header_get(int32_t name_ptr, int32_t name_len);
FLARE_HOST_IMPORT int64_t req_body(void);

FLARE_HOST_IMPORT void resp_set_status(int32_t status);
FLARE_HOST_IMPORT void resp_header_set(int32_t name_ptr, int32_t name_len,
                                       int32_t val_ptr, int32_t val_len);
FLARE_HOST_IMPORT void resp_body_set(int32_t body_ptr, int32_t body_len);

FLARE_HOST_IMPORT int64_t beam_fetch(int32_t url_ptr, int32_t url_len,
                                     int32_t opts_ptr, int32_t opts_len);

FLARE_HOST_IMPORT void log_info(int32_t msg_ptr, int32_t msg_len);
FLARE_HOST_IMPORT void log_warn(int32_t msg_ptr, int32_t msg_len);
FLARE_HOST_IMPORT void log_error(int32_t msg_ptr, int32_t msg_len);

FLARE_HOST_IMPORT int64_t crypto_hash(int32_t args_ptr, int32_t args_len);
FLARE_HOST_IMPORT int64_t crypto_hmac(int32_t args_ptr, int32_t args_len);
FLARE_HOST_IMPORT int64_t crypto_sign_jwt(int32_t args_ptr, int32_t args_len);
FLARE_HOST_IMPORT int64_t crypto_encrypt_aes(int32_t args_ptr, int32_t args_len);
FLARE_HOST_IMPORT int64_t crypto_decrypt_aes(int32_t args_ptr, int32_t args_len);
FLARE_HOST_IMPORT int64_t crypto_random_bytes(int32_t length);

FLARE_HOST_IMPORT int64_t encoding_base64_encode(int32_t data_ptr, int32_t data_len);
FLARE_HOST_IMPORT int64_t encoding_base64_decode(int32_t data_ptr, int32_t data_len);
FLARE_HOST_IMPORT int64_t encoding_hex_encode(int32_t data_ptr, int32_t data_len);
FLARE_HOST_IMPORT int64_t encoding_hex_decode(int32_t data_ptr, int32_t data_len);
FLARE_HOST_IMPORT int64_t encoding_url_encode(int32_t data_ptr, int32_t data_len);
FLARE_HOST_IMPORT int64_t encoding_url_decode(int32_t data_ptr, int32_t data_len);

FLARE_HOST_IMPORT int64_t id_uuid(int32_t args_ptr, int32_t args_len);
FLARE_HOST_IMPORT int64_t id_ulid(void);
FLARE_HOST_IMPORT int64_t id_nanoid(int32_t length);
FLARE_HOST_IMPORT int64_t id_ksuid(void);
FLARE_HOST_IMPORT int64_t id_snowflake(void);
FLARE_HOST_IMPORT int64_t snowflake_id(void);

FLARE_HOST_IMPORT int64_t timestamp(int32_t args_ptr, int32_t args_len);

FLARE_HOST_IMPORT int64_t spark_get(int32_t key_ptr, int32_t key_len);
FLARE_HOST_IMPORT int32_t spark_set(int32_t key_ptr, int32_t key_len,
                                    int32_t val_ptr, int32_t val_len,
                                    int32_t ttl_secs);
FLARE_HOST_IMPORT void spark_delete(int32_t key_ptr, int32_t key_len);
FLARE_HOST_IMPORT int64_t spark_list(void);
FLARE_HOST_IMPORT int32_t spark_pull(int32_t origin_ptr, int32_t origin_len,
                                     int32_t keys_ptr, int32_t keys_len);

FLARE_HOST_IMPORT int64_t plasma_get(int32_t key_ptr, int32_t key_len);
FLARE_HOST_IMPORT int32_t plasma_set(int32_t key_ptr, int32_t key_len,
                                     int32_t val_ptr, int32_t val_len);
FLARE_HOST_IMPORT int32_t plasma_delete(int32_t key_ptr, int32_t key_len);
FLARE_HOST_IMPORT int64_t plasma_increment(int32_t key_ptr, int32_t key_len, int64_t delta);
FLARE_HOST_IMPORT int64_t plasma_decrement(int32_t key_ptr, int32_t key_len, int64_t delta);
FLARE_HOST_IMPORT int64_t plasma_list(void);

FLARE_HOST_IMPORT int64_t secret_get(int32_t key_ptr, int32_t key_len);

FLARE_HOST_IMPORT int32_t ws_send(int32_t data_ptr, int32_t data_len);
FLARE_HOST_IMPORT void ws_close_conn(int32_t code);
FLARE_HOST_IMPORT int64_t ws_conn_id(void);
FLARE_HOST_IMPORT int64_t ws_event_type(void);
FLARE_HOST_IMPORT int64_t ws_event_data(void);
FLARE_HOST_IMPORT int32_t ws_close_code(void);

#ifdef __cplusplus
}
#endif

#endif
