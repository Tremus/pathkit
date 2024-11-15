/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <cstddef>
#include <cstring>

#include "pathkit/core/SkTypes.h"

namespace pk {
/*
    memory wrappers to be implemented by the porting layer (platform)
*/

/** Free memory returned by sk_malloc(). It is safe to pass null. */
PK_API extern void sk_free(void*);

enum {
    /**
     *  If this bit is set, the returned buffer must be zero-initialized. If this bit is not set
     *  the buffer can be uninitialized.
     */
    PK_MALLOC_ZERO_INITIALIZE = 1 << 0,

    /**
     *  If this bit is set, the implementation must throw/crash/quit if the request cannot
     *  be fulfilled. If this bit is not set, then it should return nullptr on failure.
     */
    PK_MALLOC_THROW = 1 << 1,
};
/**
 *  Return a block of memory (at least 4-byte aligned) of at least the specified size.
 *  If the requested memory cannot be returned, either return nullptr or throw/exit, depending
 *  on the PK_MALLOC_THROW bit. If the allocation succeeds, the memory will be zero-initialized
 *  if the PK_MALLOC_ZERO_INITIALIZE bit was set.
 *
 *  To free the memory, call sk_free()
 */
PK_API extern void* sk_malloc_flags(size_t size, unsigned flags);

/** Same as standard realloc(), but this one never returns null on failure. It will throw
 *  an exception if it fails.
 */
PK_API extern void* sk_realloc_throw(void* buffer, size_t size);

static inline void* sk_malloc_throw(size_t size) { return sk_malloc_flags(size, PK_MALLOC_THROW); }

// Performs a safe multiply count * elemSize, checking for overflow
PK_API extern void* sk_malloc_throw(size_t count, size_t elemSize);
PK_API extern void* sk_realloc_throw(void* buffer, size_t count, size_t elemSize);

/**
 *  These variants return nullptr on failure
 */
static inline void* sk_malloc_canfail(size_t size) { return sk_malloc_flags(size, 0); }

// bzero is safer than memset, but we can't rely on it, so... sk_bzero()
static inline void sk_bzero(void* buffer, size_t size) {
    // Please c.f. sk_careful_memcpy.  It's undefined behavior to call memset(null, 0, 0).
    if (size) {
        memset(buffer, 0, size);
    }
}

/**
 *  sk_careful_memcpy() is just like memcpy(), but guards against undefined behavior.
 *
 * It is undefined behavior to call memcpy() with null dst or src, even if len is 0.
 * If an optimizer is "smart" enough, it can exploit this to do unexpected things.
 *     memcpy(dst, src, 0);
 *     if (src) {
 *         printf("%x\n", *src);
 *     }
 * In this code the compiler can assume src is not null and omit the if (src) {...} check,
 * unconditionally running the printf, crashing the program if src really is null.
 * Of the compilers we pay attention to only GCC performs this optimization in practice.
 */
static inline void* sk_careful_memcpy(void* dst, const void* src, size_t len) {
    // When we pass >0 len we had better already be passing valid pointers.
    // So we just need to skip calling memcpy when len == 0.
    if (len) {
        memcpy(dst, src, len);
    }
    return dst;
}
}  // namespace pk
