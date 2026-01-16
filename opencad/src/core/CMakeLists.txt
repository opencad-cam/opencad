# Core Geometry Module
# OpenCASCADE wrapper for B-Rep modeling

set(CMAKE_AUTOMOC ON)

set(CORE_SOURCES
    geometry/Shape.cpp
    geometry/Primitives.cpp
    geometry/BooleanOps.cpp
    geometry/Transform.cpp
    Parameter.cpp
    UndoRedoManager.cpp
    Feature.cpp
    FeatureTree.cpp
    Document.cpp
)

set(CORE_HEADERS
    geometry/Shape.h
    geometry/Primitives.h
    geometry/BooleanOps.h
    geometry/Transform.h
    Parameter.h
    UndoRedoManager.h
    Feature.h
    FeatureTree.h
    Document.h
)

add_library(opencad_core STATIC ${CORE_SOURCES} ${CORE_HEADERS})

target_include_directories(opencad_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCASCADE_INCLUDE_DIR}
)

# Link OpenCASCADE libraries
target_link_libraries(opencad_core PUBLIC
    TKernel
    TKMath
    TKG3d
    TKBRep
    TKTopAlgo
    TKPrim
    TKBO
    TKShHealing
    Qt6::Core
)
