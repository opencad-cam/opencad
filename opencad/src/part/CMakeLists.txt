# Part Module - 3D part features (Extrude, Cut, etc.)

set(PART_SOURCES
    WireBuilder.cpp
    ExtrudeFeature.cpp
    RevolveFeature.cpp
    CutFeature.cpp
    FilletFeature.cpp
    ChamferFeature.cpp
    ShellFeature.cpp
    SweepFeature.cpp
    LoftFeature.cpp
    PatternFeature.cpp
    MirrorFeature.cpp
    DraftFeature.cpp
    RibFeature.cpp
    OffsetSurfaceFeature.cpp
    ThickenFeature.cpp
    ScaleFeature.cpp
    HoleFeature.cpp
    SplitFeature.cpp
    DomeFeature.cpp
)

set(PART_HEADERS
    WireBuilder.h
    ExtrudeFeature.h
    RevolveFeature.h
    CutFeature.h
    FilletFeature.h
    ChamferFeature.h
    ShellFeature.h
    SweepFeature.h
    LoftFeature.h
    PatternFeature.h
    MirrorFeature.h
    DraftFeature.h
    RibFeature.h
    OffsetSurfaceFeature.h
    ThickenFeature.h
    ScaleFeature.h
    HoleFeature.h
    SplitFeature.h
    DomeFeature.h
)

add_library(opencad_part STATIC ${PART_SOURCES} ${PART_HEADERS})

target_include_directories(opencad_part PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(opencad_part PUBLIC
    opencad_sketch
    TKernel
    TKMath
    TKBRep
    TKGeomBase
    TKGeomAlgo
    TKTopAlgo
    TKPrim
    TKBO
    TKBool
    TKShHealing
    TKOffset
)

