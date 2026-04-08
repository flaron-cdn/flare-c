#include "flare/edgeops.h"
#include "flare/runtime.h"
#include "flare/env.h"
#include "flare_mem.h"

#include <stdint.h>

/*
 * The host expects JSON envelopes for crypto, id, and timestamp operations.
 * C has no stdlib JSON encoder, so we hand-write a tiny string-escaping
 * builder that writes directly into the bump arena. The shapes we need are
 * fixed (small flat objects, sometimes a one-level nested map for JWT
 * claims) so a full JSON library would be overkill.
 */

typedef struct {
    char  *buf;
    size_t cap;
    size_t len;
    int    overflow;
} json_writer_t;

static void jw_putc(json_writer_t *w, char c) {
    if (w->len + 1 > w->cap) { w->overflow = 1; return; }
    w->buf[w->len++] = c;
}

static void jw_puts(json_writer_t *w, const char *s, size_t n) {
    if (w->len + n > w->cap) { w->overflow = 1; return; }
    flare_memcpy(w->buf + w->len, s, n);
    w->len += n;
}

static void jw_hex_nibble(json_writer_t *w, uint8_t v) {
    static const char hex[] = "0123456789abcdef";
    jw_putc(w, hex[v & 0xf]);
}

static void jw_quote(json_writer_t *w, const char *s, size_t n) {
    jw_putc(w, '"');
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)s[i];
        switch (c) {
        case '"':  jw_puts(w, "\\\"", 2); break;
        case '\\': jw_puts(w, "\\\\", 2); break;
        case '\b': jw_puts(w, "\\b", 2); break;
        case '\f': jw_puts(w, "\\f", 2); break;
        case '\n': jw_puts(w, "\\n", 2); break;
        case '\r': jw_puts(w, "\\r", 2); break;
        case '\t': jw_puts(w, "\\t", 2); break;
        default:
            if (c < 0x20) {
                jw_puts(w, "\\u00", 4);
                jw_hex_nibble(w, (uint8_t)(c >> 4));
                jw_hex_nibble(w, (uint8_t)(c & 0xf));
            } else {
                jw_putc(w, (char)c);
            }
        }
    }
    jw_putc(w, '"');
}

static void jw_field(json_writer_t *w, int *first,
                     const char *key, size_t key_len,
                     const char *val, size_t val_len) {
    if (!*first) jw_putc(w, ',');
    *first = 0;
    jw_quote(w, key, key_len);
    jw_putc(w, ':');
    jw_quote(w, val, val_len);
}

/*
 * Allocate a temporary JSON envelope inside the bump arena and return its
 * handle + length. Returns 0 handle on overflow or arena exhaustion.
 */
static int32_t build_envelope(size_t cap, void (*build)(json_writer_t *, void *),
                              void *ctx, int32_t *out_len) {
    int32_t handle = flare_alloc((int32_t)cap);
    if (handle == 0) return 0;
    json_writer_t w;
    w.buf = (char *)flare_arena_addr(handle);
    w.cap = cap;
    w.len = 0;
    w.overflow = 0;
    build(&w, ctx);
    if (w.overflow) return 0;
    if (out_len) *out_len = (int32_t)w.len;
    return handle;
}

/* ---------- generic packed-string reader ---------- */

static flare_status_t read_str(int64_t packed, const char **out_str, size_t *out_len) {
    if (out_str) *out_str = NULL;
    if (out_len) *out_len = 0;
    if (packed == 0) return FLARE_ERR_HOST;
    int32_t handle = 0, len = 0;
    flare_decode_ptr_len(packed, &handle, &len);
    if (handle == 0 || len < 0) return FLARE_ERR_HOST;
    if (out_str) *out_str = (const char *)flare_arena_addr(handle);
    if (out_len) *out_len = (size_t)len;
    return FLARE_OK;
}

static flare_status_t read_bytes(int64_t packed, const uint8_t **out, size_t *out_len) {
    return read_str(packed, (const char **)out, out_len);
}

/* ---------- crypto ---------- */

typedef struct {
    const char *algorithm;
    size_t      algorithm_len;
    const char *secret_key;
    size_t      secret_key_len;
    const char *input;
    size_t      input_len;
} crypto_args_t;

