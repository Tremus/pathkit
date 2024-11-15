/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "pathkit/core/SkContourMeasure.h"
#include "pathkit/core/SkGeometry.h"
#include "pathkit/core/SkPath.h"
#include "pathkit/core/SkPathMeasurePriv.h"
#include "pathkit/core/SkPathPriv.h"

namespace pk {
#define kMaxTValue 0x3FFFFFFF

constexpr static inline SkScalar tValue2Scalar(int t) {
    // 1/kMaxTValue can't be represented as a float, but it's close and the limits work fine.
    const SkScalar kMaxTReciprocal = 1.0f / (SkScalar)kMaxTValue;
    return t * kMaxTReciprocal;
}

static_assert(0.0f == tValue2Scalar(0), "Lower limit should be exact.");
static_assert(1.0f == tValue2Scalar(kMaxTValue), "Upper limit should be exact.");

SkScalar SkContourMeasure::Segment::getScalarT() const { return tValue2Scalar(fTValue); }

void SkContourMeasure_segTo(
        const SkPoint pts[], unsigned segType, SkScalar startT, SkScalar stopT, SkPath* dst) {
    if (startT == stopT) {
        if (!dst->isEmpty()) {
            /* if the dash as a zero-length on segment, add a corresponding zero-length line.
               The stroke code will add end caps to zero length lines as appropriate */
            SkPoint lastPt;
            PkAssertResult(dst->getLastPt(&lastPt));
            dst->lineTo(lastPt);
        }
        return;
    }

    SkPoint tmp0[7], tmp1[7];

    switch (segType) {
        case kLine_SegType:
            if (PK_Scalar1 == stopT) {
                dst->lineTo(pts[1]);
            } else {
                dst->lineTo(SkScalarInterp(pts[0].fX, pts[1].fX, stopT),
                            SkScalarInterp(pts[0].fY, pts[1].fY, stopT));
            }
            break;
        case kQuad_SegType:
            if (0 == startT) {
                if (PK_Scalar1 == stopT) {
                    dst->quadTo(pts[1], pts[2]);
                } else {
                    SkChopQuadAt(pts, tmp0, stopT);
                    dst->quadTo(tmp0[1], tmp0[2]);
                }
            } else {
                SkChopQuadAt(pts, tmp0, startT);
                if (PK_Scalar1 == stopT) {
                    dst->quadTo(tmp0[3], tmp0[4]);
                } else {
                    SkChopQuadAt(&tmp0[2], tmp1, (stopT - startT) / (1 - startT));
                    dst->quadTo(tmp1[1], tmp1[2]);
                }
            }
            break;
        case kConic_SegType: {
            SkConic conic(pts[0], pts[2], pts[3], pts[1].fX);

            if (0 == startT) {
                if (PK_Scalar1 == stopT) {
                    dst->conicTo(conic.fPts[1], conic.fPts[2], conic.fW);
                } else {
                    SkConic tmp[2];
                    if (conic.chopAt(stopT, tmp)) {
                        dst->conicTo(tmp[0].fPts[1], tmp[0].fPts[2], tmp[0].fW);
                    }
                }
            } else {
                if (PK_Scalar1 == stopT) {
                    SkConic tmp[2];
                    if (conic.chopAt(startT, tmp)) {
                        dst->conicTo(tmp[1].fPts[1], tmp[1].fPts[2], tmp[1].fW);
                    }
                } else {
                    SkConic tmp;
                    conic.chopAt(startT, stopT, &tmp);
                    dst->conicTo(tmp.fPts[1], tmp.fPts[2], tmp.fW);
                }
            }
        } break;
        case kCubic_SegType:
            if (0 == startT) {
                if (PK_Scalar1 == stopT) {
                    dst->cubicTo(pts[1], pts[2], pts[3]);
                } else {
                    SkChopCubicAt(pts, tmp0, stopT);
                    dst->cubicTo(tmp0[1], tmp0[2], tmp0[3]);
                }
            } else {
                SkChopCubicAt(pts, tmp0, startT);
                if (PK_Scalar1 == stopT) {
                    dst->cubicTo(tmp0[4], tmp0[5], tmp0[6]);
                } else {
                    SkChopCubicAt(&tmp0[3], tmp1, (stopT - startT) / (1 - startT));
                    dst->cubicTo(tmp1[1], tmp1[2], tmp1[3]);
                }
            }
            break;
        default:
            PK_ABORT("unknown segType");
    }
}

///////////////////////////////////////////////////////////////////////////////

static inline int tspan_big_enough(int tspan) { return tspan >> 10; }

// can't use tangents, since we need [0..1..................2] to be seen
// as definitely not a line (it is when drawn, but not parametrically)
// so we compare midpoints
#define CHEAP_DIST_LIMIT (PK_Scalar1 / 2)  // just made this value up

static bool quad_too_curvy(const SkPoint pts[3], SkScalar tolerance) {
    // diff = (a/4 + b/2 + c/4) - (a/2 + c/2)
    // diff = -a/4 + b/2 - c/4
    SkScalar dx = PkScalarHalf(pts[1].fX) - PkScalarHalf(PkScalarHalf(pts[0].fX + pts[2].fX));
    SkScalar dy = PkScalarHalf(pts[1].fY) - PkScalarHalf(PkScalarHalf(pts[0].fY + pts[2].fY));

    SkScalar dist = std::max(PkScalarAbs(dx), PkScalarAbs(dy));
    return dist > tolerance;
}

static bool conic_too_curvy(const SkPoint& firstPt,
                            const SkPoint& midTPt,
                            const SkPoint& lastPt,
                            SkScalar tolerance) {
    SkPoint midEnds = firstPt + lastPt;
    midEnds *= 0.5f;
    SkVector dxy = midTPt - midEnds;
    SkScalar dist = std::max(PkScalarAbs(dxy.fX), PkScalarAbs(dxy.fY));
    return dist > tolerance;
}

static bool cheap_dist_exceeds_limit(const SkPoint& pt,
                                     SkScalar x,
                                     SkScalar y,
                                     SkScalar tolerance) {
    SkScalar dist = std::max(PkScalarAbs(x - pt.fX), PkScalarAbs(y - pt.fY));
    // just made up the 1/2
    return dist > tolerance;
}

static bool cubic_too_curvy(const SkPoint pts[4], SkScalar tolerance) {
    return cheap_dist_exceeds_limit(pts[1],
                                    SkScalarInterp(pts[0].fX, pts[3].fX, PK_Scalar1 / 3),
                                    SkScalarInterp(pts[0].fY, pts[3].fY, PK_Scalar1 / 3),
                                    tolerance) ||
           cheap_dist_exceeds_limit(pts[2],
                                    SkScalarInterp(pts[0].fX, pts[3].fX, PK_Scalar1 * 2 / 3),
                                    SkScalarInterp(pts[0].fY, pts[3].fY, PK_Scalar1 * 2 / 3),
                                    tolerance);
}

class SkContourMeasureIter::Impl {
public:
    Impl(const SkPath& path, bool forceClosed, SkScalar resScale)
            : fPath(path)
            , fIter(SkPathPriv::Iterate(fPath).begin())
            , fTolerance(CHEAP_DIST_LIMIT * PkScalarInvert(resScale))
            , fForceClosed(forceClosed) {}

