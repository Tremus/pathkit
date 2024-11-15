/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "pathkit/core/SkPathEffect.h"

namespace pk {
/** \class SkCornerPathEffect

    SkCornerPathEffect is a subclass of SkPathEffect that can turn sharp corners
    into various treatments (e.g. rounded corners)
*/
class PK_API SkCornerPathEffect {
public:
    /** radius must be > 0 to have an effect. It specifies the distance from each corner
        that should be "rounded".
    */
    static sk_sp<SkPathEffect> Make(SkScalar radius);
};
}  // namespace pk
