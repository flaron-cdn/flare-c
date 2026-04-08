#include "test_runner.h"
#include "mock_host.h"

#include "flare/beam.h"
#include "flare/runtime.h"

#include <string.h>

FLARE_TEST(beam_fetch_returns_response_json) {
    flare_mock_reset();
    flare_reset_arena();
    flare_mock_set_beam_response("{\"status\":200,\"body\":\"hi\"}");

    const uint8_t *out = NULL;
    size_t out_len = 0;
    flare_status_t st = flare_beam_fetch("https://x.test", 14,
                                         "{\"method\":\"GET\"}", 16,
                                         &out, &out_len);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT(out != NULL);
    FLARE_ASSERT(out_len > 0);
    FLARE_ASSERT(memcmp(out, "{\"status\":200,\"body\":\"hi\"}", out_len) == 0);
}

FLARE_TEST(beam_fetch_no_response_returns_no_response_err) {
    flare_mock_reset();
    flare_reset_arena();
    const uint8_t *out = NULL;
    size_t out_len = 0;
    flare_status_t st = flare_beam_fetch("https://x.test", 14, NULL, 0, &out, &out_len);
    FLARE_ASSERT_EQ_INT(st, FLARE_ERR_NO_RESPONSE);
}

void register_beam_tests(void) {
    FLARE_RUN(beam_fetch_returns_response_json);
    FLARE_RUN(beam_fetch_no_response_returns_no_response_err);
}
