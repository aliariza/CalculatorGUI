cmake_minimum_required(VERSION 3.20)
project(CalculatorGUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt")

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(CalculatorGUI
    src/main.cpp
    src/calculator_widget.cpp
    include/calculator_widget.h
    src/calculator.cpp
    include/calculator.h
)

target_include_directories(CalculatorGUI PRIVATE include)
target_link_libraries(CalculatorGUI PRIVATE Qt6::Widgets)

set_target_properties(CalculatorGUI PROPERTIES
    MACOSX_BUNDLE TRUE
)
