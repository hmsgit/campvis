PROJECT(campvis-modules)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
INCLUDE(../cmake/commonconf.cmake)

MESSAGE(STATUS "Configuring CAMPVis Modules")

FILE(GLOB CAMPVIS_MODULES_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    classification/*.cpp
    io/*.cpp
    pipelines/*.cpp
    preprocessing/*.cpp
    vis/*.cpp
)

FILE(GLOB CAMPVIS_MODULES_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    classification/*.h
    io/*.h
    pipelines/*.h
    preprocessing/*.h
    vis/*.h
    vis/*.frag
    vis/*.cl
)

#SET(CAMPVIS_MODULES_SOURCES
#    application/campvis.cpp
#    application/campvispainter.cpp
#    datastructures/
#)

#SET(CAMPVIS_MODULES_HEADERS 
#    application/campvispainter.h
#)

#
# Qt related stuff:
#
FIND_PACKAGE(Qt4 ${CAMPVIS_REQUIRED_QT_VERSION} REQUIRED QtCore QtGui QtOpenGL)
#QT4_WRAP_CPP(CAMPVIS_MODULES_MOC ${CAMPVIS_MODULES_HEADERS})
INCLUDE(${QT_USE_FILE})

ADD_LIBRARY(campvis-modules
    ${CAMPVIS_MODULES_SOURCES} ${CAMPVIS_MODULES_HEADERS} 
#    ${CAMPVIS_MODULES_MOC}
) 
ADD_DEFINITIONS(${CAMPVIS_DEFINITIONS} ${QT_DEFINITIONS})
INCLUDE_DIRECTORIES(${CAMPVIS_INCLUDE_DIRECTORIES})
TARGET_LINK_LIBRARIES(campvis-modules campvis-core tgt kisscl ${CAMPVIS_EXTERNAL_LIBRARIES} ${QT_LIBRARIES})
