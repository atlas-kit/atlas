set(tfs_SRC
	${CMAKE_CURRENT_LIST_DIR}/actions.cpp
	${CMAKE_CURRENT_LIST_DIR}/ban.cpp
	${CMAKE_CURRENT_LIST_DIR}/base64.cpp
	${CMAKE_CURRENT_LIST_DIR}/baseevents.cpp
	${CMAKE_CURRENT_LIST_DIR}/chat.cpp
	${CMAKE_CURRENT_LIST_DIR}/combat.cpp
	${CMAKE_CURRENT_LIST_DIR}/condition.cpp
	${CMAKE_CURRENT_LIST_DIR}/configmanager.cpp
	${CMAKE_CURRENT_LIST_DIR}/connection.cpp
	${CMAKE_CURRENT_LIST_DIR}/container.cpp
	${CMAKE_CURRENT_LIST_DIR}/creature.cpp
	${CMAKE_CURRENT_LIST_DIR}/databasemanager.cpp
	${CMAKE_CURRENT_LIST_DIR}/databasetasks.cpp
	${CMAKE_CURRENT_LIST_DIR}/depotchest.cpp
	${CMAKE_CURRENT_LIST_DIR}/depotlocker.cpp
	${CMAKE_CURRENT_LIST_DIR}/fileloader.cpp
	${CMAKE_CURRENT_LIST_DIR}/game.cpp
	${CMAKE_CURRENT_LIST_DIR}/groups.cpp
	${CMAKE_CURRENT_LIST_DIR}/guild.cpp
	${CMAKE_CURRENT_LIST_DIR}/house.cpp
	${CMAKE_CURRENT_LIST_DIR}/housetile.cpp
	${CMAKE_CURRENT_LIST_DIR}/inbox.cpp
	${CMAKE_CURRENT_LIST_DIR}/iologindata.cpp
	${CMAKE_CURRENT_LIST_DIR}/iomap.cpp
	${CMAKE_CURRENT_LIST_DIR}/iomapserialize.cpp
	${CMAKE_CURRENT_LIST_DIR}/iomarket.cpp
	${CMAKE_CURRENT_LIST_DIR}/item.cpp
	${CMAKE_CURRENT_LIST_DIR}/items.cpp
	${CMAKE_CURRENT_LIST_DIR}/mailbox.cpp
	${CMAKE_CURRENT_LIST_DIR}/map.cpp
	${CMAKE_CURRENT_LIST_DIR}/matrixarea.cpp
	${CMAKE_CURRENT_LIST_DIR}/monster.cpp
	${CMAKE_CURRENT_LIST_DIR}/monsters.cpp
	${CMAKE_CURRENT_LIST_DIR}/movement.cpp
	${CMAKE_CURRENT_LIST_DIR}/networkmessage.cpp
	${CMAKE_CURRENT_LIST_DIR}/npc.cpp
	${CMAKE_CURRENT_LIST_DIR}/outputmessage.cpp
	${CMAKE_CURRENT_LIST_DIR}/party.cpp
	${CMAKE_CURRENT_LIST_DIR}/player.cpp
	${CMAKE_CURRENT_LIST_DIR}/podium.cpp
	${CMAKE_CURRENT_LIST_DIR}/position.cpp
	${CMAKE_CURRENT_LIST_DIR}/protocol.cpp
	${CMAKE_CURRENT_LIST_DIR}/protocolgame.cpp
	${CMAKE_CURRENT_LIST_DIR}/protocolstatus.cpp
	${CMAKE_CURRENT_LIST_DIR}/rsa.cpp
	${CMAKE_CURRENT_LIST_DIR}/scheduler.cpp
	${CMAKE_CURRENT_LIST_DIR}/script.cpp
	${CMAKE_CURRENT_LIST_DIR}/scriptmanager.cpp
	${CMAKE_CURRENT_LIST_DIR}/server.cpp
	${CMAKE_CURRENT_LIST_DIR}/signals.cpp
	${CMAKE_CURRENT_LIST_DIR}/spawn.cpp
	${CMAKE_CURRENT_LIST_DIR}/spells.cpp
	${CMAKE_CURRENT_LIST_DIR}/storeinbox.cpp
	${CMAKE_CURRENT_LIST_DIR}/talkaction.cpp
	${CMAKE_CURRENT_LIST_DIR}/tasks.cpp
	${CMAKE_CURRENT_LIST_DIR}/teleport.cpp
	${CMAKE_CURRENT_LIST_DIR}/tile.cpp
	${CMAKE_CURRENT_LIST_DIR}/tools.cpp
	${CMAKE_CURRENT_LIST_DIR}/trashholder.cpp
	${CMAKE_CURRENT_LIST_DIR}/vocation.cpp
	${CMAKE_CURRENT_LIST_DIR}/weapons.cpp
	${CMAKE_CURRENT_LIST_DIR}/wildcardtree.cpp
	${CMAKE_CURRENT_LIST_DIR}/xtea.cpp
	)

