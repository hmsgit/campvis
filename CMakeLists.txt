# = CAMPVis - Yet another medical visualization framework =====================
PROJECT(CAMPVis)
CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0 FATAL_ERROR)

SET(CAMPVIS_VERSION 1.0.0)

# = Global Build Options ======================================================
OPTION(BUILD_SHARED_LIBS            "Build shared libraries (strongly recommended!)"                ON)
OPTION(CAMPVIS_DEBUG                "Activate debug code?"                                          ON)
OPTION(CAMPVIS_BUILD_APPLICATION    "Build CAMPVis Application"                                     ON)
OPTION(CAMPVIS_BUILD_MODULES        "Build CAMPVis Modules"                                         ON)
OPTION(CAMPVIS_ENABLE_SCRIPTING     "Add support for scripting CAMPVis using Lua"                   OFF)
OPTION(CAMPVIS_BUILD_DOXYGEN        "Build Doxygen Documentation"                                   OFF)

OPTION(CAMPVIS_DEPLOY_SHADERS       "Deploy Shader files to binary directory"                       OFF)
OPTION(CAMPVIS_GROUP_SOURCE_FILES   "Group source files by directory"                               ON)
OPTION(CAMPVIS_ENABLE_TESTING       "Build CAMPVis Unit Tests using gooogletest"                    OFF)

IF(WIN32)
    OPTION(CAMPVIS_COPY_EXTERNAL_DLLS   "Copy external DLLs to bin directory?"                      ON)
    OPTION(CAMPVIS_INCREMENTAL_LINKING  "Enable incremental linking in Visual Studio debug builds?" ON)
    OPTION(CAMPVIS_GENERATE_MANIFEST    "Generate manifest in Visual Studio debug builds (switch on when encountering errors using incremental linking)?"          OFF)
ENDIF()


# = Further Build Options =====================================================
SET(CAMPVIS_DEFAULT_ENABLED_MODULES "STABLE_NO_DEPENDENCIES" CACHE STRING "Default CAMPVis modules to activate")
SET_PROPERTY(CACHE CAMPVIS_DEFAULT_ENABLED_MODULES PROPERTY STRINGS "NONE" "STABLE_NO_DEPENDENCIES" "STABLE_WITH_EXTERNAL_DEPENDENCIES" "TESTING" "ALL")

# propagate CAMPVIS_ENABLE_SCRIPTING to CAMPVIS_BUILD_LIB_LUA to support deprecated code
IF (CAMPVIS_ENABLE_SCRIPTING)
    SET(CAMPVIS_BUILD_LIB_LUA ON)
ENDIF()

include(cmake/commonconf.cmake)
include(cmake/parseModulesDir.cmake)


# = Start the definition of all CAMPVis targets ===============================
MESSAGE(STATUS "--------------------------------------------------------------------------------")

# cgt, sigslot and campvis-core are enabled by default.
ADD_SUBDIRECTORY(ext/cgt)
ADD_SUBDIRECTORY(ext/sigslot)
ADD_SUBDIRECTORY(core)

IF(CAMPVIS_BUILD_MODULES)
    ADD_SUBDIRECTORY(modules)
ENDIF()

# build scripting targets when enabled
IF(CAMPVIS_ENABLE_SCRIPTING)
    # build Lua from source
    IF(WIN32)
        # Keep LUA_BUILD_AS_DLL in sync with BUILD_SHARED_LIBS
        SET(LUA_BUILD_AS_DLL ${BUILD_SHARED_LIBS} CACHE BOOL "Build Lua library as DLL" FORCE)
    ENDIF(WIN32)

    ADD_SUBDIRECTORY(ext/lua EXCLUDE_FROM_ALL)
    # If the above command failed, provide a hint how to fix the problem
    IF(NOT TARGET liblua)
        MESSAGE(SEND_ERROR "Did you forget to run `git submodule update --init`?")
    ENDIF(NOT TARGET liblua)

    # First, find Lua to setup paths for all projects correctly
    FIND_PACKAGE(Lua REQUIRED)
    IF(LUA_FOUND)
        MESSAGE(STATUS "* Found Lua")
        LIST(APPEND CampvisGlobalDefinitions ${LUA_DEFINITIONS})
        LIST(APPEND CampvisGlobalIncludeDirs ${LUA_INCLUDE_DIR})
        LIST(APPEND CampvisGlobalExternalLibs ${LUA_LIBRARY})
    ELSE(LUA_FOUND)
        MESSAGE(FATAL_ERROR "Lua not found!")
    ENDIF(LUA_FOUND)
    
    ADD_SUBDIRECTORY(scripting)
    LIST(APPEND CampvisGlobalDefinitions "-DCAMPVIS_HAS_SCRIPTING")
ENDIF()

IF(CAMPVIS_BUILD_APPLICATION)
    ADD_SUBDIRECTORY(application)
ENDIF()

# build campvis-test when enabled
IF(CAMPVIS_ENABLE_TESTING)
    ADD_SUBDIRECTORY(ext/gtest-1.7.0)
    ADD_SUBDIRECTORY(test)
ENDIF()

# build doxygen when enabled
IF(CAMPVIS_BUILD_DOXYGEN)
    ADD_SUBDIRECTORY(doc EXCLUDE_FROM_ALL)
ENDIF()


MESSAGE(STATUS "--------------------------------------------------------------------------------")
MESSAGE(STATUS "Finishing up")


# = Export all CAMPVis Targets so that they can be included easily ============
# write package version file
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/CAMPVisConfigVersion.cmake"
  VERSION ${CAMPVIS_VERSION}
  COMPATIBILITY AnyNewerVersion
)
# write campvis targets file
export(EXPORT campvis-targets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/CAMPVisTargets.cmake"
#  NAMESPACE Upstream::
)
# write campvis configuration file
configure_file("${CampvisHome}/cmake/CAMPVisConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/CAMPVisConfig.cmake"
  @ONLY
)

#INSTALL(EXPORT campvis-targets NAMESPACE Upstream:: DESTINATION ${CampvisLibInstallDir})
EXPORT(PACKAGE CAMPVis)

# = Copy Windows DLLs to binary dir for improved development experience =======
IF(WIN32)
    # gather Qt4 DLLs - TODO: remove redundant naming of required components 
    FIND_PACKAGE(Qt4DLLs COMPONENTS QtCore QtGui QtOpenGL QtNetwork QtXmlPatterns)
    LIST(APPEND CampvisExternalDllsDebug ${QT_DEBUG_DLLS})
    LIST(APPEND CampvisExternalDllsRelease ${QT_RELEASE_DLLS})

    LIST(REMOVE_DUPLICATES CampvisExternalDllsDebug)
    LIST(REMOVE_DUPLICATES CampvisExternalDllsRelease)

    IF(CAMPVIS_COPY_EXTERNAL_DLLS)
        COPY_EXTERNAL_DLLS(CampvisExternalDllsDebug CampvisExternalDllsRelease false)
    ENDIF()
ENDIF()

MESSAGE(STATUS "--------------------------------------------------------------------------------")
