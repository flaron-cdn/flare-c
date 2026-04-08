#include "test_runner.h"
#include "mock_host.h"

#include "flare/response.h"

#include <string.h>

FLARE_TEST(resp_set_status_records) {
    flare_mock_reset();
    flare_resp_set_status(404);
    FLARE_ASSERT_EQ_INT(flare_mock_resp_status(), 404);
}

FLARE_TEST(resp_set_status_default_is_200) {
    flare_mock_reset();
    FLARE_ASSERT_EQ_INT(flare_mock_resp_status(), 200);
}

FLARE_TEST(resp_set_header_round_trips) {
    flare_mock_reset();
    flare_resp_set_header("Content-Type", 12, "application/json", 16);
    const char *got = flare_mock_resp_header("Content-Type");
    FLARE_ASSERT(got != NULL);
    FLARE_ASSERT_EQ_STR(got, "application/json");
}

FLARE_TEST(resp_set_body_records_bytes) {
    flare_mock_reset();
    const uint8_t body[] = "hello world";
    flare_resp_set_body(body, 11);

    size_t out_len = 0;
    const uint8_t *got = flare_mock_resp_body(&out_len);
    FLARE_ASSERT_EQ_INT(out_len, 11);
    FLARE_ASSERT_EQ_MEM(got, out_len, body, 11);
}

FLARE_TEST(resp_set_body_str_records) {
    flare_mock_reset();
    flare_resp_set_body_str("ok", 2);
    size_t out_len = 0;
    const uint8_t *got = flare_mock_resp_body(&out_len);
    FLARE_ASSERT_EQ_INT(out_len, 2);
    FLARE_ASSERT(memcmp(got, "ok", 2) == 0);
}

void register_response_tests(void) {
    FLARE_RUN(resp_set_status_records);
    FLARE_RUN(resp_set_status_default_is_200);
    FLARE_RUN(resp_set_header_round_trips);
    FLARE_RUN(resp_set_body_records_bytes);
    FLARE_RUN(resp_set_body_str_records);
}
