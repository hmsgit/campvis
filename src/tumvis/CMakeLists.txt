#
# TUMVis - Yet another visualization framework
# CMake file inspired by Voreen
#

PROJECT(TUMVis)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)

#
# Global Build Options
#

OPTION(TUMVIS_SHARED_LIBS          "Build shared libraries?"                                       ON )
#OPTION(TUMVIS_PRECOMPILED_HEADER   "Use pre-compiled headers?"                                     ON )
OPTION(TUMVIS_DEBUG                "Activate debug code?"                                          ON )
OPTION(TUMVIS_BUILD_APPLICATION    "Build TUMVis Application"                                      ON )
OPTION(TUMVIS_BUILD_CORE           "Build TUMVis Core"                                             ON )
OPTION(TUMVIS_BUILD_MODULES        "Build TUMVis Modules"                                          ON )
OPTION(TUMVIS_BUILD_LIB_TGT        "Build TGT Library"                                             ON )
OPTION(TUMVIS_BUILD_LIB_CLLIB      "Build CLLib Library"                                           ON )

IF(WIN32)
    OPTION(TUMVIS_COPY_EXTERNAL_DLLS   "Copy external DLLs to bin directory?"                      ON )
    OPTION(TUMVIS_INCREMENTAL_LINKING  "Enable incremental linking in Visual Studio debug builds?" ON )
    OPTION(TUMVIS_GENERATE_MANIFEST    "Generate manifest in Visual Studio debug builds (switch on when encountering errors using incremental linking)?"          OFF)
ENDIF()

#
# Further Build Configurations
#

include(cmake/commonconf.cmake)

IF(TUMVIS_BUILD_LIB_TGT)
    ADD_SUBDIRECTORY(ext/tgt)
ENDIF()

IF(TUMVIS_BUILD_LIB_CLLIB)
    ADD_SUBDIRECTORY(ext/cllib)
ENDIF()

IF(TUMVIS_BUILD_CORE)
    ADD_SUBDIRECTORY(core)
ENDIF()

IF(TUMVIS_BUILD_MODULES)
    ADD_SUBDIRECTORY(modules)
ENDIF()

IF(TUMVIS_BUILD_APPLICATION)
    ADD_SUBDIRECTORY(application)
ENDIF()

#
# Copy External DLLs
#
IF(WIN32)
    LIST(REMOVE_DUPLICATES TUMVIS_EXTERNAL_DEBUG_DLLS)
    LIST(REMOVE_DUPLICATES TUMVIS_EXTERNAL_RELEASE_DLLS)

    IF(TUMVIS_COPY_EXTERNAL_DLLS)
        COPY_EXTERNAL_DLLS(TUMVIS_EXTERNAL_DEBUG_DLLS TUMVIS_EXTERNAL_RELEASE_DLLS false)
    ENDIF()
    
    IF(TUMVIS_ADD_INSTALL_TARGET)
        INSTALL(FILES ${TUMVIS_EXTERNAL_DEBUG_DLLS} 
            DESTINATION .
            CONFIGURATIONS Debug
        )
        INSTALL(FILES ${TUMVIS_EXTERNAL_RELEASE_DLLS}
            DESTINATION .
            CONFIGURATIONS Release
        )
    ENDIF()
ENDIF()
