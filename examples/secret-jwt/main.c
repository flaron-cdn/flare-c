#include <flare.h>

/*
 * Issue a signed JWT to authenticated callers. Uses the host's
 * crypto_sign_jwt host function with HS256 and a secret stored in the
 * domain's secret store under the name "jwt-key".
 *
 * Required capability: allowed_secrets must include "jwt-key"
 */

static flare_action_t handle_request(void) {
    flare_jwt_claim_t claims[] = {
        {"sub", 3, "demo-user", 9},
        {"iss", 3, "flaron",    6},
    };

    const char *jwt = NULL;
    size_t jwt_len = 0;
    flare_status_t st = flare_crypto_sign_jwt(
        "HS256", 5, "jwt-key", 7,
        claims, sizeof(claims) / sizeof(claims[0]),
        &jwt, &jwt_len);

    if (st != FLARE_OK) {
        flare_resp_set_status(500);
        flare_resp_set_body((const uint8_t *)"jwt sign failed", 15);
        return FLARE_ACTION_RESPOND;
    }

    flare_resp_set_status(200);
    flare_resp_set_header("content-type", 12, "application/jwt", 15);
    flare_resp_set_body((const uint8_t *)jwt, jwt_len);
    return FLARE_ACTION_RESPOND;
}

FLARE_EXPORT_HANDLE_REQUEST(handle_request)
