# CMake file for vis module

IF(${ModuleEnabled})
	# Find CUDA
    FIND_PACKAGE(CUDA REQUIRED)

    if(CUDA_FOUND)
		# Source files:
		FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
			modules/cudaconfidencemaps/pipelines/*.cpp
			modules/cudaconfidencemaps/processors/*.cpp
		)

		# Header files (including GLSL files so that they'll appear in VS projects)
		FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
			modules/cudaconfidencemaps/glsl/*.frag
			modules/cudaconfidencemaps/glsl/*.geom
			modules/cudaconfidencemaps/glsl/*.vert
			modules/cudaconfidencemaps/pipelines/*.h
			modules/cudaconfidencemaps/processors/*.h
		)

		SET(ThisModShaderDirectories "modules/cudaconfidencemaps/glsl")
		SET(ThisModDependencies base io)
	else()
		MESSAGE(FATAL_ERROR "Could not find CUDA SDK.")
	endif()
ENDIF(${ModuleEnabled})

SET(ThisModStatus EXPERIMENTAL)
SET(ThisModExternalDependencies TRUE)