static void build_crypto_args(json_writer_t *w, void *ctx) {
    crypto_args_t *a = (crypto_args_t *)ctx;
    int first = 1;
    jw_putc(w, '{');
    if (a->algorithm) {
        jw_field(w, &first, "algorithm", 9, a->algorithm, a->algorithm_len);
    }
    if (a->secret_key) {
        jw_field(w, &first, "secret_key", 10, a->secret_key, a->secret_key_len);
    }
    if (a->input) {
        jw_field(w, &first, "input", 5, a->input, a->input_len);
    }
    jw_putc(w, '}');
}

static size_t crypto_envelope_cap(const crypto_args_t *a) {
    /* Worst case: every byte of every value escapes to 6 chars (\uXXXX). */
    size_t cap = 64;
    cap += (a->algorithm_len + a->secret_key_len + a->input_len) * 6;
    return cap;
}

flare_status_t flare_crypto_hash(const char *algorithm, size_t algorithm_len,
                                 const char *input, size_t input_len,
                                 const char **out_hex, size_t *out_len) {
    if (out_hex) *out_hex = NULL;
    if (out_len) *out_len = 0;
    if (algorithm == NULL || algorithm_len == 0) return FLARE_ERR_INVALID_ARG;

    crypto_args_t args = {0};
    args.algorithm = algorithm; args.algorithm_len = algorithm_len;
    args.input = input ? input : "";
    args.input_len = input ? input_len : 0;

    int32_t env_len = 0;
    int32_t env_h = build_envelope(crypto_envelope_cap(&args),
                                   build_crypto_args, &args, &env_len);
    if (env_h == 0) return FLARE_ERR_OUT_OF_MEMORY;

    return read_str(crypto_hash(env_h, env_len), out_hex, out_len);
}

flare_status_t flare_crypto_hmac(const char *secret_key, size_t secret_key_len,
                                 const char *input, size_t input_len,
                                 const char **out_hex, size_t *out_len) {
    if (out_hex) *out_hex = NULL;
    if (out_len) *out_len = 0;
    if (secret_key == NULL || secret_key_len == 0) return FLARE_ERR_INVALID_ARG;

    crypto_args_t args = {0};
    args.secret_key = secret_key; args.secret_key_len = secret_key_len;
    args.input = input ? input : "";
    args.input_len = input ? input_len : 0;

    int32_t env_len = 0;
    int32_t env_h = build_envelope(crypto_envelope_cap(&args),
                                   build_crypto_args, &args, &env_len);
    if (env_h == 0) return FLARE_ERR_OUT_OF_MEMORY;

    return read_str(crypto_hmac(env_h, env_len), out_hex, out_len);
}

flare_status_t flare_crypto_random_bytes(uint32_t length,
                                         const char **out_hex, size_t *out_len) {
    return read_str(crypto_random_bytes((int32_t)length), out_hex, out_len);
}

flare_status_t flare_crypto_encrypt_aes(const char *secret_key, size_t secret_key_len,
                                        const char *plaintext, size_t plaintext_len,
                                        const char **out_b64, size_t *out_len) {
    if (out_b64) *out_b64 = NULL;
    if (out_len) *out_len = 0;
    if (secret_key == NULL || secret_key_len == 0) return FLARE_ERR_INVALID_ARG;

    crypto_args_t args = {0};
    args.secret_key = secret_key; args.secret_key_len = secret_key_len;
    args.input = plaintext ? plaintext : "";
    args.input_len = plaintext ? plaintext_len : 0;

    int32_t env_len = 0;
    int32_t env_h = build_envelope(crypto_envelope_cap(&args),
                                   build_crypto_args, &args, &env_len);
    if (env_h == 0) return FLARE_ERR_OUT_OF_MEMORY;

    return read_str(crypto_encrypt_aes(env_h, env_len), out_b64, out_len);
}

flare_status_t flare_crypto_decrypt_aes(const char *secret_key, size_t secret_key_len,
                                        const char *b64_ciphertext, size_t b64_len,
                                        const uint8_t **out_plain, size_t *out_len) {
    if (out_plain) *out_plain = NULL;
    if (out_len)   *out_len = 0;
    if (secret_key == NULL || secret_key_len == 0) return FLARE_ERR_INVALID_ARG;

    crypto_args_t args = {0};
    args.secret_key = secret_key; args.secret_key_len = secret_key_len;
    args.input = b64_ciphertext ? b64_ciphertext : "";
    args.input_len = b64_ciphertext ? b64_len : 0;

    int32_t env_len = 0;
    int32_t env_h = build_envelope(crypto_envelope_cap(&args),
                                   build_crypto_args, &args, &env_len);
    if (env_h == 0) return FLARE_ERR_OUT_OF_MEMORY;

    return read_bytes(crypto_decrypt_aes(env_h, env_len), out_plain, out_len);
}

