#include "test_runner.h"
#include "mock_host.h"

#include "flare/spark.h"
#include "flare/runtime.h"

#include <string.h>

FLARE_TEST(spark_get_returns_value_with_ttl) {
    flare_mock_reset();
    flare_reset_arena();
    const uint8_t value[] = "session-token-xyz";
    flare_mock_spark_seed("user:42:session", value, 17, 600);

    flare_spark_entry_t entry = {0};
    flare_status_t st = flare_spark_get("user:42:session", 15, &entry);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT_EQ_INT(entry.value_len, 17);
    FLARE_ASSERT_EQ_INT(entry.ttl_secs, 600);
    FLARE_ASSERT_EQ_MEM(entry.value, entry.value_len, value, 17);
}

FLARE_TEST(spark_get_strips_ttl_prefix_correctly) {
    flare_mock_reset();
    flare_reset_arena();
    const uint8_t value[] = {0xde, 0xad, 0xbe, 0xef};
    flare_mock_spark_seed("k", value, 4, 0);

    flare_spark_entry_t entry = {0};
    flare_status_t st = flare_spark_get("k", 1, &entry);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT_EQ_INT(entry.value_len, 4);
    FLARE_ASSERT_EQ_INT(entry.ttl_secs, 0);
    FLARE_ASSERT_EQ_MEM(entry.value, entry.value_len, value, 4);
}

FLARE_TEST(spark_get_missing_returns_not_found) {
    flare_mock_reset();
    flare_reset_arena();
    flare_spark_entry_t entry = {0};
    flare_status_t st = flare_spark_get("missing", 7, &entry);
    FLARE_ASSERT_EQ_INT(st, FLARE_ERR_NOT_FOUND);
}

FLARE_TEST(spark_set_writes_value) {
    flare_mock_reset();
    flare_reset_arena();
    const uint8_t value[] = "v";
    flare_status_t st = flare_spark_set("k1", 2, value, 1, 30);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT(flare_mock_spark_has("k1"));
}

FLARE_TEST(spark_set_propagates_quota_error) {
    flare_mock_reset();
    flare_reset_arena();
    flare_mock_spark_set_next_error(4); /* sparkErrQuota */
    const uint8_t value[] = "v";
    flare_status_t st = flare_spark_set("k", 1, value, 1, 60);
    FLARE_ASSERT_EQ_INT(st, FLARE_ERR_SPARK_QUOTA);
}

FLARE_TEST(spark_set_propagates_no_capability_error) {
    flare_mock_reset();
    flare_reset_arena();
    flare_mock_spark_set_next_error(9);
    flare_status_t st = flare_spark_set("k", 1, (const uint8_t *)"v", 1, 0);
    FLARE_ASSERT_EQ_INT(st, FLARE_ERR_SPARK_NO_CAPAB);
}

FLARE_TEST(spark_delete_removes_entry) {
    flare_mock_reset();
    flare_reset_arena();
    flare_mock_spark_seed("kgone", (const uint8_t *)"v", 1, 0);
    FLARE_ASSERT(flare_mock_spark_has("kgone"));
    flare_spark_delete("kgone", 5);
    FLARE_ASSERT(!flare_mock_spark_has("kgone"));
}

FLARE_TEST(spark_list_returns_json_array) {
    flare_mock_reset();
    flare_reset_arena();
    flare_mock_spark_seed("a", (const uint8_t *)"1", 1, 0);
    flare_mock_spark_seed("b", (const uint8_t *)"2", 1, 0);

    const uint8_t *json = NULL;
    size_t len = 0;
    flare_status_t st = flare_spark_list(&json, &len);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT(len > 0);
    FLARE_ASSERT(json[0] == '[');
    FLARE_ASSERT(json[len - 1] == ']');
}

FLARE_TEST(spark_pull_positive_return_is_count) {
    flare_mock_reset();
    flare_reset_arena();
    flare_mock_spark_pull_set_next_return(5);

    uint32_t count = 99;
    const char *keys = "[\"k1\",\"k2\"]";
    flare_status_t st = flare_spark_pull("node-2", 6, keys, 11, &count);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT_EQ_INT(count, 5);
}

FLARE_TEST(spark_pull_zero_is_ok_with_zero_count) {
    flare_mock_reset();
    flare_reset_arena();
    flare_mock_spark_pull_set_next_return(0);

    uint32_t count = 99;
    flare_status_t st = flare_spark_pull("node-2", 6, "[\"k\"]", 5, &count);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT_EQ_INT(count, 0);
}

FLARE_TEST(spark_pull_negative_three_is_write_limit) {
    flare_mock_reset();
    flare_reset_arena();
    flare_mock_spark_pull_set_next_return(-3);

    uint32_t count = 99;
    flare_status_t st = flare_spark_pull("node-2", 6, "[\"k\"]", 5, &count);
    FLARE_ASSERT_EQ_INT(st, FLARE_ERR_SPARK_WRITE_LIMIT);
    FLARE_ASSERT_EQ_INT(count, 0);
}

FLARE_TEST(spark_pull_negative_eight_is_bad_key) {
    flare_mock_reset();
    flare_reset_arena();
    flare_mock_spark_pull_set_next_return(-8);

    uint32_t count = 99;
    flare_status_t st = flare_spark_pull("node-2", 6, "[\"k\"]", 5, &count);
    FLARE_ASSERT_EQ_INT(st, FLARE_ERR_SPARK_BAD_KEY);
}

FLARE_TEST(spark_pull_negative_nine_is_no_capability) {
    flare_mock_reset();
    flare_reset_arena();
    flare_mock_spark_pull_set_next_return(-9);

    uint32_t count = 99;
    flare_status_t st = flare_spark_pull("node-2", 6, "[\"k\"]", 5, &count);
    FLARE_ASSERT_EQ_INT(st, FLARE_ERR_SPARK_NO_CAPAB);
}

FLARE_TEST(spark_pull_unknown_negative_is_host_error) {
    flare_mock_reset();
    flare_reset_arena();
    flare_mock_spark_pull_set_next_return(-42);

    uint32_t count = 99;
    flare_status_t st = flare_spark_pull("node-2", 6, "[\"k\"]", 5, &count);
    FLARE_ASSERT_EQ_INT(st, FLARE_ERR_HOST);
}

void register_spark_tests(void) {
    FLARE_RUN(spark_get_returns_value_with_ttl);
    FLARE_RUN(spark_get_strips_ttl_prefix_correctly);
    FLARE_RUN(spark_get_missing_returns_not_found);
    FLARE_RUN(spark_set_writes_value);
    FLARE_RUN(spark_set_propagates_quota_error);
    FLARE_RUN(spark_set_propagates_no_capability_error);
    FLARE_RUN(spark_delete_removes_entry);
    FLARE_RUN(spark_list_returns_json_array);
    FLARE_RUN(spark_pull_positive_return_is_count);
    FLARE_RUN(spark_pull_zero_is_ok_with_zero_count);
    FLARE_RUN(spark_pull_negative_three_is_write_limit);
    FLARE_RUN(spark_pull_negative_eight_is_bad_key);
    FLARE_RUN(spark_pull_negative_nine_is_no_capability);
    FLARE_RUN(spark_pull_unknown_negative_is_host_error);
}
