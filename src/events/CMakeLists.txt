set(events_SRC
	${CMAKE_CURRENT_LIST_DIR}/creature.cpp
	${CMAKE_CURRENT_LIST_DIR}/events.cpp
	${CMAKE_CURRENT_LIST_DIR}/game.cpp
	${CMAKE_CURRENT_LIST_DIR}/monster.cpp
	${CMAKE_CURRENT_LIST_DIR}/party.cpp
	${CMAKE_CURRENT_LIST_DIR}/player.cpp
	)

set(events_HDR
	${CMAKE_CURRENT_LIST_DIR}/creature.h
	${CMAKE_CURRENT_LIST_DIR}/events.h
	${CMAKE_CURRENT_LIST_DIR}/game.h
	${CMAKE_CURRENT_LIST_DIR}/monster.h
	${CMAKE_CURRENT_LIST_DIR}/party.h
	${CMAKE_CURRENT_LIST_DIR}/player.h
	)

set(events_HDR ${events_HDR} PARENT_SCOPE)

add_library(events OBJECT ${events_SRC} ${events_HDR})
