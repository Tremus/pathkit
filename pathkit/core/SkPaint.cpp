/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "pathkit/core/SkPaint.h"

#include "pathkit/core/SkPaintDefaults.h"
#include "pathkit/core/SkPaintPriv.h"
#include "pathkit/core/SkPathEffect.h"
#include "pathkit/core/SkScalar.h"
#include "pathkit/core/SkStrokeRec.h"
#include "pathkit/private/SkTo.h"

namespace pk {
SkPaint::SkPaint()
        : fWidth{0}
        , fMiterLimit{PkPaintDefaults_MiterLimit}
        , fBitfields{(unsigned)false,                   // fAntiAlias
                     (unsigned)false,                   // fDither
                     (unsigned)SkPaint::kDefault_Cap,   // fCapType
                     (unsigned)SkPaint::kDefault_Join,  // fJoinType
                     (unsigned)SkPaint::kFill_Style,    // fStyle
                     0}                                 // fPadding
{
    static_assert(sizeof(fBitfields) == sizeof(fBitfieldsUInt), "");
}

SkPaint::SkPaint(const SkPaint& src) = default;

SkPaint::SkPaint(SkPaint&& src) = default;

SkPaint::~SkPaint() = default;

SkPaint& SkPaint::operator=(const SkPaint& src) = default;

SkPaint& SkPaint::operator=(SkPaint&& src) = default;

bool operator==(const SkPaint& a, const SkPaint& b) {
#define EQUAL(field) (a.field == b.field)
    return EQUAL(fWidth) && EQUAL(fMiterLimit) && EQUAL(fBitfieldsUInt);
#undef EQUAL
}

void SkPaint::reset() { *this = SkPaint(); }

void SkPaint::setStyle(Style style) {
    if ((unsigned)style < kStyleCount) {
        fBitfields.fStyle = style;
    }
}

void SkPaint::setStroke(bool isStroke) {
    fBitfields.fStyle = isStroke ? kStroke_Style : kFill_Style;
}

void SkPaint::setStrokeWidth(SkScalar width) {
    if (width >= 0) {
        fWidth = width;
    }
}

void SkPaint::setStrokeMiter(SkScalar limit) {
    if (limit >= 0) {
        fMiterLimit = limit;
    }
}

void SkPaint::setStrokeCap(Cap ct) {
    if ((unsigned)ct < kCapCount) {
        fBitfields.fCapType = SkToU8(ct);
    }
}

void SkPaint::setStrokeJoin(Join jt) {
    if ((unsigned)jt < kJoinCount) {
        fBitfields.fJoinType = SkToU8(jt);
    }
}

bool SkPaint::getFillPath(const SkPath& src,
                          SkPath* dst,
                          const SkRect* cullRect,
                          SkScalar resScale) const {
    return this->getFillPath(src, dst, cullRect, SkMatrix::Scale(resScale, resScale));
}

bool SkPaint::getFillPath(const SkPath& src,
                          SkPath* dst,
                          const SkRect* cullRect,
                          const SkMatrix& ctm) const {
    if (!src.isFinite()) {
        dst->reset();
        return false;
    }

    const SkScalar resScale = SkPaintPriv::ComputeResScaleForStroking(ctm);
    SkStrokeRec rec(*this, resScale);

    const SkPath* srcPtr = &src;
    SkPath tmpPath;

    if (!rec.applyToPath(dst, *srcPtr)) {
        if (srcPtr == &tmpPath) {
            // If path's were copy-on-write, this trick would not be needed.
            // As it is, we want to save making a deep-copy from tmpPath -> dst
            // since we know we're just going to delete tmpPath when we return,
            // so the swap saves that copy.
            dst->swap(tmpPath);
        } else {
            *dst = *srcPtr;
        }
    }

    if (!dst->isFinite()) {
        dst->reset();
        return false;
    }
    return !rec.isHairlineStyle();
}
}  // namespace pk