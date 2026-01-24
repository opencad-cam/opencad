# Assembly Module - Assembly of parts and constraint solving

set(ASSEMBLY_SOURCES
    Assembly.cpp
    Component.cpp
    AssemblyConstraint.cpp
)

set(ASSEMBLY_HEADERS
    Assembly.h
    Component.h
    AssemblyConstraint.h
)

add_library(opencad_assembly STATIC ${ASSEMBLY_SOURCES} ${ASSEMBLY_HEADERS})

target_include_directories(opencad_assembly PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(opencad_assembly PUBLIC
    opencad_core
    TKernel
    TKMath
)
