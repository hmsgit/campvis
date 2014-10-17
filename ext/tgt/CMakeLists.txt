################################################################################
# Project file for the tgt library
################################################################################
PROJECT(tgt)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
INCLUDE(../../cmake/commonconf.cmake)

MESSAGE(STATUS "Configuring tgt Library")

# configuration
IF(TGT_WITH_WMI)
	ADD_DEFINITIONS("-DTGT_WITH_WMI")
	ADD_DEFINITIONS("-D_WIN32_DCOM") #< for CoInitializeSecurity() to be defined
	LIST(APPEND CampvisGlobalExternalLibs "WbemUuid")
ENDIF(TGT_WITH_WMI)


# headers
FILE(GLOB TGT_HEADERS *.h event/*.h navigation/*.h)

# sources
SET(TGT_SOURCES
	assert.cpp
	bounds.cpp
	bspline.cpp
	buffer.cpp
	camera.cpp
	catmullromspline.cpp
	curve.cpp
	exception.cpp
	filesystem.cpp
	font.cpp
	framebufferobject.cpp
	frustum.cpp
	glcanvas.cpp
	glcontextmanager.cpp
	gpucapabilities.cpp
	gpucapabilitieswindows.cpp
	guiapplication.cpp
	init.cpp
	light.cpp
	logmanager.cpp
	job.cpp
	naturalcubicspline.cpp
	openglgarbagecollector.cpp
	opengljobprocessor.cpp
	painter.cpp
	runnable.cpp
	shadermanager.cpp
	spline.cpp
	stopwatch.cpp
	tesselator.cpp
	texture.cpp
	texturemanager.cpp
	texturereader.cpp
	texturereaderdevil.cpp
	texturereadertga.cpp
	textureunit.cpp
	tgt_gl.cpp
	timer.cpp
	vertexarrayobject.cpp
#event
	event/eventhandler.cpp
	event/eventlistener.cpp
	event/keyevent.cpp
	event/mouseevent.cpp
	event/timeevent.cpp
#navigation 
	navigation/navigation.cpp
	navigation/trackball.cpp

)


# Qt related stuff:
IF(TGT_WITH_QT)
	LIST(APPEND TGT_HEADERS 
		qt/qtapplication.h
		qt/qtcanvas.h
		qt/qtthreadedcanvas.h
		qt/qttimer.h)
	
	LIST(APPEND TGT_SOURCES 
		qt/qtapplication.cpp
		qt/qtcanvas.cpp
		qt/qtthreadedcanvas.cpp
		qt/qttimer.cpp)
ENDIF(TGT_WITH_QT)


################################################################################
# define library target
################################################################################
ADD_LIBRARY(tgt ${TGT_SOURCES} ${TGT_HEADERS})
ADD_DEFINITIONS(${CampvisGlobalDefinitions} ${CampvisModulesDefinitions} ${QT_DEFINITIONS})
INCLUDE_DIRECTORIES(${CampvisGlobalIncludeDirs} ${CampvisModulesIncludeDirs})
TARGET_LINK_LIBRARIES(tgt ${CampvisGlobalExternalLibs} ${QT_LIBRARIES})

# if TGT is built as a shared library, CMake will define the following flag to instruct the code to
# export DLL symbols
SET_TARGET_PROPERTIES(tgt PROPERTIES DEFINE_SYMBOL "TGT_BUILD_DLL")


################################################################################
# deployment
################################################################################
IF(CAMPVIS_ADD_INSTALL_TARGET)
	INSTALL(TARGETS tgt 
		RUNTIME DESTINATION .
	)
ENDIF()