/* ---------- JWT ---------- */

typedef struct {
    const char              *algorithm;
    size_t                   algorithm_len;
    const char              *secret_key;
    size_t                   secret_key_len;
    const flare_jwt_claim_t *claims;
    size_t                   claim_count;
} jwt_args_t;

static void build_jwt_args(json_writer_t *w, void *ctx) {
    jwt_args_t *a = (jwt_args_t *)ctx;
    int first = 1;
    jw_putc(w, '{');
    jw_field(w, &first, "algorithm", 9, a->algorithm, a->algorithm_len);
    jw_field(w, &first, "secret_key", 10, a->secret_key, a->secret_key_len);

    if (!first) jw_putc(w, ',');
    jw_quote(w, "claims", 6);
    jw_putc(w, ':');
    jw_putc(w, '{');
    int cf = 1;
    for (size_t i = 0; i < a->claim_count; i++) {
        jw_field(w, &cf,
                 a->claims[i].key, a->claims[i].key_len,
                 a->claims[i].value, a->claims[i].value_len);
    }
    jw_putc(w, '}');
    jw_putc(w, '}');
}

flare_status_t flare_crypto_sign_jwt(const char *algorithm, size_t algorithm_len,
                                     const char *secret_key, size_t secret_key_len,
                                     const flare_jwt_claim_t *claims, size_t claim_count,
                                     const char **out_jwt, size_t *out_len) {
    if (out_jwt) *out_jwt = NULL;
    if (out_len) *out_len = 0;
    if (algorithm == NULL || secret_key == NULL) return FLARE_ERR_INVALID_ARG;

    jwt_args_t args = {algorithm, algorithm_len, secret_key, secret_key_len,
                       claims, claim_count};

    size_t cap = 128 + (algorithm_len + secret_key_len) * 6;
    for (size_t i = 0; i < claim_count; i++) {
        cap += (claims[i].key_len + claims[i].value_len) * 6 + 8;
    }

    int32_t env_len = 0;
    int32_t env_h = build_envelope(cap, build_jwt_args, &args, &env_len);
    if (env_h == 0) return FLARE_ERR_OUT_OF_MEMORY;

    return read_str(crypto_sign_jwt(env_h, env_len), out_jwt, out_len);
}

/* ---------- encoding (raw bytes in / packed bytes out) ---------- */

typedef int64_t (*encoding_fn_t)(int32_t, int32_t);

static flare_status_t encode_bytes(encoding_fn_t fn,
                                   const uint8_t *data, size_t len,
                                   const uint8_t **out, size_t *out_len) {
    if (out)     *out = NULL;
    if (out_len) *out_len = 0;
    int32_t in_h = 0;
    if (data != NULL && len > 0) {
        in_h = flare_copy_to_arena(data, len);
        if (in_h == 0) return FLARE_ERR_OUT_OF_MEMORY;
    }
    return read_bytes(fn(in_h, (int32_t)len), out, out_len);
}

flare_status_t flare_b64_encode(const uint8_t *data, size_t len,
                                const char **out_b64, size_t *out_len) {
    return encode_bytes(encoding_base64_encode, data, len,
                        (const uint8_t **)out_b64, out_len);
}

flare_status_t flare_b64_decode(const char *b64, size_t b64_len,
                                const uint8_t **out_bytes, size_t *out_len) {
    return encode_bytes(encoding_base64_decode, (const uint8_t *)b64, b64_len,
                        out_bytes, out_len);
}

flare_status_t flare_hex_encode(const uint8_t *data, size_t len,
                                const char **out_hex, size_t *out_len) {
    return encode_bytes(encoding_hex_encode, data, len,
                        (const uint8_t **)out_hex, out_len);
}

flare_status_t flare_hex_decode(const char *hex, size_t hex_len,
                                const uint8_t **out_bytes, size_t *out_len) {
    return encode_bytes(encoding_hex_decode, (const uint8_t *)hex, hex_len,
                        out_bytes, out_len);
}

flare_status_t flare_url_encode(const char *data, size_t len,
                                const char **out_enc, size_t *out_len) {
    return encode_bytes(encoding_url_encode, (const uint8_t *)data, len,
                        (const uint8_t **)out_enc, out_len);
}

