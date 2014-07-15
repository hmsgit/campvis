IF(NOT CommonconfProcessed)

SET(CampvisHome ${CMAKE_CURRENT_SOURCE_DIR})
SET(CampvisBinaryDir ${CMAKE_BINARY_DIR})
MESSAGE(STATUS "TUMVis Source Directory: ${CampvisHome}")
MESSAGE(STATUS "TUMVis Binary Directory: ${CampvisBinaryDir}")

# include macros
INCLUDE(${CampvisHome}/cmake/macros.cmake)

# detect compiler and architecture
IF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 9 2008")
    SET(CAMPVIS_MSVC2008 TRUE)
    SET(CAMPVIS_WIN32 TRUE)
    MESSAGE(STATUS "Visual Studio 2008 build (32 Bit)")
ELSEIF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 9 2008 Win64")
    SET(CAMPVIS_MSVC2008 TRUE)
    SET(CAMPVIS_WIN64 TRUE)
    MESSAGE(STATUS "Visual Studio 2008 Build (64 Bit)")
ELSEIF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 10")
    SET(CAMPVIS_MSVC2010 TRUE)
    SET(CAMPVIS_WIN32 TRUE)
    MESSAGE(STATUS "Visual Studio 2010 Build (32 Bit)")
ELSEIF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 10 Win64")
    SET(CAMPVIS_MSVC2010 TRUE)
    SET(CAMPVIS_WIN64 TRUE)
    MESSAGE(STATUS "Visual Studio 2010 Build (64 Bit)")
ELSEIF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 11")
    SET(CAMPVIS_MSVC11 TRUE)
    SET(CAMPVIS_WIN32 TRUE)
    MESSAGE("Visual Studio 11 Build (32 Bit)")
ELSEIF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 11 Win64")
    SET(CAMPVIS_MSVC11 TRUE)
    SET(CAMPVIS_WIN64 TRUE)
    MESSAGE("Visual Studio 11 Build (64 Bit)")
ELSEIF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 12")
    SET(CAMPVIS_MSVC12 TRUE)
    SET(CAMPVIS_WIN32 TRUE)
    MESSAGE("Visual Studio 12 Build (32 Bit)")
ELSEIF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 12 Win64")
    SET(CAMPVIS_MSVC12 TRUE)
    SET(CAMPVIS_WIN64 TRUE)
    MESSAGE("Visual Studio 12 Build (64 Bit)")
ELSEIF(${CMAKE_GENERATOR} MATCHES "NMake") 
    SET(CAMPVIS_NMAKE TRUE)

    # NMake-based builds may very well use a Visual Studio compiler
    IF(MSVC90)
        SET(CAMPVIS_MSVC2008 TRUE)
    ELSEIF(MSVC10)
        SET(CAMPVIS_MSVC2010 TRUE)
    ELSEIF(MSVC11)
        SET(CAMPVIS_MSVC11 TRUE)
    ELSEIF(MSVC12)
        SET(CAMPVIS_MSVC12 TRUE)
    ENDIF(MSVC90)

    IF(CMAKE_CL_64)
        SET(CAMPVIS_WIN64 TRUE)
        MESSAGE(STATUS "NMake 64 Bit Build")
    ELSE(CMAKE_CL_64)
        SET(CAMPVIS_WIN32 TRUE)
        MESSAGE(STATUS "NMake 32 Bit Build")            
    ENDIF(CMAKE_CL_64)    
ELSEIF(${CMAKE_GENERATOR} MATCHES "MinGW")
    SET(CAMPVIS_MINGW TRUE)
    SET(CAMPVIS_WIN32 TRUE)
    MESSAGE("MinGW 32 Bit Build (not actively supported)")
ELSEIF(${CMAKE_GENERATOR} MATCHES "Unix")
    SET(CAMPVIS_UNIX TRUE)
    MESSAGE(STATUS "Unix Build")
ELSE()
    MESSAGE(WARNING "Unsupported or unknown generator: ${CMAKE_GENERATOR}")
ENDIF()


# set binary output path
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# common include directories
LIST(APPEND CampvisGlobalIncludeDirs "${CampvisHome}/ext")
LIST(APPEND CampvisGlobalIncludeDirs ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}) 

