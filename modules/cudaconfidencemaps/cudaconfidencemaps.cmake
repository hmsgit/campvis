# CMake file for vis module

IF(${ModuleEnabled})
    # Find CUDA
    FIND_PACKAGE(CUDA REQUIRED)

    if(CUDA_FOUND)
        # Source files:
        FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
            modules/cudaconfidencemaps/core/*.cpp
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

        IF(NOT WIN32)
            # Otherwise -std=c++11 is passed to the compiler on linux. However official
            # c++11 support for cuda is only available in CUDA 7
            set(CUDA_PROPAGATE_HOST_FLAGS OFF)
        ENDIF()

        IF(CMAKE_BUILD_TYPE MATCHES RELEASE)
            # Enable optimizations when building a release version
            set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-O3)
        ENDIF()

        # Build CUDA sources
        file(GLOB cuda_SOURCES modules/cudaconfidencemaps/core/*.cu)
        CUDA_ADD_LIBRARY(CudaConfidenceMaps_CUDA STATIC
            ${cuda_SOURCES}
            )        

        # Make sure code can find the CUSP include files included with this module
        set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};"-I ${ThisModDir}/ext/cusplibrary-0.4.0")

        LIST(APPEND ThisModExternalLibs CudaConfidenceMaps_CUDA)

        SET(ThisModShaderDirectories "modules/cudaconfidencemaps/glsl")
        SET(ThisModDependencies base io)
    else()
        MESSAGE(FATAL_ERROR "Could not find CUDA SDK.")
    endif()

ENDIF(${ModuleEnabled})

SET(ThisModStatus EXPERIMENTAL)
SET(ThisModExternalDependencies TRUE)
