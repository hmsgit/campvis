# Try to find TBB library and include path. Once done this will define:
# TBB_FOUND
# TBB_DEFINITIONS
# TBB_INCLUDE_DIR
# TBB_LIBRARY (containing both debug and release libraries on win32)
# TBB_LICENSE_FILE

IF (WIN32)
    IF(NOT TBB_ROOT)
        # check environment variables
        IF(NOT "$ENV{TBB_INSTALL_DIR}" STREQUAL "")
            SET(TbbDirectory $ENV{TBB_INSTALL_DIR})
        ENDIF(NOT "$ENV{TBB_INSTALL_DIR}" STREQUAL "")
        
        # check default install directory
        set(_TbbSeachPaths "C:/Program Files/Intel/TBB" "C:/Program Files (x86)/Intel/TBB" "${CampvisHome}/ext/tbb")
        FIND_PATH( 
            TBB_ROOT 
            NAMES include/tbb/tbb.h
            PATHS ${_TbbSeachPaths}
            DOC "Intel TBB directory with includes, libs and dlls (i.e. where you extracted the binary distribution from threadingbuildingblocks.org.)"
        )
        
        IF(NOT TBB_ROOT)
            MESSAGE(FATAL_ERROR "Could not find Intel TBB. Please set TBB_ROOT to the Intel TBB directory with includes, libs and dlls (i.e. where you extracted the binary distribution from threadingbuildingblocks.org.)")
        ENDIF()
    ENDIF()
    
    SET(TbbDirectory ${TBB_ROOT})
   
    IF(CAMPVIS_MSVC2010)
        SET(TbbCompilerDirectory "vc10")
    ELSEIF(CAMPVIS_MSVC11)
        SET(TbbCompilerDirectory "vc11")
    ELSEIF(CAMPVIS_MSVC12)
        SET(TbbCompilerDirectory "vc12")
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
        SET(TbbPlatformDirectory "ia32")
    ELSEIF(CAMPVIS_WIN64)
        SET(TbbPlatformDirectory "intel64")
    ELSE()
        MESSAGE(FATAL_ERROR "Neither CAMPVIS_WIN32 nor CAMPVIS_WIN64 defined!")
    ENDIF(CAMPVIS_WIN32)

    SET(TBB_LIBRARY_DEBUG      "${TbbDirectory}/lib/${TbbPlatformDirectory}/${TbbCompilerDirectory}/tbb_debug.lib")
    SET(TBB_DLL_DEBUG          "${TbbDirectory}/bin/${TbbPlatformDirectory}/${TbbCompilerDirectory}/tbb_debug.dll")
    SET(TBB_PDB_DEBUG          "${TbbDirectory}/bin/${TbbPlatformDirectory}/${TbbCompilerDirectory}/tbb_debug.pdb")
    SET(TBB_LIBRARY_RELEASE    "${TbbDirectory}/lib/${TbbPlatformDirectory}/${TbbCompilerDirectory}/tbb.lib")
    SET(TBB_DLL_RELEASE        "${TbbDirectory}/bin/${TbbPlatformDirectory}/${TbbCompilerDirectory}/tbb.dll")
    SET(TBB_PDB_RELEASE        "${TbbDirectory}/bin/${TbbPlatformDirectory}/${TbbCompilerDirectory}/tbb.pdb")
    IF(TBB_LIBRARY_DEBUG AND TBB_LIBRARY_RELEASE)
        SET(TBB_LIBRARY debug ${TBB_LIBRARY_DEBUG} optimized ${TBB_LIBRARY_RELEASE})
    ENDIF(TBB_LIBRARY_DEBUG AND TBB_LIBRARY_RELEASE)
    

    SET(TBB_MALLOC_LIBRARY_DEBUG      "${TbbDirectory}/lib/${TbbPlatformDirectory}/${TbbCompilerDirectory}/tbbmalloc_debug.lib")
    SET(TBB_MALLOC_DLL_DEBUG          "${TbbDirectory}/bin/${TbbPlatformDirectory}/${TbbCompilerDirectory}/tbbmalloc_debug.dll")
    SET(TBB_MALLOC_PDB_DEBUG          "${TbbDirectory}/bin/${TbbPlatformDirectory}/${TbbCompilerDirectory}/tbbmalloc_debug.pdb")
    SET(TBB_MALLOC_LIBRARY_RELEASE    "${TbbDirectory}/lib/${TbbPlatformDirectory}/${TbbCompilerDirectory}/tbbmalloc.lib")
    SET(TBB_MALLOC_DLL_RELEASE        "${TbbDirectory}/bin/${TbbPlatformDirectory}/${TbbCompilerDirectory}/tbbmalloc.dll")
    SET(TBB_MALLOC_PDB_RELEASE        "${TbbDirectory}/bin/${TbbPlatformDirectory}/${TbbCompilerDirectory}/tbbmalloc.pdb")
    IF(TBB_MALLOC_LIBRARY_DEBUG AND TBB_MALLOC_LIBRARY_RELEASE)
        SET(TBB_MALLOC_LIBRARY debug ${TBB_MALLOC_LIBRARY_DEBUG} optimized ${TBB_MALLOC_LIBRARY_RELEASE})
    ENDIF(TBB_MALLOC_LIBRARY_DEBUG AND TBB_MALLOC_LIBRARY_RELEASE)
    
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

IF(TBB_INCLUDE_DIR AND TBB_LIBRARY)
    SET(TBB_FOUND TRUE)
ELSE(TBB_INCLUDE_DIR AND TBB_LIBRARY)
    SET(TBB_FOUND FALSE)
ENDIF(TBB_INCLUDE_DIR AND TBB_LIBRARY)

UNSET(TbbDirectory)
MARK_AS_ADVANCED(TbbDirectory TbbDirectory TBB_INCLUDE_DIR TBB_LIBRARY TBB_LIBRARY_DEBUG TBB_LIBRARY_RELEASE TBB_DLL_DEBUG TBB_DLL_RELEASE)
