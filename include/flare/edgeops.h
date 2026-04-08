#ifndef FLARE_EDGEOPS_H
#define FLARE_EDGEOPS_H

#include <stdint.h>
#include <stddef.h>

#include "flare/status.h"

#ifdef __cplusplus
extern "C" {
#endif

flare_status_t flare_crypto_hash(const char *algorithm, size_t algorithm_len,
                                 const char *input, size_t input_len,
                                 const char **out_hex, size_t *out_len);

flare_status_t flare_crypto_hmac(const char *secret_key, size_t secret_key_len,
                                 const char *input, size_t input_len,
                                 const char **out_hex, size_t *out_len);

flare_status_t flare_crypto_random_bytes(uint32_t length,
                                         const char **out_hex, size_t *out_len);

flare_status_t flare_crypto_encrypt_aes(const char *secret_key, size_t secret_key_len,
                                        const char *plaintext, size_t plaintext_len,
                                        const char **out_b64, size_t *out_len);

flare_status_t flare_crypto_decrypt_aes(const char *secret_key, size_t secret_key_len,
                                        const char *b64_ciphertext, size_t b64_len,
                                        const uint8_t **out_plain, size_t *out_len);

typedef struct {
    const char *key;
    size_t      key_len;
    const char *value;
    size_t      value_len;
} flare_jwt_claim_t;

flare_status_t flare_crypto_sign_jwt(const char *algorithm, size_t algorithm_len,
                                     const char *secret_key, size_t secret_key_len,
                                     const flare_jwt_claim_t *claims, size_t claim_count,
                                     const char **out_jwt, size_t *out_len);

flare_status_t flare_b64_encode(const uint8_t *data, size_t len,
                                const char **out_b64, size_t *out_len);

flare_status_t flare_b64_decode(const char *b64, size_t b64_len,
                                const uint8_t **out_bytes, size_t *out_len);

flare_status_t flare_hex_encode(const uint8_t *data, size_t len,
                                const char **out_hex, size_t *out_len);

flare_status_t flare_hex_decode(const char *hex, size_t hex_len,
                                const uint8_t **out_bytes, size_t *out_len);

flare_status_t flare_url_encode(const char *data, size_t len,
                                const char **out_enc, size_t *out_len);

flare_status_t flare_url_decode(const char *data, size_t len,
                                const char **out_dec, size_t *out_len);

flare_status_t flare_id_uuid_v4(const char **out_uuid, size_t *out_len);
flare_status_t flare_id_uuid_v7(const char **out_uuid, size_t *out_len);
flare_status_t flare_id_ulid(const char **out_ulid, size_t *out_len);
flare_status_t flare_id_nanoid(uint32_t length, const char **out_id, size_t *out_len);
flare_status_t flare_id_ksuid(const char **out_id, size_t *out_len);
flare_status_t flare_id_snowflake(const char **out_id, size_t *out_len);

typedef enum {
    FLARE_TIME_UNIX    = 0,
    FLARE_TIME_MS      = 1,
    FLARE_TIME_NS      = 2,
    FLARE_TIME_RFC3339 = 3,
    FLARE_TIME_HTTP    = 4,
    FLARE_TIME_ISO8601 = 5,
} flare_time_format_t;

flare_status_t flare_timestamp(flare_time_format_t fmt,
                               const char **out_str, size_t *out_len);

flare_status_t flare_secret_get(const char *key, size_t key_len,
                                const char **out_value, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif
