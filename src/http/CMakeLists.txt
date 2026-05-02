set(http_SRC
	${CMAKE_CURRENT_LIST_DIR}/cacheinfo.cpp
	${CMAKE_CURRENT_LIST_DIR}/error.cpp
	${CMAKE_CURRENT_LIST_DIR}/http.cpp
	${CMAKE_CURRENT_LIST_DIR}/listener.cpp
	${CMAKE_CURRENT_LIST_DIR}/login.cpp
	${CMAKE_CURRENT_LIST_DIR}/router.cpp
	${CMAKE_CURRENT_LIST_DIR}/serverinfo.cpp
	${CMAKE_CURRENT_LIST_DIR}/session.cpp
	)

set(http_HDR
	${CMAKE_CURRENT_LIST_DIR}/cacheinfo.h
	${CMAKE_CURRENT_LIST_DIR}/error.h
	${CMAKE_CURRENT_LIST_DIR}/http.h
	${CMAKE_CURRENT_LIST_DIR}/listener.h
	${CMAKE_CURRENT_LIST_DIR}/login.h
	${CMAKE_CURRENT_LIST_DIR}/router.h
	${CMAKE_CURRENT_LIST_DIR}/serverinfo.h
	${CMAKE_CURRENT_LIST_DIR}/session.h
	)

add_library(http OBJECT ${http_SRC})
target_link_libraries(http PRIVATE Boost::json)

if(NOT MSVC)
    target_compile_options(http PRIVATE -Wno-deprecated-declarations) # maybe this could be removed when updating to boost greater than 1.83.0
endif()

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
