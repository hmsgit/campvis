################################################################################
# Project file for the cgt library
################################################################################
PROJECT(cgt)
INCLUDE(../../cmake/commonconf.cmake)

MESSAGE(STATUS "Configuring cgt Library")

# configuration
IF(CGT_WITH_WMI)
	ADD_DEFINITIONS("-DCGT_WITH_WMI")
	ADD_DEFINITIONS("-D_WIN32_DCOM") #< for CoInitializeSecurity() to be defined
	LIST(APPEND CampvisGlobalExternalLibs "WbemUuid")
ENDIF(CGT_WITH_WMI)


# headers
FILE(GLOB CGT_HEADERS *.h event/*.h navigation/*.h)

# sources
SET(CGT_SOURCES
	assert.cpp
	bounds.cpp
	buffer.cpp
	camera.cpp
	exception.cpp
	filesystem.cpp
	framebufferobject.cpp
	frustum.cpp
	glcanvas.cpp
	glcontextmanager.cpp
	gpucapabilities.cpp
	gpucapabilitieswindows.cpp
	init.cpp
	logmanager.cpp
	job.cpp
	opengljobprocessor.cpp
	painter.cpp
	runnable.cpp
	shadermanager.cpp
	stopwatch.cpp
	texture.cpp
	texturereader.cpp
	texturereadertga.cpp
	textureunit.cpp
	cgt_gl.cpp
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
IF(CGT_WITH_QT)
	LIST(APPEND CGT_HEADERS 
		qt/qtcanvas.h
		qt/qtthreadedcanvas.h
		qt/qttimer.h)
	
	LIST(APPEND CGT_SOURCES 
		qt/qtcanvas.cpp
		qt/qtthreadedcanvas.cpp
		qt/qttimer.cpp)
ENDIF(CGT_WITH_QT)


################################################################################
# define library target
################################################################################
ADD_LIBRARY(cgt ${CGT_SOURCES} ${CGT_HEADERS})
ADD_DEFINITIONS(${CampvisGlobalDefinitions} ${CampvisModulesDefinitions})
INCLUDE_DIRECTORIES(${CampvisGlobalIncludeDirs} ${CampvisModulesIncludeDirs})
TARGET_LINK_LIBRARIES(cgt ${CampvisGlobalExternalLibs} Qt5::Widgets Qt5::OpenGL)

# if CGT is built as a shared library, CMake will define the following flag to instruct the code to
# export DLL symbols
SET_TARGET_PROPERTIES(cgt PROPERTIES DEFINE_SYMBOL "CGT_BUILD_DLL")


################################################################################
# deployment
################################################################################
IF(CAMPVIS_ADD_INSTALL_TARGET)
	INSTALL(TARGETS cgt 
		RUNTIME DESTINATION .
	)
ENDIF()


INSTALL(TARGETS cgt DESTINATION exports EXPORT campvis-targets)
