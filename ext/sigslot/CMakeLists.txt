################################################################################
# Project file for the sigslot library
################################################################################
PROJECT(sigslot)
MESSAGE(STATUS "Configuring sigslot Library")

# headers
SET(SIGSLOT_HEADERS sigslot.h sigslot.i)

# sources
SET(SIGSLOT_SOURCES sigslot.cpp)


################################################################################
# define library target
################################################################################
ADD_LIBRARY(sigslot ${SIGSLOT_HEADERS} ${SIGSLOT_SOURCES})
ADD_DEFINITIONS(${CampvisGlobalDefinitions})
INCLUDE_DIRECTORIES(${CampvisGlobalIncludeDirs})
TARGET_LINK_LIBRARIES(sigslot cgt)

# if campvis-core is built as a shared library, CMake will define the following flag to instruct
# the code to export DLL symbols
SET_TARGET_PROPERTIES(sigslot PROPERTIES DEFINE_SYMBOL "SIGSLOT_BUILD_DLL")

################################################################################
# deployment
################################################################################
IF(CAMPVIS_ADD_INSTALL_TARGET)
	INSTALL(TARGETS sigslot 
		RUNTIME DESTINATION .
	)
ENDIF()


INSTALL(TARGETS sigslot DESTINATION exports EXPORT campvis-targets)
