#include <flare.h>

static const char body[] = "hello from flare-c";

static flare_action_t handle_request(void) {
    flare_resp_set_status(200);
    flare_resp_set_header("content-type", 12, "text/plain", 10);
    flare_resp_set_body((const uint8_t *)body, sizeof(body) - 1);
    return FLARE_ACTION_RESPOND;
}

FLARE_EXPORT_HANDLE_REQUEST(handle_request)
