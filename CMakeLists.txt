cmake_minimum_required(VERSION 3.21)
project(BinaryInspector VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

add_executable(BinaryInspector WIN32
    src/main.cpp
    src/core/FileReader.cpp
    src/core/FileReader.h
    src/core/Analysis.cpp
    src/core/Analysis.h
    src/search/PatternSearch.cpp
    src/search/PatternSearch.h
    src/ui/HexView.cpp
    src/ui/HexView.h
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
    src/plugins/IAnalyzerPlugin.h
    src/plugins/PluginManager.cpp
    src/plugins/PluginManager.h
)

target_link_libraries(BinaryInspector PRIVATE
    Qt6::Core
    Qt6::Widgets
)

target_include_directories(BinaryInspector PRIVATE src)

if(MSVC)
    target_compile_options(BinaryInspector PRIVATE /W4 /permissive-)
else()
    target_compile_options(BinaryInspector PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install executable
install(TARGETS BinaryInspector
    RUNTIME DESTINATION bin
)

# Qt deployment - Windows
if(WIN32)
    add_custom_command(TARGET BinaryInspector POST_BUILD
        COMMAND "${Qt6_DIR}/../../../bin/windeployqt.exe"
                --release
                "$<TARGET_FILE:BinaryInspector>"
        COMMENT "Deploying Qt dependencies..."
    )
endif()