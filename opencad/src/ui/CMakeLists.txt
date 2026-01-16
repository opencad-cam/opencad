# UI Module - Qt6 based user interface

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(UI_SOURCES
    MainWindow.cpp
    PropertiesPanel.cpp
    ParameterEditor.cpp
    ToolSettingsPanel.cpp
    viewport/Viewport3D.cpp
    viewport/ViewportController.cpp
    sketch/SketchView2D.cpp
    dialogs/SketchPlaneDialog.cpp
    dialogs/ExtrudeDialog.cpp
    dialogs/FilletDialog.cpp
    dialogs/ChamferDialog.cpp
    dialogs/ShellDialog.cpp
    dialogs/PatternDialog.cpp
    dialogs/MirrorDialog.cpp
    dialogs/SweepDialog.cpp
    dialogs/LoftDialog.cpp
    dialogs/SplitDialog.cpp
    dialogs/DomeDialog.cpp
)

set(UI_HEADERS
    MainWindow.h
    PropertiesPanel.h
    ParameterEditor.h
    ToolSettingsPanel.h
    viewport/Viewport3D.h
    viewport/ViewportController.h
    sketch/SketchView2D.h
    dialogs/SketchPlaneDialog.h
    dialogs/ExtrudeDialog.h
    dialogs/FilletDialog.h
    dialogs/ChamferDialog.h
    dialogs/ShellDialog.h
    dialogs/PatternDialog.h
    dialogs/MirrorDialog.h
    dialogs/SweepDialog.h
    dialogs/LoftDialog.h
    dialogs/SplitDialog.h
    dialogs/DomeDialog.h
)

add_library(opencad_ui STATIC ${UI_SOURCES} ${UI_HEADERS})

target_include_directories(opencad_ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(opencad_ui PUBLIC
    opencad_core
    opencad_io
    opencad_sketch
    opencad_part
    ${QT_LIBS}
    OpenGL::GL
    TKV3d
    TKService
    TKOpenGl
)


