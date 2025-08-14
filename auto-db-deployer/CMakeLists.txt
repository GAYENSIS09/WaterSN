cmake_minimum_required(VERSION 3.10)

project(AutoDBDeployer)

set(CMAKE_CXX_STANDARD 14)

# Include directories
include_directories(src)

# Source files
set(SOURCES
    src/main.cpp
    src/database/dbmanager.cpp
    src/platform/linux.cpp
    src/platform/windows.cpp
    src/platform/macos.cpp
    src/utils/fileutils.cpp
)

# Add executable
add_executable(AutoDBDeployer ${SOURCES})

# Optionally link libraries if needed
# target_link_libraries(AutoDBDeployer <libraries>)