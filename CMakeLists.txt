cmake_minimum_required(VERSION 3.24)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Replace with your actual project name
set(PROJECT_NAME "bruecken_spiel")

project(${PROJECT_NAME}
    VERSION 0.1.0
    DESCRIPTION "Brücken - strategisches Brettspiel (APP SoSe 2026)"
    LANGUAGES CXX
)

# Generate compile_commands.json for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -------------------------------------------------------------------
# Dependencies
# -------------------------------------------------------------------

# Preset library (bundled)
add_subdirectory(lib/preset)

# Raylib 6.0
find_package(raylib 6.0 REQUIRED)

# -------------------------------------------------------------------
# Project executable
# -------------------------------------------------------------------

add_executable(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
  PUBLIC
    include
)

# Collect source files
file(GLOB_RECURSE PROJECT_SOURCES
    "src/*.cpp"
)

target_sources(${PROJECT_NAME}
  PRIVATE
    ${PROJECT_SOURCES}
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    preset
    raylib
)
