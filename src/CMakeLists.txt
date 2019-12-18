project (cellular_automaton_lib)

set (CMAKE_CXX_STANDARD 14)

#set (CMAKE_VERBOSE_MAKEFILE ON)
set (CMAKE_POSITION_INDEPENDENT_CODE ON)

set (SOURCES
  StatusBar.cc
  ColonyRenderer.cc
  ColonyStatus.cc
  )

add_library (cellular_automaton_lib SHARED
  ${SOURCES}
  )

set(CELLULAR_AUTOMATON_LIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)

target_link_libraries(cellular_automaton_lib
  sdl_core
  sdl_graphic
  )

target_include_directories (cellular_automaton_lib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