flare_status_t flare_url_decode(const char *data, size_t len,
                                const char **out_dec, size_t *out_len) {
    return encode_bytes(encoding_url_decode, (const uint8_t *)data, len,
                        (const uint8_t **)out_dec, out_len);
}

/* ---------- ID generators ---------- */

typedef struct {
    const char *version;
    size_t      version_len;
} uuid_args_t;

static void build_uuid_args(json_writer_t *w, void *ctx) {
    uuid_args_t *a = (uuid_args_t *)ctx;
    jw_putc(w, '{');
    jw_quote(w, "version", 7);
    jw_putc(w, ':');
    jw_quote(w, a->version, a->version_len);
    jw_putc(w, '}');
}

static flare_status_t uuid_with_version(const char *v, size_t vlen,
                                        const char **out, size_t *out_len) {
    uuid_args_t args = {v, vlen};
    int32_t env_len = 0;
    int32_t env_h = build_envelope(64, build_uuid_args, &args, &env_len);
    if (env_h == 0) return FLARE_ERR_OUT_OF_MEMORY;
    return read_str(id_uuid(env_h, env_len), out, out_len);
}

flare_status_t flare_id_uuid_v4(const char **out, size_t *out_len) {
    return uuid_with_version("v4", 2, out, out_len);
}

flare_status_t flare_id_uuid_v7(const char **out, size_t *out_len) {
    return uuid_with_version("v7", 2, out, out_len);
}

flare_status_t flare_id_ulid(const char **out, size_t *out_len) {
    return read_str(id_ulid(), out, out_len);
}

flare_status_t flare_id_nanoid(uint32_t length, const char **out, size_t *out_len) {
    return read_str(id_nanoid((int32_t)length), out, out_len);
}

flare_status_t flare_id_ksuid(const char **out, size_t *out_len) {
    return read_str(id_ksuid(), out, out_len);
}

flare_status_t flare_id_snowflake(const char **out, size_t *out_len) {
    return read_str(id_snowflake(), out, out_len);
}

/* ---------- timestamp ---------- */

typedef struct {
    const char *fmt;
    size_t      fmt_len;
} ts_args_t;

static void build_ts_args(json_writer_t *w, void *ctx) {
    ts_args_t *a = (ts_args_t *)ctx;
    jw_putc(w, '{');
    jw_quote(w, "format", 6);
    jw_putc(w, ':');
    jw_quote(w, a->fmt, a->fmt_len);
    jw_putc(w, '}');
}

static const char *time_fmt_str(flare_time_format_t fmt, size_t *out_len) {
    switch (fmt) {
    case FLARE_TIME_UNIX:    *out_len = 4; return "unix";
    case FLARE_TIME_MS:      *out_len = 2; return "ms";
    case FLARE_TIME_NS:      *out_len = 2; return "ns";
    case FLARE_TIME_RFC3339: *out_len = 7; return "rfc3339";
    case FLARE_TIME_HTTP:    *out_len = 4; return "http";
    case FLARE_TIME_ISO8601: *out_len = 7; return "iso8601";
    }
    *out_len = 7;
    return "rfc3339";
}

flare_status_t flare_timestamp(flare_time_format_t fmt,
                               const char **out_str, size_t *out_len) {
    ts_args_t args;
    args.fmt = time_fmt_str(fmt, &args.fmt_len);
    int32_t env_len = 0;
    int32_t env_h = build_envelope(64, build_ts_args, &args, &env_len);
    if (env_h == 0) return FLARE_ERR_OUT_OF_MEMORY;
    return read_str(timestamp(env_h, env_len), out_str, out_len);
}

/* ---------- secrets ---------- */

flare_status_t flare_secret_get(const char *key, size_t key_len,
                                const char **out_value, size_t *out_len) {
    if (out_value) *out_value = NULL;
    if (out_len)   *out_len = 0;
    if (key == NULL || key_len == 0) return FLARE_ERR_INVALID_ARG;

    int32_t key_h = flare_copy_to_arena(key, key_len);
    if (key_h == 0) return FLARE_ERR_OUT_OF_MEMORY;

    int64_t packed = secret_get(key_h, (int32_t)key_len);
    if (packed == 0) return FLARE_ERR_NOT_FOUND;

    int32_t handle = 0, len = 0;
    flare_decode_ptr_len(packed, &handle, &len);
    if (handle == 0 || len < 0) return FLARE_ERR_HOST;
    if (out_value) *out_value = (const char *)flare_arena_addr(handle);
    if (out_len)   *out_len = (size_t)len;
    return FLARE_OK;
}
