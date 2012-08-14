PROJECT(tumvis-modules)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
INCLUDE(../cmake/commonconf.cmake)

MESSAGE(STATUS "Configuring TUMVis Modules")

FILE(GLOB TUMVIS_MODULES_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    io/*.cpp
    pipelines/*.cpp
    vis/*.cpp
)

FILE(GLOB TUMVIS_MODULES_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    io/*.h
    pipelines/*.h
    vis/*.h
    vis/*.frag
)

#SET(TUMVIS_MODULES_SOURCES
#    application/tumvis.cpp
#    application/tumvispainter.cpp
#    datastructures/
#)

#SET(TUMVIS_MODULES_HEADERS 
#    application/tumvispainter.h
#)

#
# Qt related stuff:
#
FIND_PACKAGE(Qt4 ${TUMVIS_REQUIRED_QT_VERSION} REQUIRED QtCore QtGui QtOpenGL)
#QT4_WRAP_CPP(TUMVIS_MODULES_MOC ${TUMVIS_MODULES_HEADERS})
INCLUDE(${QT_USE_FILE})

ADD_LIBRARY(tumvis-modules
    ${TUMVIS_MODULES_SOURCES} ${TUMVIS_MODULES_HEADERS} 
#    ${TUMVIS_MODULES_MOC}
) 
ADD_DEFINITIONS(${TUMVIS_DEFINITIONS} ${QT_DEFINITIONS})
INCLUDE_DIRECTORIES(${TUMVIS_INCLUDE_DIRECTORIES})
TARGET_LINK_LIBRARIES(tumvis-modules tumvis-core tgt cllib ${TUMVIS_EXTERNAL_LIBRARIES} ${QT_LIBRARIES})
