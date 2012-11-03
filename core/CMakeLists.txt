PROJECT(campvis-core)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
INCLUDE(../cmake/commonconf.cmake)

MESSAGE(STATUS "Configuring CAMPVis Core")

FILE(GLOB CampvisCoreSources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    classification/*.cpp
    datastructures/*.cpp
    eventhandlers/*.cpp
    pipeline/*.cpp
    properties/*.cpp
    tools/*.cpp
)

FILE(GLOB CampvisCoreHeaders RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    classification/*.h
    datastructures/*.h
    eventhandlers/*.h
    pipeline/*.h
    properties/*.h
    tools/*.h
    glsl/*.vert
    glsl/*.frag
    glsl/tools/*.vert
    glsl/tools/*.frag
)

#SET(CampvisCoreSources
#    application/campvis.cpp
#    application/campvispainter.cpp
#    datastructures/
#)

#SET(CampvisCoreHeaders 
#    application/campvispainter.h
#)

#
# Qt related stuff:
#
# TODO: remove Qt stuff here!
FIND_PACKAGE(Qt4 ${CampvisRequiredQtVersion} REQUIRED QtCore QtGui QtOpenGL)
INCLUDE(${QT_USE_FILE})


ADD_LIBRARY(campvis-core 
    ${CampvisCoreSources} ${CampvisCoreHeaders} 
)
ADD_DEFINITIONS(${CampvisGlobalDefinitions} ${QT_DEFINITIONS})
INCLUDE_DIRECTORIES(${CampvisGlobalIncludeDirs})
TARGET_LINK_LIBRARIES(campvis-core tgt ${CampvisGlobalExternalLibs} ${QT_LIBRARIES})
