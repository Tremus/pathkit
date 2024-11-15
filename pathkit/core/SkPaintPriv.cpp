/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "pathkit/core/SkPaintPriv.h"
#include "pathkit/core/SkMatrix.h"
#include "pathkit/core/SkPoint.h"

namespace pk {
SkScalar SkPaintPriv::ComputeResScaleForStroking(const SkMatrix& matrix) {
    // Not sure how to handle perspective differently, so we just don't try (yet)
    SkScalar sx = SkPoint::Length(matrix[SkMatrix::kMScaleX], matrix[SkMatrix::kMSkewY]);
    SkScalar sy = SkPoint::Length(matrix[SkMatrix::kMSkewX], matrix[SkMatrix::kMScaleY]);
    if (SkScalarsAreFinite(sx, sy)) {
        SkScalar scale = std::max(sx, sy);
        if (scale > 0) {
            return scale;
        }
    }
    return 1;
}
}  // namespace pk
