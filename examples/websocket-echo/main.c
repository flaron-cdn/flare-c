#include <flare.h>

/*
 * WebSocket echo flare. Greets new connections, echoes every message back
 * to the sender, and silently handles close events.
 *
 * The flare exports ws_open, ws_message, and ws_close instead of
 * handle_request. The host invokes whichever event matches the connection
 * lifecycle.
 */

static void ws_open(void) {
    flare_ws_send_text("welcome", 7);
}

static void ws_message(void) {
    const uint8_t *data = NULL;
    size_t data_len = 0;
    if (flare_ws_event_data(&data, &data_len) == FLARE_OK && data_len > 0) {
        flare_ws_send(data, data_len);
    }
}

static void ws_close(void) {
    /* Per-event arena reset is automatic; nothing to clean up. */
}

FLARE_EXPORT_WS_HANDLERS(ws_open, ws_message, ws_close)