    bool hasNextSegments() const { return fIter != SkPathPriv::Iterate(fPath).end(); }
    SkContourMeasure* buildSegments();

private:
    SkPath fPath;
    SkPathPriv::RangeIter fIter;
    SkScalar fTolerance;
    bool fForceClosed;

    // temporary
    SkTDArray<SkContourMeasure::Segment> fSegments;
    SkTDArray<SkPoint> fPts;  // Points used to define the segments

    SkScalar compute_line_seg(SkPoint p0, SkPoint p1, SkScalar distance, unsigned ptIndex);
    SkScalar compute_quad_segs(
            const SkPoint pts[3], SkScalar distance, int mint, int maxt, unsigned ptIndex);
    SkScalar compute_conic_segs(const SkConic& conic,
                                SkScalar distance,
                                int mint,
                                const SkPoint& minPt,
                                int maxt,
                                const SkPoint& maxPt,
                                unsigned ptIndex);
    SkScalar compute_cubic_segs(
            const SkPoint pts[4], SkScalar distance, int mint, int maxt, unsigned ptIndex);
};

SkScalar SkContourMeasureIter::Impl::compute_quad_segs(
        const SkPoint pts[3], SkScalar distance, int mint, int maxt, unsigned ptIndex) {
    if (tspan_big_enough(maxt - mint) && quad_too_curvy(pts, fTolerance)) {
        SkPoint tmp[5];
        int halft = (mint + maxt) >> 1;

        SkChopQuadAtHalf(pts, tmp);
        distance = this->compute_quad_segs(tmp, distance, mint, halft, ptIndex);
        distance = this->compute_quad_segs(&tmp[2], distance, halft, maxt, ptIndex);
    } else {
        SkScalar d = SkPoint::Distance(pts[0], pts[2]);
        SkScalar prevD = distance;
        distance += d;
        if (distance > prevD) {
            SkContourMeasure::Segment* seg = fSegments.append();
            seg->fDistance = distance;
            seg->fPtIndex = ptIndex;
            seg->fType = kQuad_SegType;
            seg->fTValue = maxt;
        }
    }
    return distance;
}

SkScalar SkContourMeasureIter::Impl::compute_conic_segs(const SkConic& conic,
                                                        SkScalar distance,
                                                        int mint,
                                                        const SkPoint& minPt,
                                                        int maxt,
                                                        const SkPoint& maxPt,
                                                        unsigned ptIndex) {
    int halft = (mint + maxt) >> 1;
    SkPoint halfPt = conic.evalAt(tValue2Scalar(halft));
    if (!halfPt.isFinite()) {
        return distance;
    }
    if (tspan_big_enough(maxt - mint) && conic_too_curvy(minPt, halfPt, maxPt, fTolerance)) {
        distance = this->compute_conic_segs(conic, distance, mint, minPt, halft, halfPt, ptIndex);
        distance = this->compute_conic_segs(conic, distance, halft, halfPt, maxt, maxPt, ptIndex);
    } else {
        SkScalar d = SkPoint::Distance(minPt, maxPt);
        SkScalar prevD = distance;
        distance += d;
        if (distance > prevD) {
            SkContourMeasure::Segment* seg = fSegments.append();
            seg->fDistance = distance;
            seg->fPtIndex = ptIndex;
            seg->fType = kConic_SegType;
            seg->fTValue = maxt;
        }
    }
    return distance;
}

SkScalar SkContourMeasureIter::Impl::compute_cubic_segs(
        const SkPoint pts[4], SkScalar distance, int mint, int maxt, unsigned ptIndex) {
    if (tspan_big_enough(maxt - mint) && cubic_too_curvy(pts, fTolerance)) {
        SkPoint tmp[7];
        int halft = (mint + maxt) >> 1;

        SkChopCubicAtHalf(pts, tmp);
        distance = this->compute_cubic_segs(tmp, distance, mint, halft, ptIndex);
        distance = this->compute_cubic_segs(&tmp[3], distance, halft, maxt, ptIndex);
    } else {
        SkScalar d = SkPoint::Distance(pts[0], pts[3]);
        SkScalar prevD = distance;
        distance += d;
        if (distance > prevD) {
            SkContourMeasure::Segment* seg = fSegments.append();
            seg->fDistance = distance;
            seg->fPtIndex = ptIndex;
            seg->fType = kCubic_SegType;
            seg->fTValue = maxt;
        }
    }
    return distance;
}

SkScalar SkContourMeasureIter::Impl::compute_line_seg(SkPoint p0,
                                                      SkPoint p1,
                                                      SkScalar distance,
                                                      unsigned ptIndex) {
    SkScalar d = SkPoint::Distance(p0, p1);
    SkScalar prevD = distance;
    distance += d;
    if (distance > prevD) {
        SkContourMeasure::Segment* seg = fSegments.append();
        seg->fDistance = distance;
        seg->fPtIndex = ptIndex;
        seg->fType = kLine_SegType;
        seg->fTValue = kMaxTValue;
    }
    return distance;
}

SkContourMeasure* SkContourMeasureIter::Impl::buildSegments() {
    int ptIndex = -1;
    SkScalar distance = 0;
    bool haveSeenClose = fForceClosed;
    bool haveSeenMoveTo = false;

    /*  Note:
     *  as we accumulate distance, we have to check that the result of +=
     *  actually made it larger, since a very small delta might be > 0, but
     *  still have no effect on distance (if distance >>> delta).
     *
     *  We do this check below, and in compute_quad_segs and compute_cubic_segs
     */

    fSegments.reset();
    fPts.reset();

    auto end = SkPathPriv::Iterate(fPath).end();
    for (; fIter != end; ++fIter) {
        auto verb = std::get<0>(*fIter);
        auto pts = std::get<1>(*fIter);
        auto w = std::get<2>(*fIter);
        if (haveSeenMoveTo && verb == SkPathVerb::kMove) {
            break;
        }
        switch (verb) {
            case SkPathVerb::kMove:
                ptIndex += 1;
                fPts.append(1, pts);
                haveSeenMoveTo = true;
                break;

            case SkPathVerb::kLine: {
                SkScalar prevD = distance;
                distance = this->compute_line_seg(pts[0], pts[1], distance, ptIndex);
                if (distance > prevD) {
                    fPts.append(1, pts + 1);
                    ptIndex++;
                }
            } break;

            case SkPathVerb::kQuad: {
                SkScalar prevD = distance;
                distance = this->compute_quad_segs(pts, distance, 0, kMaxTValue, ptIndex);
                if (distance > prevD) {
                    fPts.append(2, pts + 1);
                    ptIndex += 2;
                }
            } break;

            case SkPathVerb::kConic: {
                const SkConic conic(pts, *w);
                SkScalar prevD = distance;
                distance = this->compute_conic_segs(
                        conic, distance, 0, conic.fPts[0], kMaxTValue, conic.fPts[2], ptIndex);
                if (distance > prevD) {
                    // we store the conic weight in our next point, followed by the last 2 pts
                    // thus to reconstitue a conic, you'd need to say
                    // SkConic(pts[0], pts[2], pts[3], weight = pts[1].fX)
                    fPts.append()->set(conic.fW, 0);
                    fPts.append(2, pts + 1);
                    ptIndex += 3;
                }
            } break;

            case SkPathVerb::kCubic: {
                SkScalar prevD = distance;
                distance = this->compute_cubic_segs(pts, distance, 0, kMaxTValue, ptIndex);
                if (distance > prevD) {
                    fPts.append(3, pts + 1);
                    ptIndex += 3;
                }
            } break;

            case SkPathVerb::kClose:
                haveSeenClose = true;
                break;
        }
    }

    if (!SkScalarIsFinite(distance)) {
        return nullptr;
    }
    if (fSegments.count() == 0) {
        return nullptr;
    }

    if (haveSeenClose) {
        SkScalar prevD = distance;
        SkPoint firstPt = fPts[0];
        distance = this->compute_line_seg(fPts[ptIndex], firstPt, distance, ptIndex);
        if (distance > prevD) {
            *fPts.append() = firstPt;
        }
    }

    return new SkContourMeasure(std::move(fSegments), std::move(fPts), distance, haveSeenClose);
}

static void compute_pos_tan(
        const SkPoint pts[], unsigned segType, SkScalar t, SkPoint* pos, SkVector* tangent) {
    switch (segType) {
        case kLine_SegType:
            if (pos) {
                pos->set(SkScalarInterp(pts[0].fX, pts[1].fX, t),
                         SkScalarInterp(pts[0].fY, pts[1].fY, t));
            }
            if (tangent) {
                tangent->setNormalize(pts[1].fX - pts[0].fX, pts[1].fY - pts[0].fY);
            }
            break;
        case kQuad_SegType:
            SkEvalQuadAt(pts, t, pos, tangent);
            if (tangent) {
                tangent->normalize();
            }
            break;
        case kConic_SegType: {
            SkConic(pts[0], pts[2], pts[3], pts[1].fX).evalAt(t, pos, tangent);
            if (tangent) {
                tangent->normalize();
            }
        } break;
        case kCubic_SegType:
            SkEvalCubicAt(pts, t, pos, tangent, nullptr);
            if (tangent) {
                tangent->normalize();
            }
            break;
        default:
            PkDEBUGFAIL("unknown segType");
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

SkContourMeasureIter::SkContourMeasureIter() {}

SkContourMeasureIter::SkContourMeasureIter(const SkPath& path,
                                           bool forceClosed,
                                           SkScalar resScale) {
    this->reset(path, forceClosed, resScale);
}

SkContourMeasureIter::~SkContourMeasureIter() {}

/** Assign a new path, or null to have none.
 */
void SkContourMeasureIter::reset(const SkPath& path, bool forceClosed, SkScalar resScale) {
    if (path.isFinite()) {
        fImpl = std::make_unique<Impl>(path, forceClosed, resScale);
    } else {
        fImpl.reset();
    }
}

sk_sp<SkContourMeasure> SkContourMeasureIter::next() {
    if (!fImpl) {
        return nullptr;
    }
    while (fImpl->hasNextSegments()) {
        auto cm = fImpl->buildSegments();
        if (cm) {
            return sk_sp<SkContourMeasure>(cm);
        }
    }
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SkContourMeasure::SkContourMeasure(SkTDArray<Segment>&& segs,
                                   SkTDArray<SkPoint>&& pts,
                                   SkScalar length,
                                   bool isClosed)
        : fSegments(std::move(segs)), fPts(std::move(pts)), fLength(length), fIsClosed(isClosed) {}

template <typename T, typename K> int SkTKSearch(const T base[], int count, const K& key) {
    if (count <= 0) {
        return ~0;
    }

    unsigned lo = 0;
    unsigned hi = count - 1;

    while (lo < hi) {
        unsigned mid = (hi + lo) >> 1;
        if (base[mid].fDistance < key) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }

    if (base[hi].fDistance < key) {
        hi += 1;
        hi = ~hi;
    } else if (key < base[hi].fDistance) {
        hi = ~hi;
    }
    return hi;
}

const SkContourMeasure::Segment* SkContourMeasure::distanceToSegment(SkScalar distance,
                                                                     SkScalar* t) const {
    const Segment* seg = fSegments.begin();
    int count = fSegments.count();

    int index = SkTKSearch<Segment, SkScalar>(seg, count, distance);
    // don't care if we hit an exact match or not, so we xor index if it is negative
    index ^= (index >> 31);
    seg = &seg[index];

    // now interpolate t-values with the prev segment (if possible)
    SkScalar startT = 0, startD = 0;
    // check if the prev segment is legal, and references the same set of points
    if (index > 0) {
        startD = seg[-1].fDistance;
        if (seg[-1].fPtIndex == seg->fPtIndex) {
            startT = seg[-1].getScalarT();
        }
    }

    *t = startT + (seg->getScalarT() - startT) * (distance - startD) / (seg->fDistance - startD);
    return seg;
}

bool SkContourMeasure::getPosTan(SkScalar distance, SkPoint* pos, SkVector* tangent) const {
    if (SkScalarIsNaN(distance)) {
        return false;
    }

    const SkScalar length = this->length();

    // pin the distance to a legal range
    if (distance < 0) {
        distance = 0;
    } else if (distance > length) {
        distance = length;
    }

    SkScalar t;
    const Segment* seg = this->distanceToSegment(distance, &t);
    if (SkScalarIsNaN(t)) {
        return false;
    }

    compute_pos_tan(&fPts[seg->fPtIndex], seg->fType, t, pos, tangent);
    return true;
}

bool SkContourMeasure::getMatrix(SkScalar distance, SkMatrix* matrix, MatrixFlags flags) const {
    SkPoint position;
    SkVector tangent;

    if (this->getPosTan(distance, &position, &tangent)) {
        if (matrix) {
            if (flags & kGetTangent_MatrixFlag) {
                matrix->setSinCos(tangent.fY, tangent.fX, 0, 0);
            } else {
                matrix->reset();
            }
            if (flags & kGetPosition_MatrixFlag) {
                matrix->postTranslate(position.fX, position.fY);
            }
        }
        return true;
    }
    return false;
}

bool SkContourMeasure::getSegment(SkScalar startD,
                                  SkScalar stopD,
                                  SkPath* dst,
                                  bool startWithMoveTo) const {
    SkScalar length = this->length();  // ensure we have built our segments

    if (startD < 0) {
        startD = 0;
    }
    if (stopD > length) {
        stopD = length;
    }
    if (!(startD <= stopD)) {  // catch NaN values as well
        return false;
    }
    if (!fSegments.count()) {
        return false;
    }

    SkPoint p;
    SkScalar startT, stopT;
    const Segment* seg = this->distanceToSegment(startD, &startT);
    if (!SkScalarIsFinite(startT)) {
        return false;
    }
    const Segment* stopSeg = this->distanceToSegment(stopD, &stopT);
    if (!SkScalarIsFinite(stopT)) {
        return false;
    }
    if (startWithMoveTo) {
        compute_pos_tan(&fPts[seg->fPtIndex], seg->fType, startT, &p, nullptr);
        dst->moveTo(p);
    }

    if (seg->fPtIndex == stopSeg->fPtIndex) {
        SkContourMeasure_segTo(&fPts[seg->fPtIndex], seg->fType, startT, stopT, dst);
    } else {
        do {
            SkContourMeasure_segTo(&fPts[seg->fPtIndex], seg->fType, startT, PK_Scalar1, dst);
            seg = SkContourMeasure::Segment::Next(seg);
            startT = 0;
        } while (seg->fPtIndex < stopSeg->fPtIndex);
        SkContourMeasure_segTo(&fPts[seg->fPtIndex], seg->fType, 0, stopT, dst);
    }

    return true;
}
}  // namespace pk