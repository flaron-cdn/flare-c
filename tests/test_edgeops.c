#include "test_runner.h"
#include "mock_host.h"

#include "flare/edgeops.h"
#include "flare/runtime.h"

#include <string.h>

FLARE_TEST(crypto_hash_returns_hex) {
    flare_mock_reset();
    flare_reset_arena();
    flare_mock_set_hash_result("e3b0c44298fc1c149afbf4c8996fb924");

    const char *out = NULL;
    size_t out_len = 0;
    flare_status_t st = flare_crypto_hash("sha256", 6, "", 0, &out, &out_len);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT_EQ_INT(out_len, 32);
    FLARE_ASSERT(memcmp(out, "e3b0c44298fc1c149afbf4c8996fb924", 32) == 0);
}

FLARE_TEST(crypto_hmac_returns_hex) {
    flare_mock_reset();
    flare_reset_arena();
    flare_mock_set_hmac_result("aabbccdd");
    const char *out = NULL;
    size_t out_len = 0;
    flare_status_t st = flare_crypto_hmac("hmac-key", 8, "msg", 3, &out, &out_len);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT_EQ_INT(out_len, 8);
}

FLARE_TEST(crypto_random_bytes_returns_hex) {
    flare_mock_reset();
    flare_reset_arena();
    flare_mock_set_random_bytes_hex("deadbeef");
    const char *out = NULL;
    size_t out_len = 0;
    flare_status_t st = flare_crypto_random_bytes(4, &out, &out_len);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT_EQ_INT(out_len, 8);
    FLARE_ASSERT(memcmp(out, "deadbeef", 8) == 0);
}

FLARE_TEST(b64_encode_round_trips) {
    flare_mock_reset();
    flare_reset_arena();
    flare_mock_set_b64_encode_result("aGVsbG8=");
    const char *out = NULL;
    size_t out_len = 0;
    flare_status_t st = flare_b64_encode((const uint8_t *)"hello", 5, &out, &out_len);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT_EQ_INT(out_len, 8);
    FLARE_ASSERT(memcmp(out, "aGVsbG8=", 8) == 0);
}

FLARE_TEST(b64_decode_returns_bytes) {
    flare_mock_reset();
    flare_reset_arena();
    const uint8_t expected[] = "hello";
    flare_mock_set_b64_decode_result(expected, 5);
    const uint8_t *out = NULL;
    size_t out_len = 0;
    flare_status_t st = flare_b64_decode("aGVsbG8=", 8, &out, &out_len);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT_EQ_INT(out_len, 5);
    FLARE_ASSERT_EQ_MEM(out, out_len, expected, 5);
}

FLARE_TEST(uuid_v4_returns_seeded) {
    flare_mock_reset();
    flare_reset_arena();
    flare_mock_set_uuid_result("00000000-0000-4000-8000-000000000000");
    const char *out = NULL;
    size_t out_len = 0;
    flare_status_t st = flare_id_uuid_v4(&out, &out_len);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT_EQ_INT(out_len, 36);
}

FLARE_TEST(timestamp_returns_seeded) {
    flare_mock_reset();
    flare_reset_arena();
    flare_mock_set_timestamp_result("2026-04-07T11:12:13Z");
    const char *out = NULL;
    size_t out_len = 0;
    flare_status_t st = flare_timestamp(FLARE_TIME_RFC3339, &out, &out_len);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT_EQ_INT(out_len, 20);
    FLARE_ASSERT(memcmp(out, "2026-04-07T11:12:13Z", 20) == 0);
}

FLARE_TEST(secret_get_returns_value) {
    flare_mock_reset();
    flare_reset_arena();
    flare_mock_secret_seed("api-key", "topsecret");
    const char *out = NULL;
    size_t out_len = 0;
    flare_status_t st = flare_secret_get("api-key", 7, &out, &out_len);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT_EQ_INT(out_len, 9);
    FLARE_ASSERT(memcmp(out, "topsecret", 9) == 0);
}

FLARE_TEST(secret_get_missing_returns_not_found) {
    flare_mock_reset();
    flare_reset_arena();
    const char *out = NULL;
    size_t out_len = 0;
    flare_status_t st = flare_secret_get("nope", 4, &out, &out_len);
    FLARE_ASSERT_EQ_INT(st, FLARE_ERR_NOT_FOUND);
}

void register_edgeops_tests(void) {
    FLARE_RUN(crypto_hash_returns_hex);
    FLARE_RUN(crypto_hmac_returns_hex);
    FLARE_RUN(crypto_random_bytes_returns_hex);
    FLARE_RUN(b64_encode_round_trips);
    FLARE_RUN(b64_decode_returns_bytes);
    FLARE_RUN(uuid_v4_returns_seeded);
    FLARE_RUN(timestamp_returns_seeded);
    FLARE_RUN(secret_get_returns_value);
    FLARE_RUN(secret_get_missing_returns_not_found);
}