LIST(APPEND CampvisGlobalDefinitions "-DCAMPVIS_SOURCE_DIR=\"${CampvisHome}\"")

# platform-dependent configuration
IF(WIN32)
    LIST(APPEND CampvisGlobalDefinitions "-DNOMINMAX" "-D_CRT_SECURE_NO_DEPRECATE")

    # Disable warnings for Microsoft compiler:
    # C4251  class needs to have dll interface (used for std classes)
    # C4290: C++ exception specification ignored except to indicate a function is
    #        not __declspec(nothrow)
    # C4390: ';' : empty controlled statement found; is this the intent?
    #        occurs when OpenGL error logging macros are disabled
    # C4503: The decorated name was longer than the compiler limit (4096), and was truncated.
    #        Occurs in AutoEvaluatePipeline due to some nested nested map-iterator-map. Could
    #        not be deactivated locally...
    # C4068: Unknown Pragma since we use some GCC pragmas in the code.
    LIST(APPEND CampvisGlobalDefinitions /wd4251 /wd4290 /wd4390 /wd4503 /wd4068)
    
    # enable parallel builds in Visual Studio
    LIST(APPEND CampvisGlobalDefinitions /MP)

    # prevent error: number of sections exceeded object file format limit
    LIST(APPEND CampvisGlobalDefinitions /bigobj)
    
    # allows 32 Bit builds to use more than 2GB RAM (VC++ only)
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LARGEADDRESSAWARE")
    SET(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE")

    IF(BUILD_SHARED_LIBS)
        # Linking against Windows DLLs requires explicit instantiation of templates
        LIST(APPEND CampvisGlobalDefinitions "-DDLL_TEMPLATE_INST")
        LIST(APPEND CampvisGlobalDefinitions "-DCAMPVIS_DYNAMIC_LIBS")

        IF(NOT CAMPVIS_GENERATE_MANIFEST)
            # Do not embed manifest into binaries in debug mode (slows down incremental linking)
            SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /MANIFEST:NO")
            SET(CMAKE_EXE_LINKER_FLAGS_DEBUG    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /MANIFEST:NO")
        ENDIF()
        
        # For Windows targets, we have to build campvis-modules and some other libs as static library, 
        # since we did not define the __declspec exports here.
        SET(CampvisSharedStaticModulesFix "STATIC")
    ENDIF()

    # enable/disable incremental linking in debug builds
    If(CAMPVIS_INCREMENTAL_LINKING)
        SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /INCREMENTAL")
        SET(CMAKE_EXE_LINKER_FLAGS_DEBUG    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /INCREMENTAL")
    ELSE()
        SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /INCREMENTAL:NO")
        SET(CMAKE_EXE_LINKER_FLAGS_DEBUG    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /INCREMENTAL:NO")
    ENDIF()
    
    LIST(APPEND CampvisGlobalExternalLibs netapi32 version)

    # Append the name of the current build type to the path of the CMake output directory so that
    # NMake places all build artifacts in the same directories as Visual Studio. This makes NMake
    # builds compatible with our macro that copies external DLLs into the build directory.
    IF(CAMPVIS_NMAKE)
        SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}")
        SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}")
        SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}")
    ENDIF(CAMPVIS_NMAKE)
    
ELSEIF(UNIX)
    LIST(APPEND CampvisGlobalDefinitions "-DUNIX")
    LIST(APPEND CampvisGlobalDefinitions "-Wall -Wno-unused-local-typedefs -Wno-unused-variable")
    LIST(APPEND CampvisGlobalDefinitions "-D__STDC_CONSTANT_MACROS")
ENDIF(WIN32)

IF(CMAKE_COMPILER_IS_GNUCXX)
    # enable C++11 support in GCC
    LIST(APPEND CMAKE_CXX_FLAGS "-std=c++11")
    # however: we want to use the TBB implementation of threads
    LIST(APPEND CampvisGlobalDefinitions "-DTBB_IMPLEMENT_CPP0X")
ENDIF()

# tgt configuration
LIST(APPEND CampvisGlobalDefinitions "-DTGT_WITHOUT_DEFINES") # don't use tgt's build system
IF(WIN32)
    SET(TGT_WITH_WMI TRUE)  #< enable Windows Management Instrumentation for hardware detection
ENDIF()
IF(CAMPVIS_DEBUG)
#    SET_PROPERTY(GLOBAL PROPERTY COMPILE_DEFINITIONS_DEBUG TGT_DEBUG CAMPVIS_DEBUG)
    LIST(APPEND CampvisGlobalDefinitions -DTGT_DEBUG -DCAMPVIS_DEBUG)
#    set_property(GLOBAL PROPERTY COMPILE_DEFINITIONS_DEBUG "TGT_DEBUG CAMPVIS_DEBUG")
ENDIF()
 
# minimum Qt version
IF(CAMPVIS_BUILD_APPLICATION)
    FIND_PACKAGE(Qt4 ${CampvisRequiredQtVersion} REQUIRED QtCore QtGui QtOpenGL)
    INCLUDE(${QT_USE_FILE})
    SET(TGT_WITH_QT true)
    SET(CampvisRequiredQtVersion "4.8")
ENDIF(CAMPVIS_BUILD_APPLICATION)


# detect libraries
MESSAGE(STATUS "--------------------------------------------------------------------------------")
MESSAGE(STATUS "Detecting Mandatory External Libraries:")

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CampvisHome}/cmake")

