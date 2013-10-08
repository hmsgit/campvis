PROJECT(campvis-modules)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
#INCLUDE(../cmake/commonconf.cmake)

MESSAGE(STATUS "Configuring Campvis Modules")

LIST(APPEND CampvisModulesHeaders
    pipelinefactory.h
)
LIST(APPEND CampvisModulesSources
    pipelinefactory.cpp
)


LINK_DIRECTORIES(${CampvisGlobalLinkDirectories} ${CampvisModulesLinkDirectories})
ADD_LIBRARY(campvis-modules
    ${CampvisModulesSources} ${CampvisModulesHeaders} 
)

# TODO: Remove Qt dependency as soon as the pipelines got cleaned up (and thus removed their hard link to tgt/qt/qtcontextmanager.h

FIND_PACKAGE(Qt4 ${CampvisRequiredQtVersion} REQUIRED QtCore QtGui QtOpenGL)
INCLUDE(${QT_USE_FILE})

ADD_DEFINITIONS(${CampvisGlobalDefinitions} ${CampvisModulesDefinitions} ${QT_DEFINITIONS})
INCLUDE_DIRECTORIES(${CampvisGlobalIncludeDirs} ${CampvisModulesIncludeDirs})
TARGET_LINK_LIBRARIES(campvis-modules campvis-core tgt ${CampvisGlobalExternalLibs} ${CampvisModulesExternalLibs} ${QT_LIBRARIES})

IF(CAMPVIS_GROUP_SOURCE_FILES)
    DEFINE_SOURCE_GROUPS_FROM_SUBDIR(CampvisModulesSources ${CampvisHome} "")
    DEFINE_SOURCE_GROUPS_FROM_SUBDIR(CampvisModulesHeaders ${CampvisHome} "")
ENDIF()
