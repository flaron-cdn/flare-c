#include "flare/log.h"
#include "flare/runtime.h"
#include "flare/env.h"

static int32_t copy_msg(const char *msg, size_t len) {
    if (msg == NULL || len == 0) return 0;
    return flare_copy_to_arena(msg, len);
}

void flare_log_info(const char *msg, size_t len) {
    int32_t h = copy_msg(msg, len);
    if (h == 0 && len > 0) return;
    log_info(h, (int32_t)len);
}

void flare_log_warn(const char *msg, size_t len) {
    int32_t h = copy_msg(msg, len);
    if (h == 0 && len > 0) return;
    log_warn(h, (int32_t)len);
}

void flare_log_error(const char *msg, size_t len) {
    int32_t h = copy_msg(msg, len);
    if (h == 0 && len > 0) return;
    log_error(h, (int32_t)len);
}
