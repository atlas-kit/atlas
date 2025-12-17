set(luamodules_SRC
	${CMAKE_CURRENT_LIST_DIR}action.cpp
    ${CMAKE_CURRENT_LIST_DIR}combat.cpp
    ${CMAKE_CURRENT_LIST_DIR}condition.cpp
    ${CMAKE_CURRENT_LIST_DIR}configmanager.cpp
    ${CMAKE_CURRENT_LIST_DIR}container.cpp
    ${CMAKE_CURRENT_LIST_DIR}creature.cpp
    ${CMAKE_CURRENT_LIST_DIR}creatureevent.cpp
    ${CMAKE_CURRENT_LIST_DIR}dbinsert.cpp
    ${CMAKE_CURRENT_LIST_DIR}dbtransaction.cpp
    ${CMAKE_CURRENT_LIST_DIR}game.cpp
    ${CMAKE_CURRENT_LIST_DIR}globalevent.cpp
    ${CMAKE_CURRENT_LIST_DIR}group.cpp
    ${CMAKE_CURRENT_LIST_DIR}guild.cpp
    ${CMAKE_CURRENT_LIST_DIR}house.cpp
    ${CMAKE_CURRENT_LIST_DIR}item.cpp
    ${CMAKE_CURRENT_LIST_DIR}itemtype.cpp
    ${CMAKE_CURRENT_LIST_DIR}loot.cpp
    ${CMAKE_CURRENT_LIST_DIR}modalwindow.cpp
    ${CMAKE_CURRENT_LIST_DIR}monster.cpp
    ${CMAKE_CURRENT_LIST_DIR}monsterspell.cpp
    ${CMAKE_CURRENT_LIST_DIR}monstertype.cpp
    ${CMAKE_CURRENT_LIST_DIR}moveevent.cpp
    ${CMAKE_CURRENT_LIST_DIR}networkmessage.cpp
    ${CMAKE_CURRENT_LIST_DIR}npc.cpp
    ${CMAKE_CURRENT_LIST_DIR}outfit.cpp
    ${CMAKE_CURRENT_LIST_DIR}party.cpp
    ${CMAKE_CURRENT_LIST_DIR}player.cpp
    ${CMAKE_CURRENT_LIST_DIR}podium.cpp
    ${CMAKE_CURRENT_LIST_DIR}position.cpp
    ${CMAKE_CURRENT_LIST_DIR}spell.cpp
    ${CMAKE_CURRENT_LIST_DIR}talkaction.cpp
    ${CMAKE_CURRENT_LIST_DIR}teleport.cpp
    ${CMAKE_CURRENT_LIST_DIR}tile.cpp
    ${CMAKE_CURRENT_LIST_DIR}variant.cpp
    ${CMAKE_CURRENT_LIST_DIR}vocation.cpp
    ${CMAKE_CURRENT_LIST_DIR}weapon.cpp
    ${CMAKE_CURRENT_LIST_DIR}xml.cpp
	)

add_library(luamodules OBJECT ${luamodules_SRC})
target_link_libraries(luamodules PRIVATE Boost::json)
set_target_properties(luamodules PROPERTIES UNITY_BUILD ${ENABLE_UNITY_BUILD})

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
