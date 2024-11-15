#include "src/core/SkPaint.h"
#include "src/core/SkPath.h"
#include "src/core/SkStrokeRec.h"
#include "src/effects/SkDashPathEffect.h"
#include "src/pathops/SkPathOps.h"

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
