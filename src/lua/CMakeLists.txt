set(lua_SRC
	${CMAKE_CURRENT_LIST_DIR}/api.cpp
	${CMAKE_CURRENT_LIST_DIR}/env.cpp
	${CMAKE_CURRENT_LIST_DIR}/error.cpp
	${CMAKE_CURRENT_LIST_DIR}/meta.cpp
	)

set(lua_HDR
	${CMAKE_CURRENT_LIST_DIR}/api.h
	${CMAKE_CURRENT_LIST_DIR}/env.h
	${CMAKE_CURRENT_LIST_DIR}/error.h
	${CMAKE_CURRENT_LIST_DIR}/meta.h
	${CMAKE_CURRENT_LIST_DIR}/register.h
	${CMAKE_CURRENT_LIST_DIR}/variant.h
	)

add_library(lua OBJECT ${lua_SRC})
target_link_libraries(lua PRIVATE Boost::json)
set_target_properties(lua PROPERTIES UNITY_BUILD ${ENABLE_UNITY_BUILD})

add_subdirectory(luamodules)
target_link_libraries(tfslib PRIVATE luamodules)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
