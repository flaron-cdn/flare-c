#ifndef FLARE_RUNTIME_H
#define FLARE_RUNTIME_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FLARE_ACTION_RESPOND     = 1,
    FLARE_ACTION_TRANSFORM   = 2,
    FLARE_ACTION_PASSTHROUGH = 3,
} flare_action_t;

int64_t flare_action_encode(flare_action_t action);

void flare_reset_arena(void);

int32_t flare_alloc(int32_t size);

uint8_t *flare_arena_addr(int32_t handle);

void flare_decode_ptr_len(int64_t packed, int32_t *out_handle, int32_t *out_len);

int32_t flare_copy_to_arena(const void *src, size_t len);

#if defined(__wasm__) || defined(__wasm32__)
#  define FLARE_EXPORT_ALLOC                                                  \
    int32_t flare__alloc_export(int32_t size);                                \
    __attribute__((export_name("alloc")))                                     \
    int32_t flare__alloc_export(int32_t size) { return flare_alloc(size); }

#  define FLARE_EXPORT_HANDLE_REQUEST(handler_fn)                             \
    int64_t flare__hr_export(void);                                           \
    __attribute__((export_name("handle_request")))                            \
    int64_t flare__hr_export(void) {                                          \
        flare_reset_arena();                                                  \
        return flare_action_encode((handler_fn)());                           \
    }

#  define FLARE_EXPORT_WS_HANDLERS(open_fn, message_fn, close_fn)             \
    int32_t flare__ws_open_export(void);                                      \
    int32_t flare__ws_message_export(void);                                   \
    int32_t flare__ws_close_export(void);                                     \
    __attribute__((export_name("ws_open")))                                   \
    int32_t flare__ws_open_export(void) {                                     \
        flare_reset_arena();                                                  \
        (open_fn)();                                                          \
        return 0;                                                             \
    }                                                                         \
    __attribute__((export_name("ws_message")))                                \
    int32_t flare__ws_message_export(void) {                                  \
        flare_reset_arena();                                                  \
        (message_fn)();                                                       \
        return 0;                                                             \
    }                                                                         \
    __attribute__((export_name("ws_close")))                                  \
    int32_t flare__ws_close_export(void) {                                    \
        flare_reset_arena();                                                  \
        (close_fn)();                                                         \
        return 0;                                                             \
    }
#else
#  define FLARE_EXPORT_ALLOC
#  define FLARE_EXPORT_HANDLE_REQUEST(handler_fn)
#  define FLARE_EXPORT_WS_HANDLERS(open_fn, message_fn, close_fn)
#endif

#ifdef __cplusplus
}
#endif

#endif
