#include "flare/runtime.h"
#include "flare_mem.h"

#include <stdint.h>
#include <stddef.h>

#define FLARE_ARENA_SIZE (256 * 1024)
/* First 8 bytes are reserved so handle 0 stays as the "no result" sentinel. */
#define FLARE_ARENA_BASE 8

uint8_t flare_arena_buf[FLARE_ARENA_SIZE];
static int32_t flare_arena_offset = FLARE_ARENA_BASE;

int64_t flare_action_encode(flare_action_t action) {
    return ((int64_t)(uint32_t)action) << 32;
}

void flare_reset_arena(void) {
    flare_arena_offset = FLARE_ARENA_BASE;
}

int32_t flare_alloc(int32_t size) {
    if (size <= 0) return 0;
    int32_t aligned = (flare_arena_offset + 7) & ~7;
    if (aligned + size > FLARE_ARENA_SIZE || aligned + size < aligned) {
        return 0;
    }
    flare_arena_offset = aligned + size;
#if defined(__wasm__) || defined(__wasm32__)
    return (int32_t)(intptr_t)(flare_arena_buf + aligned);
#else
    return aligned;
#endif
}

uint8_t *flare_arena_addr(int32_t handle) {
    if (handle == 0) return NULL;
#if defined(__wasm__) || defined(__wasm32__)
    return (uint8_t *)(intptr_t)handle;
#else
    return flare_arena_buf + handle;
#endif
}

void flare_decode_ptr_len(int64_t packed, int32_t *out_handle, int32_t *out_len) {
    uint64_t u = (uint64_t)packed;
    if (out_handle) *out_handle = (int32_t)(u >> 32);
    if (out_len)    *out_len    = (int32_t)(u & 0xFFFFFFFFu);
}

int32_t flare_copy_to_arena(const void *src, size_t len) {
    if (len == 0) return 0;
    int32_t handle = flare_alloc((int32_t)len);
    if (handle == 0) return 0;
    flare_memcpy(flare_arena_addr(handle), src, len);
    return handle;
}

void *flare_memcpy(void *dst, const void *src, size_t n) {
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    for (size_t i = 0; i < n; i++) d[i] = s[i];
    return dst;
}

void *flare_memset(void *dst, int c, size_t n) {
    uint8_t *d = (uint8_t *)dst;
    uint8_t v = (uint8_t)c;
    for (size_t i = 0; i < n; i++) d[i] = v;
    return dst;
}

size_t flare_strlen(const char *s) {
    size_t n = 0;
    while (s[n] != '\0') n++;
    return n;
}

#if defined(__wasm__) || defined(__wasm32__)
/*
 * The clang code generator may emit calls to memcpy/memset for non-trivial
 * struct copies and large literal initialisation, even with -fno-builtin.
 * Provide them as plain symbols so the wasm linker can resolve those calls
 * without dragging in libc.
 */
void *memcpy(void *dst, const void *src, size_t n);
void *memset(void *dst, int c, size_t n);

void *memcpy(void *dst, const void *src, size_t n) {
    return flare_memcpy(dst, src, n);
}
void *memset(void *dst, int c, size_t n) {
    return flare_memset(dst, c, n);
}
#endif

#if defined(__wasm__) || defined(__wasm32__)
int32_t flare__alloc_export_default(int32_t size);

__attribute__((export_name("alloc")))
int32_t flare__alloc_export_default(int32_t size) {
    return flare_alloc(size);
}
#endif
