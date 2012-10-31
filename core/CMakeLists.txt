PROJECT(campvis-core)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
INCLUDE(../cmake/commonconf.cmake)

MESSAGE(STATUS "Configuring CAMPVis Core")

FILE(GLOB CAMPVIS_CORE_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    classification/*.cpp
    datastructures/*.cpp
    eventhandlers/*.cpp
    pipeline/*.cpp
    properties/*.cpp
    tools/*.cpp
)

FILE(GLOB CAMPVIS_CORE_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
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

#SET(CAMPVIS_CORE_SOURCES
#    application/campvis.cpp
#    application/campvispainter.cpp
#    datastructures/
#)

#SET(CAMPVIS_CORE_HEADERS 
#    application/campvispainter.h
#)

#
# Qt related stuff:
#
FIND_PACKAGE(Qt4 ${CAMPVIS_REQUIRED_QT_VERSION} REQUIRED QtCore QtGui QtOpenGL)
#QT4_WRAP_CPP(CAMPVIS_CORE_MOC ${CAMPVIS_CORE_HEADERS})
INCLUDE(${QT_USE_FILE})


ADD_LIBRARY(campvis-core 
    ${CAMPVIS_CORE_SOURCES} ${CAMPVIS_CORE_HEADERS} 
#    ${CAMPVIS_CORE_MOC}
)
ADD_DEFINITIONS(${CAMPVIS_DEFINITIONS} ${QT_DEFINITIONS})
INCLUDE_DIRECTORIES(${CAMPVIS_INCLUDE_DIRECTORIES})
TARGET_LINK_LIBRARIES(campvis-core tgt kisscl ${CAMPVIS_EXTERNAL_LIBRARIES} ${QT_LIBRARIES})
