# I/O Module - File format handlers

set(IO_SOURCES
    step/StepReader.cpp
    step/StepWriter.cpp
    mesh/StlWriter.cpp
    mesh/StlReader.cpp
    iges/IgesReader.cpp
    brep/BRepReader.cpp
)

set(IO_HEADERS
    step/StepReader.h
    step/StepWriter.h
    mesh/StlWriter.h
    mesh/StlReader.h
    iges/IgesReader.h
    brep/BRepReader.h
)

add_library(opencad_io STATIC ${IO_SOURCES} ${IO_HEADERS})

target_include_directories(opencad_io PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(opencad_io PUBLIC
    opencad_core
    TKXSBase
    TKDESTEP   # OCCT 7.9.3: was TKSTEP
    TKDEIGES   # OCCT 7.9.3: was TKIGES  
    TKDESTL    # OCCT 7.9.3: was TKSTL
    TKDE
)
