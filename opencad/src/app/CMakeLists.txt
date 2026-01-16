# OpenCAD Application

set(CMAKE_AUTOMOC ON)

add_executable(opencad
    main.cpp
)

target_link_libraries(opencad PRIVATE
    opencad_ui
    opencad_io
    opencad_core
    ${QT_LIBS}
)
