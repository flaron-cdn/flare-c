#include <flare.h>

/*
 * Outbound HTTP example: fetches https://example.com via the host's
 * Beam client and returns the upstream response body verbatim. The host
 * returns a JSON envelope ({status, headers, body}) — for simplicity this
 * example forwards the JSON as-is, but in production you would parse it
 * and reshape the response.
 *
 * Required capability: writes_beam = true (or whatever your domain config
 * uses to allow outbound fetches).
 */

static const char URL[]  = "https://example.com/";
static const char OPTS[] = "{\"method\":\"GET\"}";

static flare_action_t handle_request(void) {
    const uint8_t *resp = NULL;
    size_t resp_len = 0;
    flare_status_t st = flare_beam_fetch(
        URL,  sizeof(URL) - 1,
        OPTS, sizeof(OPTS) - 1,
        &resp, &resp_len);

    if (st != FLARE_OK) {
        flare_resp_set_status(502);
        flare_resp_set_body((const uint8_t *)"upstream fetch failed", 21);
        return FLARE_ACTION_RESPOND;
    }

    flare_resp_set_status(200);
    flare_resp_set_header("content-type", 12, "application/json", 16);
    flare_resp_set_body(resp, resp_len);
    return FLARE_ACTION_RESPOND;
}

FLARE_EXPORT_HANDLE_REQUEST(handle_request)
