################################################################################
# Project file for the cllib library
################################################################################
PROJECT(cllib)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
INCLUDE(../../cmake/commonconf.cmake)

MESSAGE(STATUS "Configuring cllib Library")

# headers
FILE(GLOB CLLIB_HEADERS *.h CL/*.hpp)

# sources
FILE(GLOB CLLIB_SOURCES *.cpp)


################################################################################
# define library target
################################################################################
ADD_LIBRARY(cllib ${CLLIB_SOURCES} ${CLLIB_HEADERS})
ADD_DEFINITIONS(${TUMVIS_DEFINITIONS} ${TUMVIS_MODULE_DEFINITIONS})
IF(TUMVIS_SHARED_LIBS AND MSVC)
	ADD_DEFINITIONS("-DCLLIB_BUILD_DLL")
ENDIF()
INCLUDE_DIRECTORIES(${TUMVIS_INCLUDE_DIRECTORIES} ${TUMVIS_MODULE_INCLUDE_DIRECTORIES})
TARGET_LINK_LIBRARIES(cllib ${TUMVIS_EXTERNAL_LIBRARIES})

################################################################################
# deployment
################################################################################
IF(TUMVIS_ADD_INSTALL_TARGET)
	INSTALL(TARGETS cllib 
		RUNTIME DESTINATION .
	)
ENDIF()

