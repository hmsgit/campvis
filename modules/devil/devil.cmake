# CMake file for DevIL module

IF(${ModuleEnabled})
    # Tell TGT that we have the DevIL module
    LIST(APPEND ThisModDefinitions -DCGT_HAS_DEVIL)

    IF(UNIX)
        # Unix-like systems have their DevIL lib via their package manager
        FIND_PACKAGE(DevIL REQUIRED)
            LIST(APPEND CampvisGlobalIncludeDirs ${IL_INCLUDE_DIR})
            LIST(APPEND CampvisGlobalExternalLibs ${IL_LIBRARIES} ${ILU_LIBRARIES})
        IF(IL_FOUND)
            MESSAGE(STATUS "** Found DevIL library")
        ELSE()
            MESSAGE(WARNING "Could not find DevIL library. Check your package manager to make sure DevIL is installed on your system.")
        ENDIF()
    ELSEIF(WIN32)
        # DevIL libs for Windows systems are shipped with this module, make sure they're copied to the dist folder.
        LIST(APPEND CampvisGlobalIncludeDirs "${ThisModDir}/ext/devil/include")

        IF(CAMPVIS_WIN32)
            LIST(APPEND CampvisGlobalExternalLibs 
                "${ThisModDir}/ext/devil/lib/win32/DevIL.lib"
                "${ThisModDir}/ext/devil/lib/win32/ILU.lib"
            )
            
            LIST(APPEND CampvisExternalDllsDebug
                "${ThisModDir}/ext/devil/lib/win32/DevIL.dll"
                "${ThisModDir}/ext/devil/lib/win32/ILU.dll"
            )
            LIST(APPEND CampvisExternalDllsRelease
                "${ThisModDir}/ext/devil/lib/win32/DevIL.dll"
                "${ThisModDir}/ext/devil/lib/win32/ILU.dll"
            )
        ELSEIF(CAMPVIS_WIN64)
            LIST(APPEND CampvisGlobalExternalLibs
                "${ThisModDir}/ext/devil/lib/win64/DevIL.lib"
                "${ThisModDir}/ext/devil/lib/win64/ILU.lib"
            )
            
            LIST(APPEND CampvisExternalDllsDebug
                "${ThisModDir}/ext/devil/lib/win64/DevIL.dll"
                "${ThisModDir}/ext/devil/lib/win64/ILU.dll"
            )
            LIST(APPEND CampvisExternalDllsRelease
                "${ThisModDir}/ext/devil/lib/win64/DevIL.dll"
                "${ThisModDir}/ext/devil/lib/win64/ILU.dll"
            )
        ENDIF()
        
    #    LIST(APPEND MOD_DEBUG_DLLS "${ThisModDir}/ext/jpeg/jpeg62.dll")
    #    LIST(APPEND MOD_RELEASE_DLLS "${ThisModDir}/ext/jpeg/jpeg62.dll")
    ENDIF()

    # Source files:
    FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
        modules/devil/processors/*.cpp
    )

    # Header files
    FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
        modules/devil/processors/*.h
    )
ENDIF(${ModuleEnabled})

SET(ThisModStatus STABLE)
IF(WIN32)
    SET(ThisModExternalDependencies FALSE)
ELSE()
    SET(ThisModExternalDependencies TRUE)
ENDIF(WIN32)
