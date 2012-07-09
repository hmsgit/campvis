PROJECT(tumvis-core)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
INCLUDE(../cmake/commonconf.cmake)

MESSAGE(STATUS "Configuring TUMVis Core")

FILE(GLOB TUMVIS_CORE_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    datastructures/*.cpp
    eventhandler/*.cpp
    pipeline/*.cpp
    tools/*.cpp
)

FILE(GLOB TUMVIS_CORE_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    datastructures/*.h
    eventhandler/*.h
    pipeline/*.h
	tools/*.h
)

#SET(TUMVIS_CORE_SOURCES
#    application/tumvis.cpp
#    application/tumvispainter.cpp
#    datastructures/
#)

#SET(TUMVIS_CORE_HEADERS 
#    application/tumvispainter.h
#)

#
# Qt related stuff:
#
FIND_PACKAGE(Qt4 ${TUMVIS_REQUIRED_QT_VERSION} REQUIRED QtCore QtGui QtOpenGL)
#QT4_WRAP_CPP(TUMVIS_CORE_MOC ${TUMVIS_CORE_HEADERS})
INCLUDE(${QT_USE_FILE})


ADD_LIBRARY(tumvis-core 
    ${TUMVIS_CORE_SOURCES} ${TUMVIS_CORE_HEADERS} 
#    ${TUMVIS_CORE_MOC}
)
ADD_DEFINITIONS(${TUMVIS_DEFINITIONS} ${QT_DEFINITIONS})
INCLUDE_DIRECTORIES(${TUMVIS_INCLUDE_DIRECTORIES})
TARGET_LINK_LIBRARIES(tumvis-core tgt ${TUMVIS_EXTERNAL_LIBRARIES} ${QT_LIBRARIES})
