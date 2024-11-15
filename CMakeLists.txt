cmake_minimum_required(VERSION 3.16)
project(PathKit)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
    src/core/SkMalloc.cpp
    src/core/SkMath.cpp
    src/core/SkCubicClipper.cpp
    src/core/SkArenaAlloc.cpp
    src/core/SkPaintPriv.cpp
    src/core/SkPaint.cpp
    src/core/SkPoint.cpp
    src/core/SkContourMeasure.cpp
    src/core/SkMatrix.cpp
    src/core/SkGeometry.cpp
    src/core/SkPathMeasure.cpp
    src/core/SkPath.cpp
    src/core/SkPathEffect.cpp
    src/core/SkPathRef.cpp
    src/core/SkRRect.cpp
    src/core/SkPathBuilder.cpp
    src/core/SkRect.cpp
    src/core/SkStrokeRec.cpp
    src/core/SkStrokerPriv.cpp
    src/core/SkStroke.cpp

    src/gpu/geometry/GrPathUtils.cpp
    src/gpu/geometry/GrAATriangulator.cpp
    src/gpu/geometry/GrTriangulator.cpp

    src/effects/SkCornerPathEffect.cpp
    src/effects/SkDashPathEffect.cpp

    src/pathops/SkDConicLineIntersection.cpp
    src/pathops/SkAddIntersections.cpp
    src/pathops/SkDCubicToQuads.cpp
    src/pathops/SkDCubicLineIntersection.cpp
    src/pathops/SkDLineIntersection.cpp
    src/pathops/SkDQuadLineIntersection.cpp
    src/pathops/SkIntersections.cpp
    src/pathops/SkOpAngle.cpp
    src/pathops/SkOpContour.cpp
    src/pathops/SkOpCubicHull.cpp
    src/pathops/SkOpBuilder.cpp
    src/pathops/SkOpCoincidence.cpp
    src/pathops/SkOpEdgeBuilder.cpp
    src/pathops/SkOpSpan.cpp
    src/pathops/SkOpSegment.cpp
    src/pathops/SkPathOpsCommon.cpp
    src/pathops/SkPathOpsAsWinding.cpp
    src/pathops/SkPathOpsConic.cpp
    src/pathops/SkPathOpsCurve.cpp
    src/pathops/SkPathOpsDebug.cpp
    src/pathops/SkPathOpsCubic.cpp
    src/pathops/SkPathOpsLine.cpp
    src/pathops/SkPathOpsQuad.cpp
    src/pathops/SkPathOpsOp.cpp
    src/pathops/SkPathOpsRect.cpp
    src/ports/SkDebug_stdio.cpp
    src/ports/SkMemory_malloc.cpp
    src/pathops/SkPathOpsSimplify.cpp
    src/pathops/SkPathOpsTightBounds.cpp
    src/pathops/SkPathOpsTypes.cpp
    src/pathops/SkPathOpsTSect.cpp
    src/pathops/SkReduceOrder.cpp
    src/pathops/SkPathOpsWinding.cpp
    src/pathops/SkPathWriter.cpp

    src/utils/SkDashPath.cpp
)

add_library(pathkit STATIC ${SOURCE_FILES})
target_compile_options(pathkit PRIVATE
    -Wno-unused-function
    -Wno-unused-variable
    )
include_directories(./)
add_executable(PathKitDemo main.cpp)
target_link_libraries(PathKitDemo pathkit)
