set(pathfinding_SRC
	${CMAKE_CURRENT_LIST_DIR}/path_goal.cpp
	${CMAKE_CURRENT_LIST_DIR}/path_request.cpp
	${CMAKE_CURRENT_LIST_DIR}/pathfinder.cpp
	)

set(pathfinding_HDR
	${CMAKE_CURRENT_LIST_DIR}/constants.h
	${CMAKE_CURRENT_LIST_DIR}/distance.h
	${CMAKE_CURRENT_LIST_DIR}/path_goal.h
	${CMAKE_CURRENT_LIST_DIR}/path_request.h
	${CMAKE_CURRENT_LIST_DIR}/path_result.h
	${CMAKE_CURRENT_LIST_DIR}/path_world.h
	${CMAKE_CURRENT_LIST_DIR}/pathfinder.h
	${CMAKE_CURRENT_LIST_DIR}/search_mode.h
	)

set(pathfinding_HDR ${pathfinding_HDR} PARENT_SCOPE)
set(pathfinding_SRC ${pathfinding_SRC} PARENT_SCOPE)

add_library(pathfinding OBJECT ${pathfinding_SRC})
target_include_directories(pathfinding PRIVATE ${PROJECT_SOURCE_DIR}/src)
