#include "test_runner.h"
#include "mock_host.h"

#include "flare/request.h"

#include <string.h>

FLARE_TEST(req_method_returns_seeded_method) {
    flare_mock_reset();
    flare_mock_set_method("POST");

    const char *method = NULL;
    size_t len = 0;
    flare_status_t st = flare_req_method(&method, &len);

    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT_EQ_INT(len, 4);
    FLARE_ASSERT(method != NULL);
    FLARE_ASSERT(memcmp(method, "POST", 4) == 0);
}

FLARE_TEST(req_method_returns_not_found_when_unset) {
    flare_mock_reset();
    const char *method = NULL;
    size_t len = 99;
    flare_status_t st = flare_req_method(&method, &len);
    FLARE_ASSERT_EQ_INT(st, FLARE_ERR_NOT_FOUND);
    FLARE_ASSERT(method == NULL);
    FLARE_ASSERT_EQ_INT(len, 0);
}

FLARE_TEST(req_url_round_trips) {
    flare_mock_reset();
    flare_mock_set_url("https://example.test/path?q=1");

    const char *url = NULL;
    size_t len = 0;
    flare_status_t st = flare_req_url(&url, &len);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT_EQ_INT(len, 29);
    FLARE_ASSERT(memcmp(url, "https://example.test/path?q=1", 29) == 0);
}

FLARE_TEST(req_header_lookup_returns_value) {
    flare_mock_reset();
    flare_mock_set_request_header("X-Trace-Id", "abc-123");

    const char *value = NULL;
    size_t len = 0;
    flare_status_t st = flare_req_header("X-Trace-Id", 10, &value, &len);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT_EQ_INT(len, 7);
    FLARE_ASSERT(memcmp(value, "abc-123", 7) == 0);
}

FLARE_TEST(req_header_missing_returns_not_found) {
    flare_mock_reset();
    const char *value = NULL;
    size_t len = 0;
    flare_status_t st = flare_req_header("X-Missing", 9, &value, &len);
    FLARE_ASSERT_EQ_INT(st, FLARE_ERR_NOT_FOUND);
}

FLARE_TEST(req_body_returns_seeded_bytes) {
    flare_mock_reset();
    const uint8_t body[] = {0x01, 0x02, 0xff, 0x7f};
    flare_mock_set_request_body(body, sizeof(body));

    const uint8_t *out = NULL;
    size_t out_len = 0;
    flare_status_t st = flare_req_body(&out, &out_len);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT_EQ_INT(out_len, 4);
    FLARE_ASSERT_EQ_MEM(out, out_len, body, sizeof(body));
}

FLARE_TEST(req_body_empty_returns_not_found) {
    flare_mock_reset();
    const uint8_t *out = NULL;
    size_t out_len = 0;
    flare_status_t st = flare_req_body(&out, &out_len);
    FLARE_ASSERT_EQ_INT(st, FLARE_ERR_NOT_FOUND);
}

void register_request_tests(void) {
    FLARE_RUN(req_method_returns_seeded_method);
    FLARE_RUN(req_method_returns_not_found_when_unset);
    FLARE_RUN(req_url_round_trips);
    FLARE_RUN(req_header_lookup_returns_value);
    FLARE_RUN(req_header_missing_returns_not_found);
    FLARE_RUN(req_body_returns_seeded_bytes);
    FLARE_RUN(req_body_empty_returns_not_found);
}
