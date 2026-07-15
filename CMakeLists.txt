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
  set(RAYLIB_VERSION "6.0")

  if(NOT DEFINED RAYLIB_ROOT)
    if(EXISTS "${CMAKE_SOURCE_DIR}/lib/raylib-${RAYLIB_VERSION}_linux_amd64")
      set(RAYLIB_ROOT "${CMAKE_SOURCE_DIR}/lib/raylib-${RAYLIB_VERSION}_linux_amd64")
    else()
      set(RAYLIB_ROOT "${CMAKE_SOURCE_DIR}/raylib")
    endif()
  endif()

  set(RAYLIB_ROOT
    "${RAYLIB_ROOT}"
    CACHE PATH "Path to the local prebuilt raylib 6.0 package"
  )

  if(WIN32 AND MSVC)
    set(RAYLIB_PACKAGE_URL
      "https://github.com/raysan5/raylib/releases/download/${RAYLIB_VERSION}/raylib-${RAYLIB_VERSION}_win64_msvc16.zip"
    )
    set(RAYLIB_LIBRARY_NAME "raylib.lib")
  elseif(APPLE)
    set(RAYLIB_PACKAGE_URL
      "https://github.com/raysan5/raylib/releases/download/${RAYLIB_VERSION}/raylib-${RAYLIB_VERSION}_macos.tar.gz"
    )
    set(RAYLIB_LIBRARY_NAME "libraylib.a")
  else()
    set(RAYLIB_PACKAGE_URL
      "https://github.com/raysan5/raylib/releases/download/${RAYLIB_VERSION}/raylib-${RAYLIB_VERSION}_linux_amd64.tar.gz"
    )
    set(RAYLIB_LIBRARY_NAME "libraylib.a")
  endif()

  set(RAYLIB_INCLUDE_DIR "${RAYLIB_ROOT}/include")
  set(RAYLIB_LIBRARY "${RAYLIB_ROOT}/lib/${RAYLIB_LIBRARY_NAME}")

  if(NOT EXISTS "${RAYLIB_INCLUDE_DIR}/raylib.h" OR NOT EXISTS "${RAYLIB_LIBRARY}")
    message(STATUS "Could not find local raylib package, downloading ${RAYLIB_PACKAGE_URL}")

    get_filename_component(RAYLIB_PACKAGE_ARCHIVE
      "${RAYLIB_PACKAGE_URL}"
      NAME
    )
    set(RAYLIB_PACKAGE_ARCHIVE "${CMAKE_BINARY_DIR}/_deps/${RAYLIB_PACKAGE_ARCHIVE}")

    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/_deps")
    file(DOWNLOAD
      "${RAYLIB_PACKAGE_URL}"
      "${RAYLIB_PACKAGE_ARCHIVE}"
      STATUS RAYLIB_DOWNLOAD_STATUS
      SHOW_PROGRESS
    )
    list(GET RAYLIB_DOWNLOAD_STATUS 0 RAYLIB_DOWNLOAD_CODE)
    list(GET RAYLIB_DOWNLOAD_STATUS 1 RAYLIB_DOWNLOAD_MESSAGE)
    if(NOT RAYLIB_DOWNLOAD_CODE EQUAL 0)
      message(FATAL_ERROR
        "Failed to download raylib package: ${RAYLIB_DOWNLOAD_MESSAGE}\n"
        "You can also download it manually and configure with:\n"
        "  -DRAYLIB_ROOT=/path/to/raylib-${RAYLIB_VERSION}_linux_amd64"
      )
    endif()

    file(MAKE_DIRECTORY "${RAYLIB_ROOT}")
    file(ARCHIVE_EXTRACT
      INPUT "${RAYLIB_PACKAGE_ARCHIVE}"
      DESTINATION "${RAYLIB_ROOT}"
    )
  endif()

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
        m
        X11
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
