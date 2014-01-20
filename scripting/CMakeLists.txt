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

# Generate header with SWIG run-time functions
ADD_CUSTOM_COMMAND(
   OUTPUT "swigluarun.h"
   COMMAND ${SWIG_EXECUTABLE} -c++ -lua -external-runtime "swigluarun.h"
 )

LIST(APPEND CampvisScriptingHeaders "swigluarun.h")

ADD_EXECUTABLE(campvis-scripting ${CampvisScriptingSources} ${CampvisScriptingHeaders})
ADD_DEFINITIONS(${CampvisGlobalDefinitions} ${CampvisScriptingDefinitions})
INCLUDE_DIRECTORIES(${CampvisGlobalIncludeDirs} ${CMAKE_CURRENT_BINARY_DIR})
TARGET_LINK_LIBRARIES(campvis-scripting campvis-core campvis-modules tgt ${CampvisGlobalExternalLibs})

# If a multi-config generator is used (e.g. Visual Studio or XCode), CAMPVis Lua modules will be
# placed in a sub-directory that won't be picked up by Lua automatically. Let the code know via
# a #define what that sub-directory is so that it can instruct Lua to search it.
FOREACH(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES})
    STRING(TOUPPER ${CONFIG_TYPE} CONFIG_TYPE_UPPER)
    SET_DIRECTORY_PROPERTIES(PROPERTIES COMPILE_DEFINITIONS_${CONFIG_TYPE_UPPER}
        "CAMPVIS_LUA_MODS_PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CONFIG_TYPE}/?${CMAKE_SHARED_MODULE_SUFFIX}")
ENDFOREACH(CONFIG_TYPE CMAKE_CONFIGURATION_TYPES)
