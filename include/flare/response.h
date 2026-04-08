#ifndef FLARE_RESPONSE_H
#define FLARE_RESPONSE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void flare_resp_set_status(uint16_t code);

void flare_resp_set_header(const char *name, size_t name_len,
                           const char *value, size_t value_len);

void flare_resp_set_body(const uint8_t *body, size_t len);

void flare_resp_set_body_str(const char *body, size_t len);

#ifdef __cplusplus
}
#endif

#endif
