#ifndef FLARE_LOG_H
#define FLARE_LOG_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void flare_log_info(const char *msg, size_t len);
void flare_log_warn(const char *msg, size_t len);
void flare_log_error(const char *msg, size_t len);

#ifdef __cplusplus
}
#endif

#endif
