cmake_minimum_required(VERSION 3.14)
get_filename_component(PROJECT_FOLDER_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
project(${PROJECT_FOLDER_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/LogMonitorApp.cpp
    src/FileWatcher.cpp
    src/CsvParser.cpp
    src/SerialOutputPrinter.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE include)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
