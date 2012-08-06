PROJECT(tumvis-application)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
INCLUDE(../cmake/commonconf.cmake)

MESSAGE(STATUS "Configuring TUMVis Application")

FILE(GLOB TUMVIS_APPLICATION_SOURCES
    *.cpp
    gui/*.cpp
    gui/properties/*.cpp
)

FILE(GLOB TUMVIS_APPLICATION_HEADERS
    *.h
    gui/*.h
    gui/properties/*.h
)

SET(TUMVIS_APPLICATION_TO_BE_MOCCED
    gui/mainwindow.h
    gui/pipelinetreewidget.h
    gui/qtcolortools.h
    gui/properties/abstractpropertywidget.h
    gui/properties/abstracttransferfunctioneditor.h
    gui/properties/intpropertywidget.h
    gui/properties/propertycollectionwidget.h
    gui/properties/simpletransferfunctioneditor.h
    gui/properties/stringpropertywidget.h
    gui/properties/transferfunctionpropertywidget.h
)

#
# Qt related stuff:
#
FIND_PACKAGE(Qt4 ${TUMVIS_REQUIRED_QT_VERSION} REQUIRED QtCore QtGui QtOpenGL)
QT4_WRAP_CPP(TUMVIS_APPLICATION_MOC ${TUMVIS_APPLICATION_TO_BE_MOCCED})
LIST(APPEND TUMVIS_APPLICATION_SOURCES ${TUMVIS_APPLICATION_MOC})

INCLUDE(${QT_USE_FILE})
ADD_EXECUTABLE(tumvis-application 
    ${TUMVIS_APPLICATION_SOURCES} ${TUMVIS_APPLICATION_HEADERS} 
    ${TUMVIS_APPLICATION_MOC}
) 
ADD_DEFINITIONS(${TUMVIS_DEFINITIONS} ${QT_DEFINITIONS})
INCLUDE_DIRECTORIES(${TUMVIS_INCLUDE_DIRECTORIES})
TARGET_LINK_LIBRARIES(tumvis-application tumvis-core tumvis-modules tgt ${TUMVIS_EXTERNAL_LIBRARIES} ${QT_LIBRARIES})
