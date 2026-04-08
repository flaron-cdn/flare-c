#ifndef FLARE_INTERNAL_MEM_H
#define FLARE_INTERNAL_MEM_H

/*
 * Tiny mem* shims that work in both freestanding wasm32 (no libc) and
 * native test builds (libc available). The wasm side cannot include
 * <string.h>; the native side could but uses these wrappers for symmetry.
 *
 * Implemented in runtime.c.
 */

#include <stddef.h>

void *flare_memcpy(void *dst, const void *src, size_t n);
void *flare_memset(void *dst, int c, size_t n);
size_t flare_strlen(const char *s);

#endif
