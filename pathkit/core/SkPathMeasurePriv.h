/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "pathkit/core/SkGeometry.h"
#include "pathkit/core/SkPath.h"
#include "pathkit/core/SkPoint.h"

namespace pk {
// Used in the Segment struct defined in SkPathMeasure.h
// It is used as a 2-bit field so if you add to this
// you must increase the size of the bitfield there.
enum SkSegType {
    kLine_SegType,
    kQuad_SegType,
    kCubic_SegType,
    kConic_SegType,
};

}  // namespace pk
