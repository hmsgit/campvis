# Try to find LUA library and include path. Once done this will define:
# LUA_FOUND
# LUA_DEFINITIONS
# LUA_INCLUDE_DIR
# LUA_LIBRARY

IF(CAMPVIS_BUILD_LIB_LUA)
    SET(LUA_DIR "${CampvisHome}/ext/lua" CACHE PATH "If Lua is not found, set this path")

    SET(LUA_INCLUDE_DIR "${LUA_DIR}/src" "${CMAKE_BINARY_DIR}/ext/lua")

    IF(LUA_INCLUDE_DIR)
        SET(LUA_LIBRARY liblua)
        SET(LUA_FOUND TRUE)
    ELSE()
        SET(LUA_FOUND FALSE)
    ENDIF()
ENDIF(CAMPVIS_BUILD_LIB_LUA)

MARK_AS_ADVANCED(LUA_DIR LUA_INCLUDE_DIR LUA_LIBRARY)
