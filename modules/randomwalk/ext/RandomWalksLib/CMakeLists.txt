################################################################################
# Project file for the RandomWalksLib library
################################################################################
PROJECT(RandomWalksLib)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
MESSAGE(STATUS "  Configuring external RandomWalksLib Library")

#all headers
SET(headers ConfidenceMaps2D.h
			ConfidenceMaps2DFacade.h
			RandomWalks2D.h
			RandomWalks2DFacade.h
			RandomWalks3D.h
			RandomWalks3DFacade.h
			RandomWalksCore.h
			SparseSolverEigenCG.h
			SparseSolverEigenCustom.h
			SparseSolverEigenLLT.h
			SparseSolverFactory.h
			SparseSolverInterface.h
			SparseSolverViennaCPU.h
			SparseSolverViennaGPU.h)

SET(sources ConfidenceMaps2D.cpp
			ConfidenceMaps2DFacade.cpp
			RandomWalks2D.cpp
			RandomWalks2DFacade.cpp
			RandomWalks3D.cpp
			RandomWalks3DFacade.cpp
			RandomWalksCore.cpp
			SparseSolverEigenCG.cpp
			SparseSolverEigenCustom.cpp
			SparseSolverEigenLLT.cpp
			SparseSolverFactory.cpp
			SparseSolverViennaCPU.cpp
			SparseSolverViennaGPU.cpp)

# Eigen headers are already in CAMPVis/ext
#include ViennaCl headers
INCLUDE_DIRECTORIES(${CampvisGlobalIncludeDirs})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES($ENV{NVSDKCOMPUTE_ROOT}/OpenCL/common/inc)

#path to OpenCl binaries
LINK_DIRECTORIES($ENV{CUDA_LIB_PATH})

################################################################################
# define library target
################################################################################
ADD_LIBRARY(RandomWalksLib STATIC ${sources} ${headers})

# We don't want this within CAMPVis...
#put debug target to x64/Debug and all other configurations to x64/Release
#SET_TARGET_PROPERTIES( RandomWalksLib PROPERTIES 
#						ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x64/Release #static libs are archives
#						ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/x64/Debug
#						ARCHIVE_OUTPUT_NAME RandomWalksLib
#						ARCHIVE_OUTPUT_NAME_DEBUG RandomWalksLibd ) #debug build should end with a 'd'
TARGET_LINK_LIBRARIES(RandomWalksLib OpenCL)
