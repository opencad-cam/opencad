
# src/ai/CMakeLists.txt

set(AI_SOURCES
    CadQueryClient.cpp
    CadQueryClient.h
    LLMClient.cpp
    LLMClient.h
)

add_library(opencad_ai STATIC ${AI_SOURCES})

target_include_directories(opencad_ai PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(opencad_ai
    PUBLIC
    ${QT_LIBS}
    TKV3d
    TKService
    TKMath
    TKernel
    TKG3d
    TKGeomBase
    TKBRep
    TKTopAlgo
    TKMesh
    opencad_core
)
