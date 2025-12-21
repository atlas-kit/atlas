set(lua_SRC
	${CMAKE_CURRENT_LIST_DIR}/api.cpp
	${CMAKE_CURRENT_LIST_DIR}/env.cpp
	${CMAKE_CURRENT_LIST_DIR}/error.cpp
	${CMAKE_CURRENT_LIST_DIR}/meta.cpp
	${CMAKE_CURRENT_LIST_DIR}/modules.cpp
	${CMAKE_CURRENT_LIST_DIR}/script.cpp
	)

set(lua_HDR
	${CMAKE_CURRENT_LIST_DIR}/api.h
	${CMAKE_CURRENT_LIST_DIR}/env.h
	${CMAKE_CURRENT_LIST_DIR}/error.h
	${CMAKE_CURRENT_LIST_DIR}/meta.h
	${CMAKE_CURRENT_LIST_DIR}/modules.h
	${CMAKE_CURRENT_LIST_DIR}/register.h
	${CMAKE_CURRENT_LIST_DIR}/script.h
	${CMAKE_CURRENT_LIST_DIR}/variant.h
	)

add_subdirectory(modules)

add_library(tfs_lua OBJECT ${lua_SRC} ${lua_HDR})
set_target_properties(tfs_lua PROPERTIES UNITY_BUILD ${ENABLE_UNITY_BUILD})

target_sources(tfs_lua PRIVATE $<TARGET_OBJECTS:luamodules>)
