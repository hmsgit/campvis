
########################################################################
#
# Project-wide settings

# Name of the project.
#
# CMake files in this project can refer to the root source directory
# as ${alglib_SOURCE_DIR} and to the root binary directory as
# ${alglib_BINARY_DIR}.
# Language "C" is required for find_package(Threads).
project(alglib CXX C)
cmake_minimum_required(VERSION 2.6.2)

MESSAGE(STATUS "  Configuring external library: Alglib")

# Where .h files can be found.
include_directories(
  ${alglib_SOURCE_DIR}
  ${alglib_SOURCE_DIR})

# Where libraries can be found.
link_directories(${gtest_BINARY_DIR})

LIST(APPEND AlglibHeaders
    ${alglib_SOURCE_DIR}/alglibinternal.h
    ${alglib_SOURCE_DIR}/alglibmisc.h
    ${alglib_SOURCE_DIR}/ap.h
#    ${alglib_SOURCE_DIR}/dataanalysis.h
    ${alglib_SOURCE_DIR}/diffequations.h
    ${alglib_SOURCE_DIR}/fasttransforms.h
    ${alglib_SOURCE_DIR}/integration.h
    ${alglib_SOURCE_DIR}/interpolation.h
    ${alglib_SOURCE_DIR}/linalg.h
    ${alglib_SOURCE_DIR}/optimization.h
    ${alglib_SOURCE_DIR}/solvers.h
    ${alglib_SOURCE_DIR}/specialfunctions.h
#    ${alglib_SOURCE_DIR}/statistics.h
    ${alglib_SOURCE_DIR}/stdafx.h
)
LIST(APPEND AlglibSources
    ${alglib_SOURCE_DIR}/alglibinternal.cpp
    ${alglib_SOURCE_DIR}/alglibmisc.cpp
    ${alglib_SOURCE_DIR}/ap.cpp
#    ${alglib_SOURCE_DIR}/dataanalysis.cpp
    ${alglib_SOURCE_DIR}/diffequations.cpp
    ${alglib_SOURCE_DIR}/fasttransforms.cpp
    ${alglib_SOURCE_DIR}/integration.cpp
    ${alglib_SOURCE_DIR}/interpolation.cpp
    ${alglib_SOURCE_DIR}/linalg.cpp
    ${alglib_SOURCE_DIR}/optimization.cpp
    ${alglib_SOURCE_DIR}/solvers.cpp
    ${alglib_SOURCE_DIR}/specialfunctions.cpp
#    ${alglib_SOURCE_DIR}/statistics.cpp
)


########################################################################

# Defines the alglib librarie.  

ADD_LIBRARY(alglib ${AlglibSources} ${AlglibHeaders})
#cxx_library(gtest_main "${cxx_strict}" src/gtest_main.cc)
#$target_link_libraries(gtest_main alglib)


if ( CMAKE_COMPILER_IS_GNUCC )
    # This is tested
    set_property( TARGET alglib APPEND_STRING PROPERTY COMPILE_FLAGS -Wall )
	SET(CMAKE_CXX_FLAGS "-fPIC -fno-strict-aliasing")
endif ( CMAKE_COMPILER_IS_GNUCC )
if ( MSVC )
    # This is untested
    set_property( TARGET alglib APPEND_STRING PROPERTY COMPILE_FLAGS "/W1 /EHsc" )
endif ( MSVC )

# define export targets
INSTALL(TARGETS alglib DESTINATION exports EXPORT campvis-targets)
