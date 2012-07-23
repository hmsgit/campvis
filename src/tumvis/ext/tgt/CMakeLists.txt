################################################################################
# Project file for the tgt library
################################################################################
PROJECT(tgt)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
INCLUDE(../../cmake/commonconf.cmake)

MESSAGE(STATUS "Configuring tgt Library")

#
# Qt related stuff:
#
FIND_PACKAGE(Qt4 ${TUMVIS_REQUIRED_QT_VERSION} REQUIRED QtCore QtGui QtOpenGL)
INCLUDE(${QT_USE_FILE})

# configuration
IF(TGT_WITH_WMI)
	ADD_DEFINITIONS("-DTGT_WITH_WMI")
	ADD_DEFINITIONS("-D_WIN32_DCOM") #< for CoInitializeSecurity() to be defined
	LIST(APPEND TUMVIS_EXTERNAL_LIBRARIES "WbemUuid")
ENDIF(TGT_WITH_WMI)    

# headers
FILE(GLOB TGT_HEADERS *.h event/*.h navigation/*.h qt/*.h)

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
	glcontext.cpp
	gpucapabilities.cpp
	gpucapabilitieswindows.cpp
	guiapplication.cpp
	init.cpp
	light.cpp
	logmanager.cpp
	naturalcubicspline.cpp
	painter.cpp
	quadrenderer.cpp
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
#event
	event/eventhandler.cpp
	event/eventlistener.cpp
	event/keyevent.cpp
	event/mouseevent.cpp
	event/timeevent.cpp
#navigation 
	navigation/navigation.cpp
	navigation/trackball.cpp
#qt
	qt/qtapplication.cpp
	qt/qtcanvas.cpp
	qt/qtcontextmanager.cpp
	qt/qtglcontext.cpp
	qt/qtthreadedcanvas.cpp
	qt/qtthreadedpainter.cpp
	qt/qttimer.cpp
)

################################################################################
# define library target
################################################################################
ADD_LIBRARY(tgt ${TGT_SOURCES} ${TGT_HEADERS})
ADD_DEFINITIONS(${TUMVIS_DEFINITIONS} ${TUMVIS_MODULE_DEFINITIONS} ${QT_DEFINITIONS})
IF(TUMVIS_SHARED_LIBS AND MSVC)
	ADD_DEFINITIONS("-DTGT_BUILD_DLL")
ENDIF()
INCLUDE_DIRECTORIES(${TUMVIS_INCLUDE_DIRECTORIES} ${TUMVIS_MODULE_INCLUDE_DIRECTORIES})
TARGET_LINK_LIBRARIES(tgt ${TUMVIS_EXTERNAL_LIBRARIES} ${QT_LIBRARIES})

################################################################################
# deployment
################################################################################
IF(TUMVIS_ADD_INSTALL_TARGET)
	INSTALL(TARGETS tgt 
		RUNTIME DESTINATION .
	)
ENDIF()

