# CMake file for DevIL module

# Tell TGT that we have the DevIL module
LIST(APPEND ThisModDefinitions -DTGT_HAS_DEVIL)

IF(UNIX)
	# Unix-like systems have their DevIL lib via their package manager
	FIND_PACKAGE(DevIL REQUIRED)
	IF(IL_FOUND)
        SET(ThisModIncludeDirs ${IL_INCLUDE_DIR})
        SET(ThisModExternalLibs ${IL_LIBRARIES} ${ILU_LIBRARIES})
        MESSAGE(STATUS "** Found DevIL library")
	ELSE()
		MESSAGE(FATAL_ERROR "Could not find DevIL library. Check your package manager to make sure DevIL is installed on your system.")
	ENDIF()
ELSEIF(WIN32)
	# DevIL libs for Windows systems are shipped with this module, make sure they're copied to the dist folder.
    SET(ThisModIncludeDirs "${ThisModDir}/ext/devil/include")

    IF(CAMPVIS_WIN32)
        SET(ThisModExternalLibs 
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
        SET(ThisModExternalLibs 
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