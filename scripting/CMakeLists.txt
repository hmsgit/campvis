PROJECT(campvis-scripting)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)

MESSAGE(STATUS "Configuring CAMPVis Scripting")

# Lua
FIND_PACKAGE(Lua REQUIRED)
IF(LUA_FOUND)
    MESSAGE(STATUS "* Found Lua")
    LIST(APPEND CampvisGlobalDefinitions ${LUA_DEFINITIONS})
    LIST(APPEND CampvisGlobalIncludeDirs ${LUA_INCLUDE_DIR})
    LIST(APPEND CampvisGlobalExternalLibs ${LUA_LIBRARY})
ELSE(LUA_FOUND)
    MESSAGE(FATAL_ERROR "Lua not found!")
ENDIF(LUA_FOUND)

FILE(GLOB CampvisScriptingSources
    *.cpp
    *.c
)

FILE(GLOB CampvisScriptingHeaders
    *.h
)

ADD_EXECUTABLE(campvis-scripting ${CampvisScriptingSources} ${CampvisScriptingHeaders})
ADD_DEFINITIONS(${CampvisGlobalDefinitions} ${CampvisScriptingDefinitions})
INCLUDE_DIRECTORIES(${CampvisGlobalIncludeDirs})
TARGET_LINK_LIBRARIES(campvis-scripting campvis-core tgt ${CampvisGlobalExternalLibs})
