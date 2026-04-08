#include "test_runner.h"
#include "mock_host.h"

#include "flare/plasma.h"
#include "flare/runtime.h"

#include <string.h>

FLARE_TEST(plasma_get_returns_value) {
    flare_mock_reset();
    flare_reset_arena();
    flare_mock_plasma_seed("user:1", (const uint8_t *)"online", 6);

    const uint8_t *out = NULL;
    size_t out_len = 0;
    flare_status_t st = flare_plasma_get("user:1", 6, &out, &out_len);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT_EQ_INT(out_len, 6);
    FLARE_ASSERT(memcmp(out, "online", 6) == 0);
}

FLARE_TEST(plasma_get_missing) {
    flare_mock_reset();
    flare_reset_arena();
    const uint8_t *out = NULL;
    size_t out_len = 0;
    flare_status_t st = flare_plasma_get("nope", 4, &out, &out_len);
    FLARE_ASSERT_EQ_INT(st, FLARE_ERR_NOT_FOUND);
}

FLARE_TEST(plasma_set_writes) {
    flare_mock_reset();
    flare_reset_arena();
    flare_status_t st = flare_plasma_set("k", 1, (const uint8_t *)"v", 1);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT(flare_mock_plasma_has("k"));
}

FLARE_TEST(plasma_delete_removes) {
    flare_mock_reset();
    flare_reset_arena();
    flare_mock_plasma_seed("k", (const uint8_t *)"v", 1);
    flare_status_t st = flare_plasma_delete("k", 1);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT(!flare_mock_plasma_has("k"));
}

FLARE_TEST(plasma_increment_returns_new_value) {
    flare_mock_reset();
    flare_reset_arena();
    int64_t v = 0;
    flare_status_t st = flare_plasma_increment("counter", 7, 5, &v);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT_EQ_INT(v, 5);
    st = flare_plasma_increment("counter", 7, 3, &v);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT_EQ_INT(v, 8);
}

FLARE_TEST(plasma_decrement_returns_new_value) {
    flare_mock_reset();
    flare_reset_arena();
    int64_t v = 0;
    flare_status_t st = flare_plasma_increment("c", 1, 10, &v);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT_EQ_INT(v, 10);
    st = flare_plasma_decrement("c", 1, 4, &v);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT_EQ_INT(v, 6);
}

FLARE_TEST(plasma_list_returns_json) {
    flare_mock_reset();
    flare_reset_arena();
    flare_mock_plasma_seed("a", (const uint8_t *)"1", 1);
    flare_mock_plasma_seed("b", (const uint8_t *)"2", 1);

    const uint8_t *json = NULL;
    size_t len = 0;
    flare_status_t st = flare_plasma_list(&json, &len);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT(json[0] == '[');
    FLARE_ASSERT(json[len - 1] == ']');
}

void register_plasma_tests(void) {
    FLARE_RUN(plasma_get_returns_value);
    FLARE_RUN(plasma_get_missing);
    FLARE_RUN(plasma_set_writes);
    FLARE_RUN(plasma_delete_removes);
    FLARE_RUN(plasma_increment_returns_new_value);
    FLARE_RUN(plasma_decrement_returns_new_value);
    FLARE_RUN(plasma_list_returns_json);
}
