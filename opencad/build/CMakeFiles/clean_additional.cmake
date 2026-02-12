# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "src\\ai\\CMakeFiles\\opencad_ai_autogen.dir\\AutogenUsed.txt"
  "src\\ai\\CMakeFiles\\opencad_ai_autogen.dir\\ParseCache.txt"
  "src\\ai\\opencad_ai_autogen"
  "src\\app\\CMakeFiles\\opencad_autogen.dir\\AutogenUsed.txt"
  "src\\app\\CMakeFiles\\opencad_autogen.dir\\ParseCache.txt"
  "src\\app\\opencad_autogen"
  "src\\assembly\\CMakeFiles\\opencad_assembly_autogen.dir\\AutogenUsed.txt"
  "src\\assembly\\CMakeFiles\\opencad_assembly_autogen.dir\\ParseCache.txt"
  "src\\assembly\\opencad_assembly_autogen"
  "src\\core\\CMakeFiles\\opencad_core_autogen.dir\\AutogenUsed.txt"
  "src\\core\\CMakeFiles\\opencad_core_autogen.dir\\ParseCache.txt"
  "src\\core\\opencad_core_autogen"
  "src\\io\\CMakeFiles\\opencad_io_autogen.dir\\AutogenUsed.txt"
  "src\\io\\CMakeFiles\\opencad_io_autogen.dir\\ParseCache.txt"
  "src\\io\\opencad_io_autogen"
  "src\\part\\CMakeFiles\\opencad_part_autogen.dir\\AutogenUsed.txt"
  "src\\part\\CMakeFiles\\opencad_part_autogen.dir\\ParseCache.txt"
  "src\\part\\opencad_part_autogen"
  "src\\sketch\\CMakeFiles\\opencad_sketch_autogen.dir\\AutogenUsed.txt"
  "src\\sketch\\CMakeFiles\\opencad_sketch_autogen.dir\\ParseCache.txt"
  "src\\sketch\\opencad_sketch_autogen"
  "src\\ui\\CMakeFiles\\opencad_ui_autogen.dir\\AutogenUsed.txt"
  "src\\ui\\CMakeFiles\\opencad_ui_autogen.dir\\ParseCache.txt"
  "src\\ui\\opencad_ui_autogen"
  "tests\\CMakeFiles\\test_extrude_autogen.dir\\AutogenUsed.txt"
  "tests\\CMakeFiles\\test_extrude_autogen.dir\\ParseCache.txt"
  "tests\\CMakeFiles\\test_geometry_autogen.dir\\AutogenUsed.txt"
  "tests\\CMakeFiles\\test_geometry_autogen.dir\\ParseCache.txt"
  "tests\\test_extrude_autogen"
  "tests\\test_geometry_autogen"
  )
endif()
