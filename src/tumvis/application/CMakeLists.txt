PROJECT(campvis-application)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
INCLUDE(../cmake/commonconf.cmake)

MESSAGE(STATUS "Configuring CAMPVis Application")

FILE(GLOB CAMPVIS_APPLICATION_SOURCES
    *.cpp
    gui/*.cpp
    gui/properties/*.cpp
    tools/*.cpp
)

FILE(GLOB CAMPVIS_APPLICATION_HEADERS
    *.h
    gui/*.h
    gui/properties/*.h
    tools/*.h
)

SET(CAMPVIS_APPLICATION_TO_BE_MOCCED
    gui/mainwindow.h
    gui/datacontainerinspectorcanvas.h
    gui/datacontainerinspectorwidget.h
    gui/datacontainertreewidget.h
    gui/pipelinetreewidget.h
    gui/qtcolortools.h
    gui/qtdatahandle.h
    gui/properties/abstractpropertywidget.h
    gui/properties/abstracttransferfunctioneditor.h
    gui/properties/boolpropertywidget.h
    gui/properties/intpropertywidget.h
    gui/properties/floatpropertywidget.h
    gui/properties/geometry1dtransferfunctioneditor.h
    gui/properties/geometry2dtransferfunctioneditor.h
    gui/properties/propertycollectionwidget.h
    gui/properties/simpletransferfunctioneditor.h
    gui/properties/stringpropertywidget.h
    gui/properties/transferfunctionpropertywidget.h
)

#
# Qt related stuff:
#
FIND_PACKAGE(Qt4 ${CAMPVIS_REQUIRED_QT_VERSION} REQUIRED QtCore QtGui QtOpenGL)
QT4_WRAP_CPP(CAMPVIS_APPLICATION_MOC ${CAMPVIS_APPLICATION_TO_BE_MOCCED})
LIST(APPEND CAMPVIS_APPLICATION_SOURCES ${CAMPVIS_APPLICATION_MOC})

INCLUDE(${QT_USE_FILE})
ADD_EXECUTABLE(campvis-application 
    ${CAMPVIS_APPLICATION_SOURCES} ${CAMPVIS_APPLICATION_HEADERS} 
    ${CAMPVIS_APPLICATION_MOC}
) 
ADD_DEFINITIONS(${CAMPVIS_DEFINITIONS} ${QT_DEFINITIONS})
INCLUDE_DIRECTORIES(${CAMPVIS_INCLUDE_DIRECTORIES})
TARGET_LINK_LIBRARIES(campvis-application campvis-core campvis-modules tgt kisscl ${CAMPVIS_EXTERNAL_LIBRARIES} ${QT_LIBRARIES})
