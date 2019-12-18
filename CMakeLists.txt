cmake_minimum_required (VERSION 3.7)

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set (CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Werror")

#set (CMAKE_VERBOSE_MAKEFILE ON)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")

project(cellular_automaton)

find_package (SDL2 REQUIRED)
find_package (SDL2_ttf REQUIRED)

add_subdirectory(src)

set (SOURCES
  main.cc
  )

add_executable(cellular_automaton
  ${SOURCES}
  )

target_include_directories (cellular_automaton PRIVATE
  )

target_link_libraries(cellular_automaton
  core_utils
  sdl_engine
  sdl_core
  sdl_graphic
  sdl_app_core
  cellular_automaton_lib
  ${SDL2_LIBRARIES}
  ${SDL2_TTF_LIBRARIES}
  )
