/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#pragma once

#include "pathkit/pathops/SkIntersectionHelper.h"
#include "pathkit/pathops/SkIntersections.h"

namespace pk {
class SkOpCoincidence;

bool AddIntersectTs(SkOpContour* test, SkOpContour* next, SkOpCoincidence* coincidence);
}  // namespace pk
