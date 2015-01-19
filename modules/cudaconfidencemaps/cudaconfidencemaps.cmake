# modules/cudaconfidencemaps/cudaconfidencemaps.cmake
# CMake file for the cuda confidence maps module

# Set module status (valid values are STABLE, TESTING and EXPERIMENTAL)
SET(ThisModStatus EXPERIMENTAL)
# Set whether this module has external dependencies that are not shipped with CAMPVis.
SET(ThisModExternalDependencies TRUE)

# The files and assignments need only to be parsed if the module is enabled
IF(ModuleEnabled)
    FIND_PACKAGE(CUDA REQUIRED)

    IF(CUDA_FOUND)
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

        # Define the GLSL shader path, so that all needed shaders will be deployed to target directory
        SET(ThisModShaderDirectories "modules/cudaconfidencemaps/glsl")

        # Define dependency modules
        SET(ThisModDependencies preprocessing advancedusvis openigtlink fontrendering)

        IF(WIN32)
            SET(CUDA_PROPAGATE_HOST_FLAGS ON)
        ELSE()
            # Otherwise -std=c++11 is passed to the compiler on linux. However official
            # c++11 support for cuda is only available in CUDA 7
            SET(CUDA_PROPAGATE_HOST_FLAGS OFF)
        ENDIF()

        # Enable optimizations when building a release version
        IF(CMAKE_BUILD_TYPE MATCHES RELEASE)
            SET(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-O3)
        ENDIF()

        # CUSP Include directory
        CUDA_INCLUDE_DIRECTORIES(${ThisModDir}/ext/cusplibrary-0.4.0)

        # Build CUDA portion of the code (STATICALLY!?)
        FILE(GLOB cuda_SOURCES modules/cudaconfidencemaps/core/*.cu)
        CUDA_ADD_LIBRARY(cudaconfidencemaps-cuda STATIC
            ${cuda_SOURCES}
            )        

        # Make sure code can find the CUSP include files included with this module
        #SET(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};"-I ${ThisModDir}/ext/cusplibrary-0.4.0")
        set(CUDA_NVCC_FLAGS "ajkladjfl" CACHE STRING "adsf")

        # Link CUDA code to module
        LIST(APPEND ThisModExternalLibs cudaconfidencemaps-cuda)
    ELSE()
        MESSAGE(FATAL_ERROR "Could not find CUDA SDK.")
    ENDIF()

ENDIF(ModuleEnabled)