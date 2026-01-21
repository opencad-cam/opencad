# Tests

enable_testing()

add_executable(test_geometry
    test_geometry.cpp
)

target_link_libraries(test_geometry PRIVATE
    opencad_core
)

add_test(NAME GeometryTests COMMAND test_geometry)
