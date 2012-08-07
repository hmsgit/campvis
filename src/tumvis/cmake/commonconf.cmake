
IF(NOT COMMONCONF_PROCESSED)

SET(TUMVIS_HOME ${CMAKE_CURRENT_SOURCE_DIR})
MESSAGE(STATUS "TUMVis Home: ${TUMVIS_HOME}")

# include macros
INCLUDE(${TUMVIS_HOME}/cmake/macros.cmake)

# detect compiler and architecture
IF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 9 2008")
    SET(TUMVIS_MSVC2008 TRUE)
    SET(TUMVIS_WIN32 TRUE)
    MESSAGE(STATUS "Visual Studio 2008 build (32 Bit)")
ELSEIF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 9 2008 Win64")
    SET(TUMVIS_MSVC2008 TRUE)
    SET(TUMVIS_WIN64 TRUE)
    MESSAGE(STATUS "Visual Studio 2008 Build (64 Bit)")
ELSEIF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 10")
    SET(TUMVIS_MSVC2010 TRUE)
    SET(TUMVIS_WIN32 TRUE)
    MESSAGE(STATUS "Visual Studio 2010 Build (32 Bit)")
ELSEIF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 10 Win64")
    SET(TUMVIS_MSVC2010 TRUE)
    SET(TUMVIS_WIN64 TRUE)
    MESSAGE(STATUS "Visual Studio 2010 Build (64 Bit)")
ELSEIF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 11")
    SET(TUMVIS_MSVC11 TRUE)
    SET(TUMVIS_WIN32 TRUE)
    MESSAGE("Visual Studio 11 Build (32 Bit) (not actively supported)")
ELSEIF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 11 Win64")
    SET(TUMVIS_MSVC11 TRUE)
    SET(TUMVIS_WIN64 TRUE)
    MESSAGE("Visual Studio 11 Build (64 Bit) (not actively supported)")
ELSEIF(${CMAKE_GENERATOR} MATCHES "NMake") 
    SET(TUMVIS_NMAKE TRUE)
    IF(CMAKE_CL_64)
        SET(TUMVIS_WIN64 TRUE)
        MESSAGE(STATUS "NMake 64 Bit Build")
    ELSE(CMAKE_CL_64)
        SET(TUMVIS_WIN32 TRUE)
        MESSAGE(STATUS "NMake 32 Bit Build")            
    ENDIF(CMAKE_CL_64)    
ELSEIF(${CMAKE_GENERATOR} MATCHES "MinGW")
    SET(TUMVIS_MINGW TRUE)
    SET(TUMVIS_WIN32 TRUE)
    MESSAGE("MinGW 32 Bit Build (not actively supported)")
ELSEIF(${CMAKE_GENERATOR} MATCHES "Unix")
    SET(TUMVIS_UNIX TRUE)
    MESSAGE(STATUS "Unix Build")
ELSE()
    MESSAGE(WARNING "Unsupported or unknown generator: ${CMAKE_GENERATOR}")
ENDIF()


# set binary output path
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${TUMVIS_HOME}/bin")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${TUMVIS_HOME}/bin")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${TUMVIS_HOME}/bin")

# common include directories
LIST(APPEND TUMVIS_INCLUDE_DIRECTORIES "${TUMVIS_HOME}/ext")
LIST(APPEND TUMVIS_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}) 

