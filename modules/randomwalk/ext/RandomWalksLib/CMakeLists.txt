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

################################################################################
# define library target
################################################################################

# Eigen RandomWalksLibHeaders are already in CAMPVis/ext
INCLUDE_DIRECTORIES(${CampvisGlobalIncludeDirs})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_LIBRARY(RandomWalksLib ${CampvisSharedStaticModulesFix} ${RandomWalksLibSources} ${RandomWalksLibHeaders})
TARGET_LINK_LIBRARIES(RandomWalksLib ${RandomWalksLibExternalLibs})
