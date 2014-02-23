################################################################################
# Project file for the sigslot library
################################################################################
PROJECT(sigslot)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
INCLUDE(../../cmake/commonconf.cmake)

MESSAGE(STATUS "Configuring sigslot Library")

# configuration


# headers
SET(SIGSLOT_HEADERS sigslot.h)

# sources
SET(SIGSLOT_SOURCES sigslot.cpp)

################################################################################
# define library target
################################################################################
ADD_LIBRARY(sigslot ${SIGSLOT_HEADERS} ${SIGSLOT_SOURCES})
ADD_DEFINITIONS(${CampvisGlobalDefinitions})
IF(CAMPVIS_SHARED_LIBS AND MSVC)
	ADD_DEFINITIONS("-DTGT_BUILD_DLL")
ENDIF()
INCLUDE_DIRECTORIES(${CampvisGlobalIncludeDirs})
TARGET_LINK_LIBRARIES(sigslot tgt)

################################################################################
# deployment
################################################################################
IF(CAMPVIS_ADD_INSTALL_TARGET)
	INSTALL(TARGETS sigslot 
		RUNTIME DESTINATION .
	)
ENDIF()

