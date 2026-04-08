#ifndef FLARE_WS_H
#define FLARE_WS_H

#include <stdint.h>
#include <stddef.h>

#include "flare/status.h"

#ifdef __cplusplus
extern "C" {
#endif

flare_status_t flare_ws_send(const uint8_t *data, size_t len);

flare_status_t flare_ws_send_text(const char *text, size_t len);

void flare_ws_close(uint16_t code);

flare_status_t flare_ws_conn_id(const char **out_id, size_t *out_len);

flare_status_t flare_ws_event_type(const char **out_type, size_t *out_len);

flare_status_t flare_ws_event_data(const uint8_t **out_data, size_t *out_len);

uint16_t flare_ws_close_code(void);

#ifdef __cplusplus
}
#endif

#endif