# platform-dependent configuration
IF(WIN32)
    LIST(APPEND TUMVIS_DEFINITIONS "-DNOMINMAX" "-D_CRT_SECURE_NO_DEPRECATE")

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
    #LIST(APPEND TUMVIS_DEFINITIONS /wd4305 /wd4800 /wd4290 /wd4068 /wd4251 /wd4355 /wd4390)
    LIST(APPEND TUMVIS_DEFINITIONS /wd4290 /wd4390)
    
    # enable parallel builds in Visual Studio
    LIST(APPEND TUMVIS_DEFINITIONS /MP)

    # prevent error: number of sections exceeded object file format limit
    LIST(APPEND TUMVIS_DEFINITIONS /bigobj)
    
    # allows 32 Bit builds to use more than 2GB RAM (VC++ only)
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LARGEADDRESSAWARE")
    SET(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE")

    IF(TUMVIS_SHARED_LIBS)
        # Linking against Windows DLLs requires explicit instantiation of templates
        LIST(APPEND TUMVIS_DEFINITIONS "-DDLL_TEMPLATE_INST")

        IF(NOT TUMVIS_GENERATE_MANIFEST)
            # Do not embed manifest into binaries in debug mode (slows down incremental linking)
            SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /MANIFEST:NO")
            SET(CMAKE_EXE_LINKER_FLAGS_DEBUG    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /MANIFEST:NO")
        ENDIF()
    ENDIF()

    # enable/disable incremental linking in debug builds
    If(TUMVIS_INCREMENTAL_LINKING)
        SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /INCREMENTAL")
        SET(CMAKE_EXE_LINKER_FLAGS_DEBUG    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /INCREMENTAL")
    ELSE()
        SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /INCREMENTAL:NO")
        SET(CMAKE_EXE_LINKER_FLAGS_DEBUG    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /INCREMENTAL:NO")
    ENDIF()
    
    LIST(APPEND TUMVIS_EXTERNAL_LIBRARIES netapi32 version)
    
ELSEIF(UNIX)
    LIST(APPEND TUMVIS_DEFINITIONS "-DUNIX")  
    LIST(APPEND TUMVIS_DEFINITIONS "-D__STDC_CONSTANT_MACROS")  
ENDIF(WIN32)

# tgt configuration
LIST(APPEND TUMVIS_DEFINITIONS "-DTGT_WITHOUT_DEFINES") # don't use tgt's build system
IF(WIN32)
    SET(TGT_WITH_WMI TRUE)  #< enable Windows Management Instrumentation for hardware detection
ENDIF()
IF(TUMVIS_DEBUG)
    LIST(APPEND TUMVIS_DEFINITIONS -DTGT_DEBUG -DTUMVIS_DEBUG)
ENDIF()
 
# minimum Qt version
SET(TUMVIS_REQUIRED_QT_VERSION "4.8")


# detect libraries
MESSAGE(STATUS "--------------------------------------------------------------------------------")
MESSAGE(STATUS "Detecting Common Mandatory Libraries:")

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${TUMVIS_HOME}/cmake")

# OpenGL
FIND_PACKAGE(OpenGL REQUIRED)
IF(OPENGL_FOUND)
    MESSAGE(STATUS "* Found OpenGL")
    LIST(APPEND TUMVIS_INCLUDE_DIRECTORIES ${OPENGL_INCLUDE_DIR})
    LIST(APPEND TUMVIS_EXTERNAL_LIBRARIES ${OPENGL_LIBRARIES})
ELSE(OPENGL_FOUND)
    MESSAGE(FATAL_ERROR "OpenGL not found!")
ENDIF(OPENGL_FOUND)
    
# GLEW
FIND_PACKAGE(Glew REQUIRED)
IF(GLEW_FOUND)
    MESSAGE(STATUS "* Found GLEW")
    LIST(APPEND TUMVIS_DEFINITIONS ${GLEW_DEFINITIONS})
    LIST(APPEND TUMVIS_INCLUDE_DIRECTORIES ${GLEW_INCLUDE_DIR})
    LIST(APPEND TUMVIS_EXTERNAL_LIBRARIES ${GLEW_LIBRARY})
    LIST(APPEND TUMVIS_EXTERNAL_DEBUG_DLLS ${GLEW_DLL_DEBUG})
    LIST(APPEND TUMVIS_EXTERNAL_RELEASE_DLLS ${GLEW_DLL_RELEASE})
    LIST(APPEND TUMVIS_EXTERNAL_LICENSE_FILES ${GLEW_LICENSE_FILE})
ELSE(GLEW_FOUND)
    MESSAGE(FATAL_ERROR "GLEW not found!")
ENDIF(GLEW_FOUND)

# TBB
FIND_PACKAGE(TBB REQUIRED)
IF(TBB_FOUND)
    MESSAGE(STATUS "* Found TBB")
    LIST(APPEND TUMVIS_DEFINITIONS ${TBB_DEFINITIONS})
    LIST(APPEND TUMVIS_INCLUDE_DIRECTORIES ${TBB_INCLUDE_DIR})
    LIST(APPEND TUMVIS_EXTERNAL_LIBRARIES ${TBB_LIBRARY})
    LIST(APPEND TUMVIS_EXTERNAL_DEBUG_DLLS ${TBB_DLL_DEBUG})
    LIST(APPEND TUMVIS_EXTERNAL_RELEASE_DLLS ${TBB_DLL_RELEASE})
    LIST(APPEND TUMVIS_EXTERNAL_LICENSE_FILES ${TBB_LICENSE_FILE})
ELSE(TBB_FOUND)
    MESSAGE(FATAL_ERROR "TBB not found!")
ENDIF(TBB_FOUND)


SET(COMMONCONF_PROCESSED TRUE)
ENDIF(NOT COMMONCONF_PROCESSED)
