
add_library (cellular_automaton_lib SHARED "")

set (CMAKE_POSITION_INDEPENDENT_CODE ON)

target_sources (cellular_automaton_lib PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/InfoBar.cc
	${CMAKE_CURRENT_SOURCE_DIR}/ColonyRenderer.cc
	${CMAKE_CURRENT_SOURCE_DIR}/ColonyStatus.cc
	${CMAKE_CURRENT_SOURCE_DIR}/Colony.cc
	${CMAKE_CURRENT_SOURCE_DIR}/ColonyScheduler.cc
	${CMAKE_CURRENT_SOURCE_DIR}/CellsBlocks.cc
	${CMAKE_CURRENT_SOURCE_DIR}/RulesetSelector.cc
	${CMAKE_CURRENT_SOURCE_DIR}/RenderingProperties.cc
	${CMAKE_CURRENT_SOURCE_DIR}/BrushSelector.cc
	${CMAKE_CURRENT_SOURCE_DIR}/CellBrush.cc
	)

target_link_libraries(cellular_automaton_lib
	sdl_core
	sdl_graphic
	)

target_include_directories (cellular_automaton_lib PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	)
