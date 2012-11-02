
IF(NOT COMMONCONF_PROCESSED)

SET(CAMPVIS_HOME ${CMAKE_CURRENT_SOURCE_DIR})
MESSAGE(STATUS "TUMVis Home: ${CAMPVIS_HOME}")

# include macros
INCLUDE(${CAMPVIS_HOME}/cmake/macros.cmake)

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
    MESSAGE("Visual Studio 11 Build (32 Bit) (not actively supported)")
ELSEIF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 11 Win64")
    SET(CAMPVIS_MSVC11 TRUE)
    SET(CAMPVIS_WIN64 TRUE)
    MESSAGE("Visual Studio 11 Build (64 Bit) (not actively supported)")
ELSEIF(${CMAKE_GENERATOR} MATCHES "NMake") 
    SET(CAMPVIS_NMAKE TRUE)
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
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CAMPVIS_HOME}/bin")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CAMPVIS_HOME}/bin")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CAMPVIS_HOME}/bin")

# common include directories
LIST(APPEND CAMPVIS_INCLUDE_DIRECTORIES "${CAMPVIS_HOME}/ext")
LIST(APPEND CAMPVIS_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}) 

# platform-dependent configuration
IF(WIN32)
    LIST(APPEND CAMPVIS_DEFINITIONS "-DNOMINMAX" "-D_CRT_SECURE_NO_DEPRECATE")

    # Disable warnings for Microsoft compiler:
    # C4305: 'identifier' : truncation from 'type1' to 'type2'
    # C4800: 'type' : forcing value to bool 'true' or 'false' (performance warning
    # C4290: C++ exception specification ignored except to indicate a function is
    #        not __declspec(nothrow)
    # C4068: unknown pragma
    # C4251  class needs to have dll interface (used for std classes)
    # C4355: 'this' : used in base member initializer list 
    #        occurs in processors' constructors when initializing event properties, 
    #        but is safe there, since the 'this' pointer is only stored and not accessed.
    # C4390: ';' : empty controlled statement found; is this the intent?
    #        occurs when OpenGL error logging macros are disabled
    #LIST(APPEND CAMPVIS_DEFINITIONS /wd4305 /wd4800 /wd4290 /wd4068 /wd4251 /wd4355 /wd4390)
    LIST(APPEND CAMPVIS_DEFINITIONS /wd4290 /wd4390)
    
    # enable parallel builds in Visual Studio
    LIST(APPEND CAMPVIS_DEFINITIONS /MP)

    # prevent error: number of sections exceeded object file format limit
    LIST(APPEND CAMPVIS_DEFINITIONS /bigobj)
    
    # allows 32 Bit builds to use more than 2GB RAM (VC++ only)
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LARGEADDRESSAWARE")
    SET(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE")

    IF(CAMPVIS_SHARED_LIBS)
        # Linking against Windows DLLs requires explicit instantiation of templates
        LIST(APPEND CAMPVIS_DEFINITIONS "-DDLL_TEMPLATE_INST")

        IF(NOT CAMPVIS_GENERATE_MANIFEST)
            # Do not embed manifest into binaries in debug mode (slows down incremental linking)
            SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /MANIFEST:NO")
            SET(CMAKE_EXE_LINKER_FLAGS_DEBUG    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /MANIFEST:NO")
        ENDIF()
    ENDIF()

    # enable/disable incremental linking in debug builds
    If(CAMPVIS_INCREMENTAL_LINKING)
        SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /INCREMENTAL")
        SET(CMAKE_EXE_LINKER_FLAGS_DEBUG    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /INCREMENTAL")
    ELSE()
        SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /INCREMENTAL:NO")
        SET(CMAKE_EXE_LINKER_FLAGS_DEBUG    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /INCREMENTAL:NO")
    ENDIF()
    
    LIST(APPEND CAMPVIS_EXTERNAL_LIBRARIES netapi32 version)
    
ELSEIF(UNIX)
    LIST(APPEND CAMPVIS_DEFINITIONS "-DUNIX")  
    LIST(APPEND CAMPVIS_DEFINITIONS "-D__STDC_CONSTANT_MACROS")  
ENDIF(WIN32)

# tgt configuration
LIST(APPEND CAMPVIS_DEFINITIONS "-DTGT_WITHOUT_DEFINES") # don't use tgt's build system
IF(WIN32)
    SET(TGT_WITH_WMI TRUE)  #< enable Windows Management Instrumentation for hardware detection
ENDIF()
IF(CAMPVIS_DEBUG)
    LIST(APPEND CAMPVIS_DEFINITIONS -DTGT_DEBUG -DCAMPVIS_DEBUG)
ENDIF()
 
# minimum Qt version
SET(CAMPVIS_REQUIRED_QT_VERSION "4.8")


# detect libraries
MESSAGE(STATUS "--------------------------------------------------------------------------------")
MESSAGE(STATUS "Detecting Common Mandatory Libraries:")

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CAMPVIS_HOME}/cmake")

# OpenGL
FIND_PACKAGE(OpenGL REQUIRED)
IF(OPENGL_FOUND)
    MESSAGE(STATUS "* Found OpenGL")
    LIST(APPEND CAMPVIS_INCLUDE_DIRECTORIES ${OPENGL_INCLUDE_DIR})
    LIST(APPEND CAMPVIS_EXTERNAL_LIBRARIES ${OPENGL_LIBRARIES})
ELSE(OPENGL_FOUND)
    MESSAGE(FATAL_ERROR "OpenGL not found!")
ENDIF(OPENGL_FOUND)
    
# GLEW
FIND_PACKAGE(Glew REQUIRED)
IF(GLEW_FOUND)
    MESSAGE(STATUS "* Found GLEW")
    LIST(APPEND CAMPVIS_DEFINITIONS ${GLEW_DEFINITIONS})
    LIST(APPEND CAMPVIS_INCLUDE_DIRECTORIES ${GLEW_INCLUDE_DIR})
    LIST(APPEND CAMPVIS_EXTERNAL_LIBRARIES ${GLEW_LIBRARY})
    LIST(APPEND CAMPVIS_EXTERNAL_DEBUG_DLLS ${GLEW_DLL_DEBUG})
    LIST(APPEND CAMPVIS_EXTERNAL_RELEASE_DLLS ${GLEW_DLL_RELEASE})
    LIST(APPEND CAMPVIS_EXTERNAL_LICENSE_FILES ${GLEW_LICENSE_FILE})
ELSE(GLEW_FOUND)
    MESSAGE(FATAL_ERROR "GLEW not found!")
ENDIF(GLEW_FOUND)

# TBB
FIND_PACKAGE(TBB REQUIRED)
IF(TBB_FOUND)
    MESSAGE(STATUS "* Found TBB")
    LIST(APPEND CAMPVIS_DEFINITIONS ${TBB_DEFINITIONS})
    LIST(APPEND CAMPVIS_INCLUDE_DIRECTORIES ${TBB_INCLUDE_DIR})
    LIST(APPEND CAMPVIS_EXTERNAL_LIBRARIES ${TBB_LIBRARY})
    LIST(APPEND CAMPVIS_EXTERNAL_DEBUG_DLLS ${TBB_DLL_DEBUG})
    LIST(APPEND CAMPVIS_EXTERNAL_RELEASE_DLLS ${TBB_DLL_RELEASE})
    LIST(APPEND CAMPVIS_EXTERNAL_LICENSE_FILES ${TBB_LICENSE_FILE})
ELSE(TBB_FOUND)
    MESSAGE(FATAL_ERROR "TBB not found!")
ENDIF(TBB_FOUND)

# OpenCL
FIND_PACKAGE(CL REQUIRED)
    MESSAGE(STATUS "* Found OpenCL")
    LIST(APPEND CAMPVIS_INCLUDE_DIRECTORIES ${OPENCL_INCLUDE_DIR})
    LIST(APPEND CAMPVIS_EXTERNAL_LIBRARIES ${OPENCL_LIBRARY})
IF (OPENCL_FOUND)
ELSE(OPENCL_FOUND)
    MESSAGE(FATAL_ERROR "OpenCL not found!")
ENDIF(OPENCL_FOUND)


# detect modules
MESSAGE(STATUS "--------------------------------------------------------------------------------")
MESSAGE(STATUS "Detecting installed modules:")

# collect list of directories in modules directories
SET(ModulesDir ${CAMPVIS_HOME}/modules)
LIST_SUBDIRECTORIES(ModDirs ${ModulesDir} false)

# remove CMake realted directories
LIST(REMOVE_ITEM ModDirs CMakeFiles)
LIST(REMOVE_ITEM ModDirs campvis-modules.dir)

# go through each subdirectory
FOREACH(ModDir ${ModDirs})
    # check whether  module.cmake file exists
    SET(ModFile ${ModulesDir}/${ModDir}/${ModDir}.cmake)
    IF(EXISTS ${ModFile})
        MESSAGE(STATUS "* Found Module ${ModDir}")
        STRING(TOLOWER ${ModDir} ModDirLower)
        STRING(TOUPPER ${ModDir} ModDirUpper)

        # check whether the option to build this very module exists and is checked
        IF(CAMPVIS_BUILD_MODULE_${ModDirUpper})
            SET(ThisModDir ${ModulesDir}/${ModDir})
            
            # load .cmake file
            INCLUDE(${ModFile})
            
            # merge module settings into global settings
            LIST(APPEND CAMPVisModulesDefinitions ${ThisModDefinitions})
            LIST(APPEND CAMPVisModulesIncludeDirs ${ThisModIncludeDirs})
            LIST(APPEND CAMPVisModulesExternalLibs ${ThisModExternalLibs})
            LIST(APPEND CAMPVisModulesSources ${ThisModSources})
            LIST(APPEND CAMPVisModulesHeaders ${ThisModHeaders})
            
            # unset module settings to avoid duplicates if module cmake file misses sth.
            UNSET(ThisModDefinitions)
            UNSET(ThisModIncludeDirs)
            UNSET(ThisModExternalLibs)
            UNSET(ThisModSources)
            UNSET(ThisModHeaders)
            
        ELSEIF(NOT DEFINED CAMPVIS_BUILD_MODULE_${ModDirUpper})
            # add a CMake option for building this module
            OPTION(CAMPVIS_BUILD_MODULE_${ModDirUpper}  "Build Module ${ModDir}" OFF)
        ENDIF(CAMPVIS_BUILD_MODULE_${ModDirUpper})
        
    ELSE(EXISTS ${ModFile})
        MESSAGE(STATUS "* WARNING: Found Directory ${ModDir} Without CMake file - ignored")
    ENDIF(EXISTS ${ModFile})
    
ENDFOREACH(ModDir ${ModDirs})

SET(COMMONCONF_PROCESSED TRUE)
ENDIF(NOT COMMONCONF_PROCESSED)
