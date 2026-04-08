#include "test_runner.h"
#include "mock_host.h"

#include "flare/ws.h"
#include "flare/runtime.h"

#include <string.h>

FLARE_TEST(ws_send_records_payload) {
    flare_mock_reset();
    flare_reset_arena();
    const uint8_t data[] = {0x01, 0x02, 0x03};
    flare_status_t st = flare_ws_send(data, 3);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    size_t got_len = 0;
    const uint8_t *got = flare_mock_ws_last_send(&got_len);
    FLARE_ASSERT_EQ_INT(got_len, 3);
    FLARE_ASSERT_EQ_MEM(got, got_len, data, 3);
    FLARE_ASSERT_EQ_INT(flare_mock_ws_send_count(), 1);
}

FLARE_TEST(ws_send_text_round_trips) {
    flare_mock_reset();
    flare_reset_arena();
    flare_status_t st = flare_ws_send_text("hello", 5);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    size_t got_len = 0;
    const uint8_t *got = flare_mock_ws_last_send(&got_len);
    FLARE_ASSERT_EQ_INT(got_len, 5);
    FLARE_ASSERT(memcmp(got, "hello", 5) == 0);
}

FLARE_TEST(ws_send_propagates_send_error) {
    flare_mock_reset();
    flare_reset_arena();
    flare_mock_ws_set_send_error(1);
    flare_status_t st = flare_ws_send((const uint8_t *)"x", 1);
    FLARE_ASSERT_EQ_INT(st, FLARE_ERR_WS_SEND);
}

FLARE_TEST(ws_close_records) {
    flare_mock_reset();
    flare_reset_arena();
    flare_ws_close(1000);
    FLARE_ASSERT_EQ_INT(flare_mock_ws_close_called(), 1);
    FLARE_ASSERT_EQ_INT(flare_mock_ws_close_code_seen(), 1000);
}

FLARE_TEST(ws_event_type_returns_seeded) {
    flare_mock_reset();
    flare_reset_arena();
    flare_mock_set_ws_event_type("message");
    const char *type = NULL;
    size_t len = 0;
    flare_status_t st = flare_ws_event_type(&type, &len);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT_EQ_INT(len, 7);
    FLARE_ASSERT(memcmp(type, "message", 7) == 0);
}

FLARE_TEST(ws_event_data_returns_seeded) {
    flare_mock_reset();
    flare_reset_arena();
    const uint8_t payload[] = {0xab, 0xcd, 0xef};
    flare_mock_set_ws_event_data(payload, 3);

    const uint8_t *got = NULL;
    size_t got_len = 0;
    flare_status_t st = flare_ws_event_data(&got, &got_len);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT_EQ_INT(got_len, 3);
    FLARE_ASSERT_EQ_MEM(got, got_len, payload, 3);
}

FLARE_TEST(ws_conn_id_returns_seeded) {
    flare_mock_reset();
    flare_reset_arena();
    flare_mock_set_ws_conn_id("conn-42");
    const char *id = NULL;
    size_t len = 0;
    flare_status_t st = flare_ws_conn_id(&id, &len);
    FLARE_ASSERT_EQ_INT(st, FLARE_OK);
    FLARE_ASSERT_EQ_INT(len, 7);
    FLARE_ASSERT(memcmp(id, "conn-42", 7) == 0);
}

FLARE_TEST(ws_close_code_passes_through) {
    flare_mock_reset();
    flare_mock_set_ws_close_code(1011);
    FLARE_ASSERT_EQ_INT(flare_ws_close_code(), 1011);
}

void register_ws_tests(void) {
    FLARE_RUN(ws_send_records_payload);
    FLARE_RUN(ws_send_text_round_trips);
    FLARE_RUN(ws_send_propagates_send_error);
    FLARE_RUN(ws_close_records);
    FLARE_RUN(ws_event_type_returns_seeded);
    FLARE_RUN(ws_event_data_returns_seeded);
    FLARE_RUN(ws_conn_id_returns_seeded);
    FLARE_RUN(ws_close_code_passes_through);
}
