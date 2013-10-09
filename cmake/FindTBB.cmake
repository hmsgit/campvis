# Try to find TBB library and include path. Once done this will define:
# TBB_FOUND
# TBB_DEFINITIONS
# TBB_INCLUDE_DIR
# TBB_LIBRARY (containing both debug and release libraries on win32)
# TBB_LICENSE_FILE

IF (WIN32)
    MESSAGE(STATUS ${TbbDirectory})
    
    IF(NOT TbbDirectory)
        # check environment variables
        IF(NOT "$ENV{TBB_INSTALL_DIR}" STREQUAL "")
            SET(TbbDirectory $ENV{TBB_INSTALL_DIR})
        ENDIF(NOT "$ENV{TBB_INSTALL_DIR}" STREQUAL "")
        
        # check default install directory
        set(_TbbSeachPaths "C:/Program Files/Intel/TBB" "C:/Program Files (x86)/Intel/TBB" "${CampvisHome}/ext/tbb")
        FIND_PATH( 
            TbbDirectory 
            NAMES include/tbb/tbb.h
            PATHS ${_TbbSeachPaths}
            DOC "Intel TBB directory with includes, libs and dlls (i.e. where you extracted the binary distribution from threadingbuildingblocks.org.)"
        )
    ENDIF()
   
    IF(CAMPVIS_MSVC2010)
        SET(TbbCompilerDirectory "vc10")
    ELSEIF(CAMPVIS_MSVC11)
        SET(TbbCompilerDirectory "vc11")
    ELSE()
        MESSAGE(WARNING "Compiler not supported by makefile, you might be able to fix that...")
    ENDIF(CAMPVIS_MSVC2010)
    
    FIND_PATH( 
        TBB_INCLUDE_DIR 
        NAMES tbb/tbb.h
        PATHS ${TbbDirectory}/include
        DOC "Intel TBB include directory (i.e. where tbb/tbb.h is)"
    )
    
    # set debug and release library
    IF(CAMPVIS_WIN32)
        SET(TBB_LIBRARY_DEBUG      "${TbbDirectory}/lib/ia32/${TbbCompilerDirectory}/tbb_debug.lib")
        SET(TBB_DLL_DEBUG          "${TbbDirectory}/bin/ia32/${TbbCompilerDirectory}/tbb_debug.dll")
        SET(TBB_PDB_DEBUG          "${TbbDirectory}/bin/ia32/${TbbCompilerDirectory}/tbb_debug.pdb")
        SET(TBB_LIBRARY_RELEASE    "${TbbDirectory}/lib/ia32/${TbbCompilerDirectory}/tbb.lib")
        SET(TBB_DLL_RELEASE        "${TbbDirectory}/bin/ia32/${TbbCompilerDirectory}/tbb.dll")
        SET(TBB_PDB_RELEASE        "${TbbDirectory}/bin/ia32/${TbbCompilerDirectory}/tbb.pdb")
    ELSEIF(CAMPVIS_WIN64)
        SET(TBB_LIBRARY_DEBUG      "${TbbDirectory}/lib/intel64/${TbbCompilerDirectory}/tbb_debug.lib")
        SET(TBB_DLL_DEBUG          "${TbbDirectory}/bin/intel64/${TbbCompilerDirectory}/tbb_debug.dll")
        SET(TBB_PDB_DEBUG          "${TbbDirectory}/bin/intel64/${TbbCompilerDirectory}/tbb_debug.pdb")
        SET(TBB_LIBRARY_RELEASE    "${TbbDirectory}/lib/intel64/${TbbCompilerDirectory}/tbb.lib")
        SET(TBB_DLL_RELEASE        "${TbbDirectory}/bin/intel64/${TbbCompilerDirectory}/tbb.dll")
        SET(TBB_PDB_RELEASE        "${TbbDirectory}/bin/intel64/${TbbCompilerDirectory}/tbb.pdb")
    ELSE()
        MESSAGE(FATAL_ERROR "Neither CAMPVIS_WIN32 nor CAMPVIS_WIN64 defined!")
    ENDIF(CAMPVIS_WIN32)

    IF (TBB_LIBRARY_DEBUG AND TBB_LIBRARY_RELEASE)
        SET(TBB_LIBRARY debug ${TBB_LIBRARY_DEBUG} optimized ${TBB_LIBRARY_RELEASE})
    ENDIF(TBB_LIBRARY_DEBUG AND TBB_LIBRARY_RELEASE)
    
    SET(TBB_LICENSE_FILE "${TbbDirectory}/COPYING")
    
ELSE (WIN32)
    FIND_PATH( 
        TBB_INCLUDE_DIR 
        NAMES tbb/mutex.h
        PATHS ${TbbDirectory}/include /usr/include /usr/local/include /sw/include /opt/local/include
        DOC "The directory where tbb/*.h resides"
    )
    FIND_LIBRARY(
        TBB_LIBRARY
        NAMES TBB libtbb.so
        PATHS ${TbbDirectory}/lib /usr/lib64 /usr/lib /usr/local/lib64 /usr/local/lib /sw/lib /opt/local/lib
        DOC "The TBB library"
    )
ENDIF (WIN32)

    MESSAGE(STATUS ${TBB_INCLUDE_DIR})
    MESSAGE(STATUS ${TBB_LIBRARY})

IF(TBB_INCLUDE_DIR AND TBB_LIBRARY)
    SET(TBB_FOUND TRUE)
ELSE(TBB_INCLUDE_DIR AND TBB_LIBRARY)
    SET(TBB_FOUND FALSE)
ENDIF(TBB_INCLUDE_DIR AND TBB_LIBRARY)

UNSET(TbbDirectory)
MARK_AS_ADVANCED(TbbDirectory TbbDirectory TBB_INCLUDE_DIR TBB_LIBRARY TBB_LIBRARY_DEBUG TBB_LIBRARY_RELEASE TBB_DLL_DEBUG TBB_DLL_RELEASE)
