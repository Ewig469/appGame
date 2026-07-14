cmake_minimum_required(VERSION 3.24)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

set(PROJECT_NAME "preset")

project(${PROJECT_NAME})

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} 
  PUBLIC
    include
)

target_sources(${PROJECT_NAME} 
  PRIVATE
    src/argument_parser.cpp
    src/logger.cpp
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
