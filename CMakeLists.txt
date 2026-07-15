cmake_minimum_required(VERSION 3.24)

set(PROJECT_NAME "knight_bridge")

project(${PROJECT_NAME}
  VERSION 0.1.0
  DESCRIPTION "Knight Bridge - strategic board game (APP SoSe 2026)"
  LANGUAGES C CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

function(enable_bruecken_warnings target_name)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${target_name}
      PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
    )
  endif()
endfunction()

# -------------------------------------------------------------------
# External libraries
# -------------------------------------------------------------------
add_subdirectory(lib/preset)

option(
  BRUECKEN_USE_BUNDLED_RAYLIB
  "Build the raylib source package in lib/raylib/raylib-6.0 instead of using a local prebuilt raylib package"
  OFF
)

if(BRUECKEN_USE_BUNDLED_RAYLIB)
  set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  add_subdirectory(lib/raylib/raylib-6.0)
else()
  set(RAYLIB_ROOT
    "${CMAKE_SOURCE_DIR}/lib/raylib-6.0_linux_amd64"
    CACHE PATH "Path to the local prebuilt raylib 6.0 package"
  )

  if(WIN32 AND MSVC)
    set(RAYLIB_LIBRARY_NAME "raylib.lib")
  else()
    set(RAYLIB_LIBRARY_NAME "libraylib.a")
  endif()

  set(RAYLIB_INCLUDE_DIR "${RAYLIB_ROOT}/include")
  set(RAYLIB_LIBRARY "${RAYLIB_ROOT}/lib/${RAYLIB_LIBRARY_NAME}")

  if(NOT EXISTS "${RAYLIB_INCLUDE_DIR}/raylib.h" OR NOT EXISTS "${RAYLIB_LIBRARY}")
    file(GLOB RAYLIB_NESTED_HEADERS
      LIST_DIRECTORIES FALSE
      "${RAYLIB_ROOT}/*/include/raylib.h"
    )

    foreach(RAYLIB_NESTED_HEADER IN LISTS RAYLIB_NESTED_HEADERS)
      get_filename_component(RAYLIB_NESTED_INCLUDE_DIR
        "${RAYLIB_NESTED_HEADER}"
        DIRECTORY
      )
      get_filename_component(RAYLIB_NESTED_ROOT
        "${RAYLIB_NESTED_INCLUDE_DIR}"
        DIRECTORY
      )
      set(RAYLIB_NESTED_LIBRARY
        "${RAYLIB_NESTED_ROOT}/lib/${RAYLIB_LIBRARY_NAME}"
      )

      if(EXISTS "${RAYLIB_NESTED_LIBRARY}")
        set(RAYLIB_ROOT "${RAYLIB_NESTED_ROOT}")
        set(RAYLIB_INCLUDE_DIR "${RAYLIB_NESTED_INCLUDE_DIR}")
        set(RAYLIB_LIBRARY "${RAYLIB_NESTED_LIBRARY}")
        break()
      endif()
    endforeach()
  endif()

  if(NOT EXISTS "${RAYLIB_INCLUDE_DIR}/raylib.h" OR NOT EXISTS "${RAYLIB_LIBRARY}")
    message(FATAL_ERROR
      "Could not find the local prebuilt raylib package.\n"
      "Expected files:\n"
      "  ${RAYLIB_INCLUDE_DIR}/raylib.h\n"
      "  ${RAYLIB_LIBRARY}\n"
      "Put raylib here or configure with:\n"
      "  -DRAYLIB_ROOT=/path/to/raylib-6.0_linux_amd64"
    )
  endif()

  message(STATUS "Using local prebuilt raylib: ${RAYLIB_ROOT}")

  add_library(raylib STATIC IMPORTED)
  set_target_properties(raylib PROPERTIES
    IMPORTED_LOCATION "${RAYLIB_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${RAYLIB_INCLUDE_DIR}"
  )

  if(WIN32)
    target_link_libraries(raylib
      INTERFACE
        winmm
        gdi32
        opengl32
    )
  elseif(APPLE)
    target_link_libraries(raylib
      INTERFACE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework OpenGL"
    )
  elseif(UNIX)
    target_link_libraries(raylib
      INTERFACE
        X11
        GL
        m
        pthread
        dl
        rt
    )
  endif()
endif()

# -------------------------------------------------------------------
# Project library shared by executable and tests
# -------------------------------------------------------------------
add_library(bruecken_core)

target_include_directories(bruecken_core
  PUBLIC
    include
)

target_sources(bruecken_core
  PRIVATE
    src/board.cpp
    src/gui_logic.cpp
    src/human_player.cpp
    src/random_ai_player.cpp
)

target_link_libraries(bruecken_core
  PUBLIC
    preset
)

target_compile_features(bruecken_core PUBLIC cxx_std_20)
enable_bruecken_warnings(bruecken_core)

# -------------------------------------------------------------------
# Game executable
# -------------------------------------------------------------------
add_executable(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
  PRIVATE
    include
)

target_sources(${PROJECT_NAME}
  PRIVATE
    src/main.cpp
    src/gui_renderer.cpp
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    bruecken_core
    raylib
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
enable_bruecken_warnings(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME}
  PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
)

# -------------------------------------------------------------------
# Tests
# -------------------------------------------------------------------
enable_testing()

add_executable(board_tests)
target_sources(board_tests PRIVATE tests/board_tests.cpp)
target_link_libraries(board_tests PRIVATE bruecken_core)
target_compile_features(board_tests PRIVATE cxx_std_20)
enable_bruecken_warnings(board_tests)

add_executable(test_player)
target_sources(test_player PRIVATE tests/test_player.cpp)
target_link_libraries(test_player PRIVATE bruecken_core)
target_compile_features(test_player PRIVATE cxx_std_20)
enable_bruecken_warnings(test_player)

add_executable(test_gui_logic)
target_sources(test_gui_logic PRIVATE tests/test_gui_logic.cpp)
target_link_libraries(test_gui_logic PRIVATE bruecken_core)
target_compile_features(test_gui_logic PRIVATE cxx_std_20)
enable_bruecken_warnings(test_gui_logic)

add_test(NAME board_rules COMMAND board_tests)
add_test(NAME player_contract COMMAND test_player)
add_test(NAME gui_logic_contract COMMAND test_gui_logic)

if(MINGW)
  foreach(target_name
      ${PROJECT_NAME}
      board_tests
      test_player
      test_gui_logic)
    target_link_options(${target_name} PRIVATE -static)
  endforeach()
endif()
