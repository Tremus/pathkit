/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "pathkit/core/SkPaint.h"
#include "pathkit/core/SkPath.h"
#include "pathkit/core/SkPoint.h"
#include "pathkit/core/SkStrokerPriv.h"
#include "pathkit/private/SkTo.h"

namespace pk {
/** \class SkStroke
    SkStroke is the utility class that constructs paths by stroking
    geometries (lines, rects, ovals, roundrects, paths). This is
    invoked when a geometry or text is drawn in a canvas with the
    kStroke_Mask bit set in the paint.
*/
class SkStroke {
public:
    SkStroke();

    SkPaint::Cap getCap() const { return (SkPaint::Cap)fCap; }
    void setCap(SkPaint::Cap);

    SkPaint::Join getJoin() const { return (SkPaint::Join)fJoin; }
    void setJoin(SkPaint::Join);

    void setMiterLimit(SkScalar);
    void setWidth(SkScalar);

    bool getDoFill() const { return SkToBool(fDoFill); }
    void setDoFill(bool doFill) { fDoFill = SkToU8(doFill); }

    /**
     *  ResScale is the "intended" resolution for the output.
     *      Default is 1.0.
     *      Larger values (res > 1) indicate that the result should be more precise, since it will
     *          be zoomed up, and small errors will be magnified.
     *      Smaller values (0 < res < 1) indicate that the result can be less precise, since it will
     *          be zoomed down, and small errors may be invisible.
     */
    SkScalar getResScale() const { return fResScale; }
    void setResScale(SkScalar rs) { fResScale = rs; }

    /**
     *  Stroke the specified rect, winding it in the specified direction..
     */
    void strokeRect(const SkRect& rect,
                    SkPath* result,
                    SkPathDirection = SkPathDirection::kCW) const;
    void strokePath(const SkPath& path, SkPath*) const;

    ////////////////////////////////////////////////////////////////

private:
    SkScalar fWidth, fMiterLimit;
    SkScalar fResScale;
    uint8_t fCap, fJoin;
    bool fDoFill;

    friend class SkPaint;
};
}  // namespace pk
