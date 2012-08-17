################################################################################
# Project file for the kisscl library
################################################################################
PROJECT(kisscl)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
INCLUDE(../../cmake/commonconf.cmake)

MESSAGE(STATUS "Configuring kisscl Library")

# headers
FILE(GLOB KISSCL_HEADERS *.h CL/*.hpp)

# sources
FILE(GLOB KISSCL_SOURCES *.cpp)


################################################################################
# define library target
################################################################################
ADD_LIBRARY(kisscl ${KISSCL_SOURCES} ${KISSCL_HEADERS})
ADD_DEFINITIONS(${TUMVIS_DEFINITIONS} ${TUMVIS_MODULE_DEFINITIONS})
IF(TUMVIS_SHARED_LIBS AND MSVC)
	ADD_DEFINITIONS("-DKISSCL_BUILD_DLL")
ENDIF()
INCLUDE_DIRECTORIES(${TUMVIS_INCLUDE_DIRECTORIES} ${TUMVIS_MODULE_INCLUDE_DIRECTORIES})
TARGET_LINK_LIBRARIES(kisscl ${TUMVIS_EXTERNAL_LIBRARIES})

################################################################################
# deployment
################################################################################
IF(TUMVIS_ADD_INSTALL_TARGET)
	INSTALL(TARGETS kisscl 
		RUNTIME DESTINATION .
	)
ENDIF()

