#include "pathkit/core/SkPaint.h"
#include "pathkit/core/SkPath.h"
#include "pathkit/core/SkStrokeRec.h"
#include "pathkit/effects/SkDashPathEffect.h"
#include "pathkit/pathops/SkPathOps.h"

using namespace pk;

int main() {
    SkPath skPath{};
    skPath.addRect(SkRect::MakeXYWH(100, 100, 100, 100));
    Op(skPath, skPath, SkPathOp::kUnion_SkPathOp, &skPath);
    float intervals[] = {1, 2, 3, 4};
    auto dashEffect = SkDashPathEffect::Make(intervals, 4, 1);
    SkStrokeRec rec(SkStrokeRec::kHairline_InitStyle);
    dashEffect->filterPath(&skPath, skPath, &rec, nullptr);
    return 0;
}
