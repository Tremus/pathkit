/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "pathkit/core/SkMathPriv.h"
#include "pathkit/core/SkRect.h"

namespace pk {
class SkRectPriv {
public:
    // Returns r.width()/2 but divides first to avoid width() overflowing.
    static SkScalar HalfWidth(const SkRect& r) {
        return PkScalarHalf(r.fRight) - PkScalarHalf(r.fLeft);
    }
    // Returns r.height()/2 but divides first to avoid height() overflowing.
    static SkScalar HalfHeight(const SkRect& r) {
        return PkScalarHalf(r.fBottom) - PkScalarHalf(r.fTop);
    }
};
}  // namespace pk