# OpenGL
FIND_PACKAGE(OpenGL REQUIRED)
IF(OPENGL_FOUND)
    MESSAGE(STATUS "* Found OpenGL")
    LIST(APPEND CampvisGlobalIncludeDirs ${OPENGL_INCLUDE_DIR})
    LIST(APPEND CampvisGlobalExternalLibs ${OPENGL_LIBRARIES})
ELSE(OPENGL_FOUND)
    MESSAGE(FATAL_ERROR "OpenGL not found!")
ENDIF(OPENGL_FOUND)
    
# GLEW
FIND_PACKAGE(Glew REQUIRED)
IF(GLEW_FOUND)
    MESSAGE(STATUS "* Found GLEW")
    LIST(APPEND CampvisGlobalDefinitions ${GLEW_DEFINITIONS})
    LIST(APPEND CampvisGlobalIncludeDirs ${GLEW_INCLUDE_DIR})
    LIST(APPEND CampvisGlobalExternalLibs ${GLEW_LIBRARY})
    LIST(APPEND CampvisExternalDllsDebug ${GLEW_DLL_DEBUG})
    LIST(APPEND CampvisExternalDllsRelease ${GLEW_DLL_RELEASE})
    LIST(APPEND CampvisExternalLicenseFiles ${GLEW_LICENSE_FILE})
ELSE(GLEW_FOUND)
    MESSAGE(FATAL_ERROR "GLEW not found!")
ENDIF(GLEW_FOUND)

# TBB
FIND_PACKAGE(TBB REQUIRED)
IF(TBB_FOUND)
    MESSAGE(STATUS "* Found TBB")
    LIST(APPEND CampvisGlobalDefinitions ${TBB_DEFINITIONS})
    LIST(APPEND CampvisGlobalIncludeDirs ${TBB_INCLUDE_DIR})
    LIST(APPEND CampvisGlobalExternalLibs ${TBB_LIBRARY})
    LIST(APPEND CampvisExternalDllsDebug ${TBB_DLL_DEBUG})
    LIST(APPEND CampvisExternalDllsRelease ${TBB_DLL_RELEASE})
    LIST(APPEND CampvisExternalLicenseFiles ${TBB_LICENSE_FILE})
ELSE(TBB_FOUND)
    MESSAGE(FATAL_ERROR "TBB not found!")
ENDIF(TBB_FOUND)

# Eigen
IF(EXISTS "${CampvisHome}/ext/eigen/Eigen/Eigen")
    MESSAGE(STATUS "* Found Eigen")
    LIST(APPEND CampvisGlobalIncludeDirs "${CampvisHome}/ext/eigen")
    LIST(APPEND CampvisGlobalDefinitions -DEIGEN_PERMANENTLY_DISABLE_STUPID_WARNINGS)
ELSE()
    MESSAGE(WARNING "Did not find Eigen - Eigen library must be placed in ext/eigen/!")
ENDIF()
SET(CommonconfProcessed TRUE)

ENDIF(NOT CommonconfProcessed)
