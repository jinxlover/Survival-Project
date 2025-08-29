cmake_minimum_required(VERSION 3.10)
project(SurvivalProject)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add include directory
include_directories(include)

# Collect source files
file(GLOB SOURCES "src/*.cpp")

# Define the executable
add_executable(survival_project ${SOURCES})

install(TARGETS survival_project RUNTIME DESTINATION bin)
