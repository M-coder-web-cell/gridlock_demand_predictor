cmake_minimum_required(VERSION 3.16)
project(NexusDB VERSION 1.0.0 LANGUAGES CXX)

# ── C++17 standard (required for <filesystem>) ──────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Source files ─────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/Row.cpp
    src/Table.cpp
    src/NexusManager.cpp
    src/CommandParser.cpp
)

# ── Executable ───────────────────────────────────────────────────
add_executable(nexusdb ${SOURCES})

# ── Include directory ────────────────────────────────────────────
target_include_directories(nexusdb PRIVATE ${CMAKE_SOURCE_DIR}/src)

# ── Compiler warnings ───────────────────────────────────────────
if(MSVC)
    target_compile_options(nexusdb PRIVATE /W4)
else()
    target_compile_options(nexusdb PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ── Create data directory at build time ──────────────────────────
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data)
