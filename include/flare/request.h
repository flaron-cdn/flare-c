#ifndef FLARE_REQUEST_H
#define FLARE_REQUEST_H

#include <stdint.h>
#include <stddef.h>

#include "flare/status.h"

#ifdef __cplusplus
extern "C" {
#endif

flare_status_t flare_req_method(const char **out_str, size_t *out_len);

flare_status_t flare_req_url(const char **out_str, size_t *out_len);

flare_status_t flare_req_header(const char *name, size_t name_len,
                                const char **out_value, size_t *out_value_len);

flare_status_t flare_req_body(const uint8_t **out_body, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif
