# 1. Set the minimum version of CMake required
cmake_minimum_required(VERSION 3.15)

# 2. Define your project name and language
project(project LANGUAGES CXX)

# 3. Require modern C++ standards (C++17 or C++20)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 4. Include directories (tells CMake where your .h files are)
include_directories(include)

# 5. Gather all your source files
set(SOURCE_FILES
    main.cpp
)

# 6. Define the executable target name and its source files
add_executable(project ${SOURCE_FILES})

# 7. Windows Specific: Link the Winsock library automatically
if(WIN32)
    target_link_libraries(project ws2_32)
endif()