cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
  set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
  set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(EclipseMenu VERSION 1.0.0 LANGUAGES CXX)

set(MACOS_SOURCES "")
if (APPLE AND NOT IOS AND NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "iOS")
  enable_language(OBJCXX)
  set(MACOS_SOURCES src/modules/keybinds/mouse/macos.mm src/modules/keybinds/keyboard/macos.mm)
endif()

# Sources
file(GLOB_RECURSE ALL_SOURCES src/*.cpp)

# Filter out iOS-excluded files when building for iOS
set(IOS_EXCLUDE_FILES
  src/hacks/Bot/Bot.cpp
  src/hacks/Bypass/AllowLowVolume.cpp
  src/hacks/Bypass/UnlockVaults.cpp
  src/hacks/Creator/DefaultSongBypass.cpp
  src/hacks/Creator/LevelEdit.cpp
  src/hacks/Creator/HideTriggers.cpp
  src/hacks/Creator/SliderLimit.cpp
  src/hacks/Global/CompactViews.cpp
  src/hacks/Level/HitboxMultiplier.cpp
)
if (IOS OR "${CMAKE_SYSTEM_NAME}" STREQUAL "iOS")
  foreach(EXCLUDE_FILE ${IOS_EXCLUDE_FILES})
    list(REMOVE_ITEM ALL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${EXCLUDE_FILE})
  endforeach()
endif()

set(SOURCES ${ALL_SOURCES})
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${MACOS_SOURCES})

# Configure for GitHub Actions builds
if (DEFINED ENV{GITHUB_ACTIONS})
  # Enable LTO for smaller binaries
  set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  target_compile_definitions(${PROJECT_NAME} PRIVATE GITHUB_ACTIONS)
  message(STATUS "GitHub Actions build detected")
  set(ECLIPSE_DISABLE_INCREMENTAL_LINK ON)
endif()

# Make sure we can include headers from src
target_include_directories(${PROJECT_NAME} PRIVATE src include)

# Get the latest abbreviated commit hash of the working branch
execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
target_compile_definitions(${PROJECT_NAME} PRIVATE GIT_HASH="${GIT_HASH}")

# Windows specific settings
if (WIN32)
  # Disable annoying warnings
  target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)

  # Export all symbols for better debugging
  # UPD: this bloats the binary size, so it's disabled for now
  # (also it doesn't work for every crashlog anyways)
  # set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

  # Disable incremental linking if requested
  if (DEFINED ECLIPSE_DISABLE_INCREMENTAL_LINK)
    message(STATUS "Disabling incremental linking")
    if (MSVC)
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL:NO")
      set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /INCREMENTAL:NO")
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-incremental:no")
      set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-incremental:no")
    endif()
  endif()
endif()

# macOS specific settings
if (APPLE AND NOT IOS AND NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "iOS")
  set_source_files_properties(${MACOS_SOURCES} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endif()

# Debug macro
if (NOT DEFINED ENV{GITHUB_ACTIONS})
  target_compile_definitions(
    ${PROJECT_NAME} PRIVATE
      $<$<CONFIG:Debug>:ECLIPSE_DEBUG_BUILD>
      $<$<CONFIG:RelWithDebInfo>:ECLIPSE_DEBUG_BUILD>
  )
endif()

# Add Geode
if (NOT DEFINED ENV{GEODE_SDK})
  message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
  message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()
add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# Libraries
include(${CMAKE_SOURCE_DIR}/cmake/IncludeLibs.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/Translations.cmake)
target_link_libraries(${PROJECT_NAME} third_party)

# Setup Geode
setup_geode_mod(${PROJECT_NAME})
