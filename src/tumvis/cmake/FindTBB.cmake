# Try to find TBB library and include path. Once done this will define:
# TBB_FOUND
# TBB_DEFINITIONS
# TBB_INCLUDE_DIR
# TBB_LIBRARY (containing both debug and release libraries on win32)
# TBB_LICENSE_FILE

IF (WIN32)
    SET(TBB_DIR "${CAMPVIS_HOME}/ext/tbb" CACHE PATH "If TBB is not found, set this path")
   
    SET(TBB_INCLUDE_DIR "${TBB_DIR}/include")
    
    IF(CAMPVIS_MSVC2008)
        SET(TBB_CDIR "vc9")
    ELSEIF(CAMPVIS_MSVC2010)
        SET(TBB_CDIR "vc10")
    ELSE()
        MESSAGE(WARNING "Compiler not supported by makefile, you might be able to fix that...")
    ENDIF(CAMPVIS_MSVC2008)

    # set debug and release library
    IF(CAMPVIS_WIN32)
        SET(TBB_LIBRARY_DEBUG      "${TBB_DIR}/lib/ia32/${TBB_CDIR}/tbb_debug.lib")
        SET(TBB_DLL_DEBUG          "${TBB_DIR}/bin/ia32/${TBB_CDIR}/tbb_debug.dll")
        SET(TBB_PDB_DEBUG          "${TBB_DIR}/bin/ia32/${TBB_CDIR}/tbb_debug.pdb")
        SET(TBB_LIBRARY_RELEASE    "${TBB_DIR}/lib/ia32/${TBB_CDIR}/tbb.lib")
        SET(TBB_DLL_RELEASE        "${TBB_DIR}/bin/ia32/${TBB_CDIR}/tbb.dll")
        SET(TBB_PDB_RELEASE        "${TBB_DIR}/bin/ia32/${TBB_CDIR}/tbb.pdb")
    ELSEIF(CAMPVIS_WIN64)
        SET(TBB_LIBRARY_DEBUG      "${TBB_DIR}/lib/intel64/${TBB_CDIR}/tbb_debug.lib")
        SET(TBB_DLL_DEBUG          "${TBB_DIR}/bin/intel64/${TBB_CDIR}/tbb_debug.dll")
        SET(TBB_PDB_DEBUG          "${TBB_DIR}/bin/intel64/${TBB_CDIR}/tbb_debug.pdb")
        SET(TBB_LIBRARY_RELEASE    "${TBB_DIR}/lib/intel64/${TBB_CDIR}/tbb.lib")
        SET(TBB_DLL_RELEASE        "${TBB_DIR}/bin/intel64/${TBB_CDIR}/tbb.dll")
        SET(TBB_PDB_RELEASE        "${TBB_DIR}/bin/intel64/${TBB_CDIR}/tbb.pdb")
    ELSE()
        MESSAGE(FATAL_ERROR "Neither CAMPVIS_WIN32 nor CAMPVIS_WIN64 defined!")
    ENDIF(CAMPVIS_WIN32)

    IF (TBB_LIBRARY_DEBUG AND TBB_LIBRARY_RELEASE)
        SET(TBB_LIBRARY debug ${TBB_LIBRARY_DEBUG} optimized ${TBB_LIBRARY_RELEASE})
    ENDIF(TBB_LIBRARY_DEBUG AND TBB_LIBRARY_RELEASE)
    
    SET(TBB_LICENSE_FILE "${TBB_DIR}/COPYING")
    
ELSE (WIN32)
    FIND_PATH( 
        TBB_INCLUDE_DIR 
        NAMES tbb/mutex.h
        PATHS ${TBB_DIR}/include /usr/include /usr/local/include /sw/include /opt/local/include
        DOC "The directory where tbb/*.h resides"
    )
    FIND_LIBRARY(
        TBB_LIBRARY
        NAMES TBB libtbb
        PATHS ${TBB_DIR}/lib /usr/lib64 /usr/lib /usr/local/lib64 /usr/local/lib /sw/lib /opt/local/lib
        DOC "The TBB library"
    )
ENDIF (WIN32)

IF(TBB_INCLUDE_DIR AND TBB_LIBRARY)
    SET(TBB_FOUND TRUE)
ELSE(TBB_INCLUDE_DIR AND TBB_LIBRARY)
    SET(TBB_FOUND FALSE)
ENDIF(TBB_INCLUDE_DIR AND TBB_LIBRARY)

UNSET(TBB_DIR)
MARK_AS_ADVANCED(TBB_DIR TBB_DIR TBB_INCLUDE_DIR TBB_LIBRARY TBB_LIBRARY_DEBUG TBB_LIBRARY_RELEASE TBB_DLL_DEBUG TBB_DLL_RELEASE)
