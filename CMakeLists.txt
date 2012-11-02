#
# CAMPVis - Yet another visualization framework
# CMake file inspired by Voreen
#

PROJECT(CAMPVis)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)

#
# Global Build Options
#

OPTION(CAMPVIS_SHARED_LIBS          "Build shared libraries?"                                       ON )
#OPTION(CAMPVIS_PRECOMPILED_HEADER   "Use pre-compiled headers?"                                     ON )
OPTION(CAMPVIS_DEBUG                "Activate debug code?"                                          ON )
OPTION(CAMPVIS_BUILD_APPLICATION    "Build CAMPVis Application"                                      ON )
OPTION(CAMPVIS_BUILD_CORE           "Build CAMPVis Core"                                             ON )
OPTION(CAMPVIS_BUILD_MODULES        "Build CAMPVis Modules"                                          ON )
OPTION(CAMPVIS_BUILD_LIB_TGT        "Build TGT Library"                                             ON )
OPTION(CAMPVIS_BUILD_LIB_KISSCL     "Build KissCL Library"                                          ON )

IF(WIN32)
    OPTION(CAMPVIS_COPY_EXTERNAL_DLLS   "Copy external DLLs to bin directory?"                      ON )
    OPTION(CAMPVIS_INCREMENTAL_LINKING  "Enable incremental linking in Visual Studio debug builds?" ON )
    OPTION(CAMPVIS_GENERATE_MANIFEST    "Generate manifest in Visual Studio debug builds (switch on when encountering errors using incremental linking)?"          OFF)
ENDIF()

#
# Further Build Configurations
#

include(cmake/commonconf.cmake)

MESSAGE(STATUS "--------------------------------------------------------------------------------")

IF(CAMPVIS_BUILD_LIB_TGT)
    ADD_SUBDIRECTORY(ext/tgt)
ENDIF()

IF(CAMPVIS_BUILD_LIB_KISSCL)
    LIST(APPEND CAMPVIS_DEFINITIONS "-DHAS_KISSCL")
    ADD_SUBDIRECTORY(ext/kisscl)
ENDIF()

IF(CAMPVIS_BUILD_CORE)
    ADD_SUBDIRECTORY(core)
ENDIF()

IF(CAMPVIS_BUILD_MODULES)
    ADD_SUBDIRECTORY(modules)
ENDIF()

IF(CAMPVIS_BUILD_APPLICATION)
    ADD_SUBDIRECTORY(application)
ENDIF()

#
# Copy External DLLs
#
IF(WIN32)
    LIST(REMOVE_DUPLICATES CAMPVIS_EXTERNAL_DEBUG_DLLS)
    LIST(REMOVE_DUPLICATES CAMPVIS_EXTERNAL_RELEASE_DLLS)

    IF(CAMPVIS_COPY_EXTERNAL_DLLS)
        COPY_EXTERNAL_DLLS(CAMPVIS_EXTERNAL_DEBUG_DLLS CAMPVIS_EXTERNAL_RELEASE_DLLS false)
    ENDIF()
    
    IF(CAMPVIS_ADD_INSTALL_TARGET)
        INSTALL(FILES ${CAMPVIS_EXTERNAL_DEBUG_DLLS} 
            DESTINATION .
            CONFIGURATIONS Debug
        )
        INSTALL(FILES ${CAMPVIS_EXTERNAL_RELEASE_DLLS}
            DESTINATION .
            CONFIGURATIONS Release
        )
    ENDIF()
ENDIF()