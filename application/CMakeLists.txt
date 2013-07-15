PROJECT(campvis-application)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
INCLUDE(../cmake/commonconf.cmake)

MESSAGE(STATUS "Configuring CAMPVis Application")

FILE(GLOB CampvisApplicationSources
    *.cpp
    gui/*.cpp
    gui/properties/*.cpp
    tools/*.cpp
)

FILE(GLOB CampvisApplicationHeaders
    *.h
    glsl/*.frag
    glsl/*.vert
    gui/*.h
    gui/properties/*.h
    tools/*.h
)

FILE(GLOB CampvisApplicationForms
    ui/*.ui
)

SET(CampvisApplicationToBeMocced
    gui/mainwindow.h
    gui/datacontainerinspectorcanvas.h
    gui/datacontainerinspectorwidget.h
    gui/datacontainertreewidget.h
    gui/pipelinetreewidget.h
    gui/qtcolortools.h
    gui/qtdatahandle.h
    gui/logviewerwidget.h
    gui/loghighlighter.h
    gui/properties/abstractpropertywidget.h
    gui/properties/abstracttransferfunctioneditor.h
    gui/properties/boolpropertywidget.h
    gui/properties/buttonpropertywidget.h
    gui/properties/camerapropertywidget.h
    gui/properties/intpropertywidget.h
    gui/properties/floatpropertywidget.h
    gui/properties/geometry1dtransferfunctioneditor.h
    gui/properties/geometry2dtransferfunctioneditor.h
    gui/properties/optionpropertywidget.h
    gui/properties/propertycollectionwidget.h
    gui/properties/simpletransferfunctioneditor.h
    gui/properties/stringpropertywidget.h
    gui/properties/transferfunctionpropertywidget.h
    tools/qtexteditlog.h
    tools/bufferinglog.h
)

LIST(APPEND CampvisApplicationDefinitions "-DCAMPVIS_SOURCE_DIR=\"${CampvisHome}\"")

#
# Qt related stuff:
#
FIND_PACKAGE(Qt4 ${CampvisRequiredQtVersion} REQUIRED QtCore QtGui QtOpenGL)
QT4_WRAP_CPP(CampvisApplicationMoc ${CampvisApplicationToBeMocced})
LIST(APPEND CampvisApplicationSources ${CampvisApplicationMoc})
QT4_WRAP_UI(CampvisApplicationFormsHeaders ${CampvisApplicationForms})
LIST(APPEND CampvisApplicationSources ${CampvisApplicationFormsHeaders})
INCLUDE(${QT_USE_FILE})

LINK_DIRECTORIES(${CampvisGlobalLinkDirectories} ${CampvisModulesLinkDirectories})

ADD_EXECUTABLE(campvis-application
    ${CampvisApplicationSources} ${CampvisApplicationHeaders}
    ${CampvisApplicationMoc}
)
ADD_DEFINITIONS(${CampvisGlobalDefinitions} ${CampvisModulesDefinitions} ${CampvisApplicationDefinitions} ${QT_DEFINITIONS})
INCLUDE_DIRECTORIES(${CampvisGlobalIncludeDirs} ${CampvisModulesIncludeDirs})
TARGET_LINK_LIBRARIES(campvis-application campvis-core campvis-modules tgt ${CampvisGlobalExternalLibs} ${CampvisModulesExternalLibs} ${QT_LIBRARIES})

IF(CAMPVIS_DEPLOY_SHADERS)
    LIST(APPEND CampvisShaderDirectories "application/glsl")
    LIST(APPEND CampvisShaderDirectories "core/glsl")

    FOREACH(Directory ${CampvisShaderDirectories})
        ADD_CUSTOM_COMMAND(
            TARGET campvis-application
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${CampvisHome}/${Directory}" "$<TARGET_FILE_DIR:campvis-application>/${Directory}"
        )
    ENDFOREACH()
ENDIF(CAMPVIS_DEPLOY_SHADERS)
