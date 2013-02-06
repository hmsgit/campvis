################################################################################
# Project file for the RandomWalksLib library
################################################################################
PROJECT(RandomWalksLib)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
MESSAGE(STATUS "  Configuring external RandomWalksLib Library")

# all RandomWalksLibHeaders except ViennaCL related stuff
SET(RandomWalksLibHeaders ConfidenceMaps2D.h
			ConfidenceMaps2DFacade.h
			RandomWalks2D.h
			RandomWalks2DFacade.h
			RandomWalks3D.h
			RandomWalks3DFacade.h
			RandomWalksCore.h
			SparseSolverEigenBiCGSTAB.h
			SparseSolverEigenCG.h
			SparseSolverEigenCustom.h
			SparseSolverEigenLLT.h
			SparseSolverFactory.h
			SparseSolverInterface.h)

SET(RandomWalksLibSources ConfidenceMaps2D.cpp
			ConfidenceMaps2DFacade.cpp
			RandomWalks2D.cpp
			RandomWalks2DFacade.cpp
			RandomWalks3D.cpp
			RandomWalks3DFacade.cpp
			RandomWalksCore.cpp
			SparseSolverEigenBiCGSTAB.cpp
			SparseSolverEigenCG.cpp
			SparseSolverEigenCustom.cpp
			SparseSolverEigenLLT.cpp
			SparseSolverFactory.cpp)

# Make OpenCL dependency optional
IF(OPENCL_FOUND)
	ADD_DEFINITIONS("-DRANDOMWALKSLIB_HAS_OPENCL")
    LIST(APPEND RandomWalksLibExternalLibs ${OPENCL_LIBRARY})
	LIST(APPEND RandomWalksLibHeaders SparseSolverViennaCPU.h SparseSolverViennaGPU.h)
	LIST(APPEND RandomWalksLibSources SparseSolverViennaCPU.cpp SparseSolverViennaGPU.cpp)
ENDIF()

################################################################################
# define library target
################################################################################
# Eigen RandomWalksLibHeaders are already in CAMPVis/ext
INCLUDE_DIRECTORIES(${CampvisGlobalIncludeDirs})
#include ViennaCl RandomWalksLibHeaders
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_LIBRARY(RandomWalksLib STATIC ${RandomWalksLibSources} ${RandomWalksLibHeaders})

# We don't want this within CAMPVis...
#put debug target to x64/Debug and all other configurations to x64/Release
#SET_TARGET_PROPERTIES( RandomWalksLib PROPERTIES 
#						ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x64/Release #static libs are archives
#						ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/x64/Debug
#						ARCHIVE_OUTPUT_NAME RandomWalksLib
#						ARCHIVE_OUTPUT_NAME_DEBUG RandomWalksLibd ) #debug build should end with a 'd'
TARGET_LINK_LIBRARIES(RandomWalksLib ${RandomWalksLibExternalLibs})
