# OpenCAD Application

set(CMAKE_AUTOMOC ON)

add_executable(opencad WIN32
    main.cpp
)

target_link_libraries(opencad PRIVATE
    opencad_ui
    opencad_io
    opencad_core
    ${QT_LIBS}
    Qt6::Svg
    Qt6::SvgWidgets
)

add_executable(test_solver
    ../../test_solver.cpp
)
target_link_libraries(test_solver PRIVATE
    opencad_ui
    opencad_io
    opencad_core
    ${QT_LIBS}
    Qt6::Svg
    Qt6::SvgWidgets
)