set(tfs_HDR
	${CMAKE_CURRENT_LIST_DIR}/otpch.h
	${CMAKE_CURRENT_LIST_DIR}/actions.h
	${CMAKE_CURRENT_LIST_DIR}/ban.h
	${CMAKE_CURRENT_LIST_DIR}/base64.h
	${CMAKE_CURRENT_LIST_DIR}/baseevents.h
	${CMAKE_CURRENT_LIST_DIR}/chat.h
	${CMAKE_CURRENT_LIST_DIR}/combat.h
	${CMAKE_CURRENT_LIST_DIR}/condition.h
	${CMAKE_CURRENT_LIST_DIR}/configmanager.h
	${CMAKE_CURRENT_LIST_DIR}/connection.h
	${CMAKE_CURRENT_LIST_DIR}/const.h
	${CMAKE_CURRENT_LIST_DIR}/container.h
	${CMAKE_CURRENT_LIST_DIR}/creature.h
	${CMAKE_CURRENT_LIST_DIR}/database.h
	${CMAKE_CURRENT_LIST_DIR}/databasemanager.h
	${CMAKE_CURRENT_LIST_DIR}/databasetasks.h
	${CMAKE_CURRENT_LIST_DIR}/definitions.h
	${CMAKE_CURRENT_LIST_DIR}/depotchest.h
	${CMAKE_CURRENT_LIST_DIR}/depotlocker.h
	${CMAKE_CURRENT_LIST_DIR}/enums.h
	${CMAKE_CURRENT_LIST_DIR}/fileloader.h
	${CMAKE_CURRENT_LIST_DIR}/game.h
	${CMAKE_CURRENT_LIST_DIR}/groups.h
	${CMAKE_CURRENT_LIST_DIR}/guild.h
	${CMAKE_CURRENT_LIST_DIR}/house.h
	${CMAKE_CURRENT_LIST_DIR}/housetile.h
	${CMAKE_CURRENT_LIST_DIR}/inbox.h
	${CMAKE_CURRENT_LIST_DIR}/iologindata.h
	${CMAKE_CURRENT_LIST_DIR}/iomap.h
	${CMAKE_CURRENT_LIST_DIR}/iomapserialize.h
	${CMAKE_CURRENT_LIST_DIR}/iomarket.h
	${CMAKE_CURRENT_LIST_DIR}/item.h
	${CMAKE_CURRENT_LIST_DIR}/itemloader.h
	${CMAKE_CURRENT_LIST_DIR}/items.h
	${CMAKE_CURRENT_LIST_DIR}/lockfree.h
	${CMAKE_CURRENT_LIST_DIR}/luavariant.h
	${CMAKE_CURRENT_LIST_DIR}/mailbox.h
	${CMAKE_CURRENT_LIST_DIR}/map.h
	${CMAKE_CURRENT_LIST_DIR}/matrixarea.h
	${CMAKE_CURRENT_LIST_DIR}/monster.h
	${CMAKE_CURRENT_LIST_DIR}/monsters.h
	${CMAKE_CURRENT_LIST_DIR}/movement.h
	${CMAKE_CURRENT_LIST_DIR}/networkmessage.h
	${CMAKE_CURRENT_LIST_DIR}/npc.h
	${CMAKE_CURRENT_LIST_DIR}/outputmessage.h
	${CMAKE_CURRENT_LIST_DIR}/party.h
	${CMAKE_CURRENT_LIST_DIR}/player.h
	${CMAKE_CURRENT_LIST_DIR}/podium.h
	${CMAKE_CURRENT_LIST_DIR}/position.h
	${CMAKE_CURRENT_LIST_DIR}/protocolgame.h
	${CMAKE_CURRENT_LIST_DIR}/protocol.h
	${CMAKE_CURRENT_LIST_DIR}/protocolstatus.h
	${CMAKE_CURRENT_LIST_DIR}/pugicast.h
	${CMAKE_CURRENT_LIST_DIR}/rsa.h
	${CMAKE_CURRENT_LIST_DIR}/scheduler.h
	${CMAKE_CURRENT_LIST_DIR}/script.h
	${CMAKE_CURRENT_LIST_DIR}/scriptmanager.h
	${CMAKE_CURRENT_LIST_DIR}/server.h
	${CMAKE_CURRENT_LIST_DIR}/signals.h
	${CMAKE_CURRENT_LIST_DIR}/spawn.h
	${CMAKE_CURRENT_LIST_DIR}/spells.h
	${CMAKE_CURRENT_LIST_DIR}/storeinbox.h
	${CMAKE_CURRENT_LIST_DIR}/talkaction.h
	${CMAKE_CURRENT_LIST_DIR}/tasks.h
	${CMAKE_CURRENT_LIST_DIR}/teleport.h
	${CMAKE_CURRENT_LIST_DIR}/thing.h
	${CMAKE_CURRENT_LIST_DIR}/thread_holder_base.h
	${CMAKE_CURRENT_LIST_DIR}/tile.h
	${CMAKE_CURRENT_LIST_DIR}/tools.h
	${CMAKE_CURRENT_LIST_DIR}/town.h
	${CMAKE_CURRENT_LIST_DIR}/trashholder.h
	${CMAKE_CURRENT_LIST_DIR}/vocation.h
	${CMAKE_CURRENT_LIST_DIR}/weapons.h
	${CMAKE_CURRENT_LIST_DIR}/wildcardtree.h
	${CMAKE_CURRENT_LIST_DIR}/xtea.h
	)

