#include "test_runner.h"
#include "mock_host.h"

#include "flare/log.h"

#include <string.h>

FLARE_TEST(log_info_records_message) {
    flare_mock_reset();
    flare_log_info("started", 7);
    FLARE_ASSERT_EQ_INT(flare_mock_log_count(0), 1);
    FLARE_ASSERT_EQ_STR(flare_mock_last_log(0), "started");
}

FLARE_TEST(log_warn_records_message) {
    flare_mock_reset();
    flare_log_warn("careful", 7);
    FLARE_ASSERT_EQ_INT(flare_mock_log_count(1), 1);
    FLARE_ASSERT_EQ_STR(flare_mock_last_log(1), "careful");
}

FLARE_TEST(log_error_records_message) {
    flare_mock_reset();
    flare_log_error("oh no", 5);
    FLARE_ASSERT_EQ_INT(flare_mock_log_count(2), 1);
    FLARE_ASSERT_EQ_STR(flare_mock_last_log(2), "oh no");
}

void register_log_tests(void) {
    FLARE_RUN(log_info_records_message);
    FLARE_RUN(log_warn_records_message);
    FLARE_RUN(log_error_records_message);
}
