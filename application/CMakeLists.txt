PROJECT(campvis-application)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
INCLUDE(../cmake/commonconf.cmake)

MESSAGE(STATUS "Configuring CAMPVis-Application")

FILE(GLOB CampvisApplicationSources
    campvisapplication.cpp
    gui/*.cpp
    gui/adjusterwidgets/*.cpp
    gui/mdi/*.cpp
    gui/properties/*.cpp
    tools/*.cpp
)

FILE(GLOB CampvisApplicationHeaders
    applicationapi.h
    campvisapplication.h
    glsl/*.frag
    glsl/*.vert
    gui/*.h
    gui/properties/*.h
    gui/adjusterwidgets/*.h
    tools/*.h
)

FILE(GLOB CampvisApplicationForms
    ui/*.ui
)

SET(CampvisApplicationToBeMocced
    gui/completinglualineedit.h
    gui/mainwindow.h
    gui/datacontainerinspectorcanvas.h
    gui/datacontainerinspectorwidget.h
    gui/datacontainerfileloaderwidget.h
    gui/datacontainertreewidget.h
    gui/pipelinetreewidget.h
    gui/qtcolortools.h
    gui/qtdatahandle.h
    gui/logviewerwidget.h
    gui/loghighlighter.h
    gui/luatablewidget.h
    gui/scriptingwidget.h
    gui/workflowcontrollerwidget.h
    gui/mdi/mdidockarea.h
    gui/mdi/mdidockablewindow.h
    gui/mdi/mdidockedwindow.h
    gui/mdi/mdifloatingwindow.h
    gui/adjusterwidgets/doubleadjusterwidget.h
    gui/adjusterwidgets/intadjusterwidget.h
    gui/properties/abstractpropertywidget.h
    gui/properties/abstracttransferfunctioneditor.h
    gui/properties/boolpropertywidget.h
    gui/properties/buttonpropertywidget.h
    gui/properties/datanamepropertywidget.h
    gui/properties/intpropertywidget.h
    gui/properties/floatpropertywidget.h
    gui/properties/colorpropertywidget.h
    gui/properties/geometry1dtransferfunctioneditor.h
    gui/properties/geometry2dtransferfunctioneditor.h
    gui/properties/metapropertywidget.h
    gui/properties/optionpropertywidget.h
    gui/properties/propertycollectionwidget.h
    gui/properties/simpletransferfunctioneditor.h
    gui/properties/stringpropertywidget.h
    gui/properties/statuspropertywidget.h
    gui/properties/transferfunctionpropertywidget.h
    tools/bufferinglog.h
    tools/qtexteditlog.h
    tools/qtjobprocessor.h
)


# add additional sources from the activated modules
FOREACH(ModFile ${CampvisModulesApplicationSources})
    # adjust relative path
    SET(ModFileRelative ../modules/${ModFile})
    LIST(APPEND CampvisApplicationSources ${ModFileRelative})
ENDFOREACH()
FOREACH(ModFile ${CampvisModulesApplicationHeaders})
    # adjust relative path
    SET(ModFileRelative ../modules/${ModFile})
    LIST(APPEND CampvisApplicationHeaders ${ModFileRelative})
ENDFOREACH()
FOREACH(ModFile ${CampvisModulesApplicationToBeMocced})
    # adjust relative path
    SET(ModFileRelative ../modules/${ModFile})
    LIST(APPEND CampvisApplicationToBeMocced ${ModFileRelative})
ENDFOREACH()

#
# Qt related stuff:
#
QT4_WRAP_CPP(CampvisApplicationMoc ${CampvisApplicationToBeMocced})
LIST(APPEND CampvisApplicationSources ${CampvisApplicationMoc})

QT4_WRAP_UI(CampvisApplicationFormsHeaders ${CampvisApplicationForms})
LIST(APPEND CampvisApplicationSources ${CampvisApplicationFormsHeaders})

LINK_DIRECTORIES(${CampvisGlobalLinkDirectories} ${CampvisModulesLinkDirectories})

SET(CampvisMainLibs campvis-core campvis-modules cgt sigslot)

IF(CAMPVIS_ENABLE_SCRIPTING)
    LIST(APPEND CampvisMainLibs campvis-scripting)
ENDIF(CAMPVIS_ENABLE_SCRIPTING)

ADD_LIBRARY(campvis-application
    ${CampvisApplicationSources} ${CampvisApplicationHeaders}
    ${CampvisApplicationMoc}
)
ADD_DEFINITIONS(${CampvisGlobalDefinitions} ${CampvisModulesDefinitions} ${CampvisApplicationDefinitions} ${QT_DEFINITIONS})
INCLUDE_DIRECTORIES(${CampvisGlobalIncludeDirs} ${CampvisModulesIncludeDirs})
TARGET_LINK_LIBRARIES(campvis-application ${CampvisMainLibs} ${CampvisGlobalExternalLibs} ${CampvisModulesExternalLibs} ${QT_LIBRARIES})

# if campvis-core is built as a shared library, CMake will define the following flag to instruct
# the code to export DLL symbols
SET_TARGET_PROPERTIES(campvis-application PROPERTIES DEFINE_SYMBOL "CAMPVIS_APPLICATION_BUILD_DLL")

IF(CAMPVIS_GROUP_SOURCE_FILES)
    DEFINE_SOURCE_GROUPS_FROM_SUBDIR(CampvisApplicationSources ${CampvisHome} "")
    DEFINE_SOURCE_GROUPS_FROM_SUBDIR(CampvisApplicationHeaders ${CampvisHome} "")
ENDIF()



ADD_EXECUTABLE(campvis-gui "campvis.cpp")
TARGET_LINK_LIBRARIES(campvis-gui campvis-application ${CampvisMainLibs} ${CampvisGlobalExternalLibs} ${CampvisModulesExternalLibs} ${QT_LIBRARIES})



IF(CAMPVIS_DEPLOY_SHADERS)
    LIST(APPEND CampvisShaderDirectories "application/data")
    LIST(APPEND CampvisShaderDirectories "application/glsl")
    LIST(APPEND CampvisShaderDirectories "core/glsl")

    FOREACH(Directory ${CampvisShaderDirectories})
        ADD_CUSTOM_COMMAND(
            TARGET campvis-application
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${CampvisHome}/${Directory}" "$<TARGET_FILE_DIR:campvis-application>/${Directory}"
        )
    ENDFOREACH()
ENDIF(CAMPVIS_DEPLOY_SHADERS)


# copy over Lua stuff
ADD_CUSTOM_COMMAND(
    TARGET campvis-application
    COMMAND ${CMAKE_COMMAND} -E copy "${CampvisHome}/application/lua/inspect.lua" "$<TARGET_FILE_DIR:campvis-application>/lua-mods/inspect.lua"
)