set(tfs_MAIN ${CMAKE_CURRENT_LIST_DIR}/otserv.cpp)

# Select exactly one database backend. The public Database/DBResult API is identical, so nothing
# else in the tree changes regardless of which backend is compiled.
if (USE_BOOST_MYSQL)
	list(APPEND tfs_SRC ${CMAKE_CURRENT_LIST_DIR}/database_boost.cpp)
else ()
	list(APPEND tfs_SRC ${CMAKE_CURRENT_LIST_DIR}/database.cpp)
endif ()

add_library(tfslib ${tfs_SRC})
target_compile_definitions(tfslib PRIVATE SPDLOG_HEADER_ONLY)
target_link_libraries(tfslib PRIVATE
	Boost::iostreams
	OpenSSL::Crypto
	pugixml::pugixml
	simdutf::simdutf
	spdlog::spdlog_header_only
	ZLIB::ZLIB
	${CMAKE_THREAD_LIBS_INIT}
	${LUA_LIBRARIES}
	)

# Link only the backend actually compiled. The C client libs are pulled in just for the
# database.cpp backend; the Boost.MySQL backend needs Boost::mysql + OpenSSL::SSL instead.
if (USE_BOOST_MYSQL)
	target_link_libraries(tfslib PRIVATE Boost::mysql OpenSSL::SSL)
else ()
	target_link_libraries(tfslib PRIVATE ${MYSQL_CLIENT_LIBS})
endif ()

if (ENABLE_HTTP)
	add_subdirectory(http)
	target_link_libraries(tfslib PRIVATE http)
endif ()

add_subdirectory(lua)
target_sources(tfslib PRIVATE $<TARGET_OBJECTS:luascript>)

add_subdirectory(events)
target_sources(tfslib PRIVATE $<TARGET_OBJECTS:events>)

add_custom_target(format COMMAND /usr/bin/clang-format -style=file -i ${tfs_HDR} ${tfs_SRC} ${tfs_MAIN} ${events_HDR})

set(tfs_RC "")
if (WIN32)
    set(tfs_RC ${CMAKE_SOURCE_DIR}/atlas.rc)
endif ()

add_executable(tfs ${tfs_MAIN} ${tfs_RC})
target_link_libraries(tfs PRIVATE tfslib)
target_precompile_headers(tfs PUBLIC otpch.h)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
