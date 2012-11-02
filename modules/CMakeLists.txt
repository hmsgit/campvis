PROJECT(campvis-modules)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
#INCLUDE(../cmake/commonconf.cmake)

MESSAGE(STATUS "Configuring CAMPVis Modules")

ADD_LIBRARY(campvis-modules
    ${CAMPVisModulesSources} ${CAMPVisModulesHeaders} 
)

# TODO: Remove Qt dependency as soon as the pipelines got cleaned up (and thus removed their hard link to tgt/qt/qtcontextmanager.h

FIND_PACKAGE(Qt4 ${CAMPVIS_REQUIRED_QT_VERSION} REQUIRED QtCore QtGui QtOpenGL)
INCLUDE(${QT_USE_FILE})


ADD_DEFINITIONS(${CAMPVIS_DEFINITIONS} ${CAMPVisModulesDefinitions} ${QT_DEFINITIONS})
INCLUDE_DIRECTORIES(${CAMPVIS_INCLUDE_DIRECTORIES} ${CAMPVisModulesIncludeDirs})
TARGET_LINK_LIBRARIES(campvis-modules campvis-core tgt ${CAMPVisModulesExternalLibs} ${QT_LIBRARIES})
