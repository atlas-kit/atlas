set(http_SRC
	${CMAKE_CURRENT_LIST_DIR}/cache_info.cpp
	${CMAKE_CURRENT_LIST_DIR}/characters.cpp
	${CMAKE_CURRENT_LIST_DIR}/check_character_name.cpp
	${CMAKE_CURRENT_LIST_DIR}/check_email.cpp
	${CMAKE_CURRENT_LIST_DIR}/check_password.cpp
	${CMAKE_CURRENT_LIST_DIR}/create_account.cpp
	${CMAKE_CURRENT_LIST_DIR}/create_character.cpp
	${CMAKE_CURRENT_LIST_DIR}/enums.cpp
	${CMAKE_CURRENT_LIST_DIR}/error.cpp
	${CMAKE_CURRENT_LIST_DIR}/http.cpp
	${CMAKE_CURRENT_LIST_DIR}/listener.cpp
	${CMAKE_CURRENT_LIST_DIR}/login.cpp
	${CMAKE_CURRENT_LIST_DIR}/router.cpp
	${CMAKE_CURRENT_LIST_DIR}/server_info.cpp
	${CMAKE_CURRENT_LIST_DIR}/session.cpp
	${CMAKE_CURRENT_LIST_DIR}/validation.cpp
	${CMAKE_CURRENT_LIST_DIR}/worlds_info.cpp
	)

add_library(http OBJECT ${http_SRC})
target_link_libraries(http PRIVATE Boost::json)

if(NOT MSVC)
    target_compile_options(http PRIVATE -Wno-deprecated-declarations) # maybe this could be removed when updating to boost greater than 1.83.0
endif()

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
