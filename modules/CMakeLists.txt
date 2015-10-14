PROJECT(campvis-modules)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
#INCLUDE(../cmake/commonconf.cmake)

MESSAGE(STATUS "Configuring CAMPVis-Modules")

LIST(APPEND CampvisModulesHeaders
    modulesapi.h
)

LINK_DIRECTORIES(${CampvisGlobalLinkDirectories} ${CampvisModulesLinkDirectories})

ADD_LIBRARY(campvis-modules
    ${CampvisModulesSources} ${CampvisModulesHeaders} 
)

ADD_DEFINITIONS(${CampvisGlobalDefinitions} ${CampvisModulesDefinitions})
INCLUDE_DIRECTORIES(${CampvisGlobalIncludeDirs} ${CampvisModulesIncludeDirs})
TARGET_LINK_LIBRARIES(campvis-modules campvis-core cgt sigslot ${CampvisGlobalExternalLibs} ${CampvisModulesExternalLibs})

# if campvis-core is built as a shared library, CMake will define the following flag to instruct
# the code to export DLL symbols
SET_TARGET_PROPERTIES(campvis-modules PROPERTIES DEFINE_SYMBOL "CAMPVIS_MODULES_BUILD_DLL")

IF(CAMPVIS_GROUP_SOURCE_FILES)
    DEFINE_SOURCE_GROUPS_FROM_SUBDIR(CampvisModulesSources ${CampvisHome} "")
    DEFINE_SOURCE_GROUPS_FROM_SUBDIR(CampvisModulesHeaders ${CampvisHome} "")
ENDIF()


INSTALL(TARGETS campvis-modules DESTINATION exports EXPORT campvis-targets)
