/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "pathkit/core/SkMathPriv.h"
#include "pathkit/core/SkSafeMath.h"
#include "pathkit/private/SkFloatingPoint.h"

namespace pk {
///////////////////////////////////////////////////////////////////////////////

/* www.worldserver.com/turk/computergraphics/FixedSqrt.pdf
 */
int32_t SkSqrtBits(int32_t x, int count) {
    uint32_t root = 0;
    uint32_t remHi = 0;
    uint32_t remLo = x;

    do {
        root <<= 1;

        remHi = (remHi << 2) | (remLo >> 30);
        remLo <<= 2;

        uint32_t testDiv = (root << 1) + 1;
        if (remHi >= testDiv) {
            remHi -= testDiv;
            root++;
        }
    } while (--count >= 0);

    return root;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

size_t SkSafeMath::Add(size_t x, size_t y) {
    SkSafeMath tmp;
    size_t sum = tmp.add(x, y);
    return tmp.ok() ? sum : SIZE_MAX;
}

size_t SkSafeMath::Mul(size_t x, size_t y) {
    SkSafeMath tmp;
    size_t prod = tmp.mul(x, y);
    return tmp.ok() ? prod : SIZE_MAX;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool sk_floats_are_unit(const float array[], size_t count) {
    bool is_unit = true;
    for (size_t i = 0; i < count; ++i) {
        is_unit &= (array[i] >= 0) & (array[i] <= 1);
    }
    return is_unit;
}
}  // namespace pk