# I/O Module - File format handlers

set(IO_SOURCES
    step/StepReader.cpp
    step/StepWriter.cpp
    mesh/StlWriter.cpp
    mesh/StlReader.cpp
    iges/IgesReader.cpp
    brep/BRepReader.cpp
    parasolid/ParasolidReader.cpp
    solidworks/SolidWorksReader.cpp
    solidworks/SolidWorksConverter.cpp
)

set(IO_HEADERS
    step/StepReader.h
    step/StepWriter.h
    mesh/StlWriter.h
    mesh/StlReader.h
    iges/IgesReader.h
    brep/BRepReader.h
    parasolid/ParasolidReader.h
    solidworks/SolidWorksReader.h
    solidworks/SolidWorksConverter.h
)

add_library(opencad_io STATIC ${IO_SOURCES} ${IO_HEADERS})

target_include_directories(opencad_io PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${OpenCASCADE_INCLUDE_DIR}
)

target_link_libraries(opencad_io PUBLIC
    opencad_core
    TKXSBase
    TKDESTEP   # OCCT 7.9.3: was TKSTEP
    TKDEIGES   # OCCT 7.9.3: was TKIGES  
    TKDESTL    # OCCT 7.9.3: was TKSTL
    TKDE
    ZLIB::ZLIB
    ole32      # COM
    oleaut32   # COM automation
    uuid       # COM IIDs
)

find_package(ZLIB REQUIRED)
