# Tests

enable_testing()

add_executable(test_geometry
    test_geometry.cpp
)

target_link_libraries(test_geometry PRIVATE
    opencad_core
)

add_test(NAME GeometryTests COMMAND test_geometry)

add_executable(test_extrude
    test_extrude.cpp
)

target_link_libraries(test_extrude PRIVATE
    opencad_core
    opencad_part
    opencad_sketch
)

add_test(NAME ExtrudeTests COMMAND test_extrude)
