# Try to find LUA library and include path. Once done this will define:
# LUA_FOUND
# LUA_DEFINITIONS
# LUA_INCLUDE_DIR
# LUA_LIBRARY

IF(WIN32)
    SET(LUA_DIR "${CampvisHome}/ext/lua" CACHE PATH "If Lua is not found, set this path")

    SET(LUA_INCLUDE_DIR "${LUA_DIR}/src" ${LUA_DIR})

    IF(LUA_INCLUDE_DIR)
        SET(LUA_LIBRARY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lua.lib")
        SET(LUA_FOUND TRUE)
    ELSE()
        SET(LUA_FOUND FALSE)
    ENDIF()

ELSE(WIN32)
    # TODO: not tested
    MESSAGE(FATAL_ERROR "FindLua.cmake doesn't support platforms other than Windows yet")
ENDIF(WIN32)

MARK_AS_ADVANCED(LUA_DIR LUA_INCLUDE_DIR LUA_LIBRARY)
