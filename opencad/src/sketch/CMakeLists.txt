# Sketch Module - 2D Sketching and Constraints

set(SKETCH_SOURCES
    Sketch.cpp
    SketchPlane.cpp
    SketchUndoRedo.cpp
    SketchTrimExtend.cpp
    SketchMirror.cpp
    SketchPattern.cpp
    entities/SketchEntity.cpp
    entities/SketchPoint.cpp
    entities/SketchLine.cpp
    entities/SketchArc.cpp
    entities/SketchCircle.cpp
    entities/SketchRectangle.cpp
    entities/SketchSpline.cpp
    entities/SketchEllipse.cpp
    entities/SketchSlot.cpp
    entities/SketchPolygon.cpp
    constraints/Constraint.cpp
    constraints/HorizontalConstraint.cpp
    constraints/VerticalConstraint.cpp
    constraints/CoincidentConstraint.cpp
    constraints/TangentConstraint.cpp
    constraints/PerpendicularConstraint.cpp
    constraints/ParallelConstraint.cpp
    constraints/EqualConstraint.cpp
    constraints/FixConstraint.cpp
    constraints/DimensionConstraint.cpp
    constraints/ConcentricConstraint.cpp
    solver/ConstraintSolver.cpp
)

set(SKETCH_HEADERS
    Sketch.h
    SketchPlane.h
    SketchUndoRedo.h
    SketchTrimExtend.h
    SketchMirror.h
    SketchPattern.h
    entities/SketchEntity.h
    entities/SketchPoint.h
    entities/SketchLine.h
    entities/SketchArc.h
    entities/SketchCircle.h
    entities/SketchRectangle.h
    entities/SketchSpline.h
    entities/SketchEllipse.h
    entities/SketchSlot.h
    entities/SketchPolygon.h
    constraints/Constraint.h
    constraints/HorizontalConstraint.h
    constraints/VerticalConstraint.h
    constraints/CoincidentConstraint.h
    constraints/TangentConstraint.h
    constraints/PerpendicularConstraint.h
    constraints/ParallelConstraint.h
    constraints/EqualConstraint.h
    constraints/FixConstraint.h
    constraints/DimensionConstraint.h
    constraints/ConcentricConstraint.h
    solver/ConstraintSolver.h
)

add_library(opencad_sketch STATIC ${SKETCH_SOURCES} ${SKETCH_HEADERS})

target_include_directories(opencad_sketch PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCASCADE_INCLUDE_DIR}
)

target_link_libraries(opencad_sketch PUBLIC
    opencad_core
    TKernel
    TKMath
    TKG2d
    TKG3d
    TKGeomBase
    TKGeomAlgo
    TKBRep
    TKTopAlgo
    Eigen3::Eigen
)
