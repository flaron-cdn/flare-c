#ifndef FLARE_BEAM_H
#define FLARE_BEAM_H

#include <stdint.h>
#include <stddef.h>

#include "flare/status.h"

#ifdef __cplusplus
extern "C" {
#endif

flare_status_t flare_beam_fetch(const char *url, size_t url_len,
                                const char *opts_json, size_t opts_json_len,
                                const uint8_t **out_response_json, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif
