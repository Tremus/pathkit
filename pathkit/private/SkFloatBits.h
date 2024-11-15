/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <float.h>

#include "pathkit/core/SkTypes.h"
#include "pathkit/private/SkSafe_math.h"

namespace pk {
/** Convert a sign-bit int (i.e. float interpreted as int) into a 2s compliement
    int. This also converts -0 (0x80000000) to 0. Doing this to a float allows
    it to be compared using normal C operators (<, <=, etc.)
*/
static inline int32_t SkSignBitTo2sCompliment(int32_t x) {
    if (x < 0) {
        x &= 0x7FFFFFFF;
        x = -x;
    }
    return x;
}

union SkFloatIntUnion {
    float fFloat;
    int32_t fSignBitInt;
};

// Helper to see a float as its bit pattern (w/o aliasing warnings)
static inline int32_t SkFloat2Bits(float x) {
    SkFloatIntUnion data;
    data.fFloat = x;
    return data.fSignBitInt;
}

constexpr int32_t gFloatBits_exponent_mask = 0x7F800000;
constexpr int32_t gFloatBits_matissa_mask = 0x007FFFFF;

static inline bool SkFloatBits_IsFinite(int32_t bits) {
    return (bits & gFloatBits_exponent_mask) != gFloatBits_exponent_mask;
}

static inline bool SkFloatBits_IsInf(int32_t bits) {
    return ((bits & gFloatBits_exponent_mask) == gFloatBits_exponent_mask) &&
           (bits & gFloatBits_matissa_mask) == 0;
}

/** Return the float as a 2s compliment int. Just to be used to compare floats
    to each other or against positive float-bit-constants (like 0). This does
    not return the int equivalent of the float, just something cheaper for
    compares-only.
 */
static inline int32_t SkFloatAs2sCompliment(float x) {
    return SkSignBitTo2sCompliment(SkFloat2Bits(x));
}

//  Scalar wrappers for float-bit routines

#define PkScalarAs2sCompliment(x) SkFloatAs2sCompliment(x)
}  // namespace pk
