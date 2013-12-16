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

# SWIG
FIND_PACKAGE(SWIG REQUIRED)
MESSAGE(STATUS "* Found SWIG")
INCLUDE(${SWIG_USE_FILE})

FILE(GLOB CampvisScriptingSources
    *.cpp
    *.c
)

FILE(GLOB CampvisScriptingHeaders
    *.h
)

SET_SOURCE_FILES_PROPERTIES(campvis.i PROPERTIES CPLUSPLUS ON)
SWIG_ADD_MODULE(campvis lua campvis.i)
SWIG_LINK_LIBRARIES(campvis campvis-core tgt ${CampvisGlobalExternalLibs})

SET_SOURCE_FILES_PROPERTIES(vis.i PROPERTIES CPLUSPLUS ON)
SWIG_ADD_MODULE(vis lua vis.i)
SWIG_LINK_LIBRARIES(vis campvis-core campvis-modules tgt ${CampvisGlobalExternalLibs})

ADD_EXECUTABLE(campvis-scripting ${CampvisScriptingSources} ${CampvisScriptingHeaders})
ADD_DEFINITIONS(${CampvisGlobalDefinitions} ${CampvisScriptingDefinitions})
INCLUDE_DIRECTORIES(${CampvisGlobalIncludeDirs})
TARGET_LINK_LIBRARIES(campvis-scripting campvis-core tgt ${CampvisGlobalExternalLibs})

# Copy Lua test scripts into the target directory
ADD_CUSTOM_COMMAND(
   TARGET campvis-scripting
   POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy "script.lua" ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
 )
SET_SOURCE_FILES_PROPERTIES(testapp.cpp PROPERTIES OBJECT_DEPENDS "scripting/script.lua")
