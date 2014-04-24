# CMake file for ITK module

IF(${ModuleEnabled})
    FIND_PACKAGE (ITK REQUIRED)
        
    IF(ITK_FOUND)
        # Since we use ITK classes also in the core module, we need to set global include dirs and global external libs
        LIST(APPEND CampvisGlobalIncludeDirs ${ITK_INCLUDE_DIRS})
        LIST(APPEND CampvisGlobalExternalLibs ${ITK_LIBRARIES})
        
        IF("${ITK_VERSION_MAJOR}" LESS 4)
            MESSAGE(FATAL_ERROR "Found ITK Version ${ITK_VERSION_MAJOR} < 4. CAMPVis needs ITK version >= 4.")
        ELSE()
            MESSAGE(STATUS "  Found ITK library")
        ENDIF()
    ELSE(ITK_FOUND)
        MESSAGE(FATAL_ERROR "Could not locate ITK.")
    ENDIF(ITK_FOUND)

    # Source files:
    FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
        modules/itk/processors/*.cpp
    )

    # Header files
    FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
        modules/itk/processors/*.h
    )
    
    
    # Source files:
    FILE(GLOB CampvisModulesCoreSources RELATIVE ${ModulesDir}
        modules/itk/core/*.cpp
    )

    # Header files
    FILE(GLOB CampvisModulesCoreHeaders RELATIVE ${ModulesDir}
        modules/itk/core/*.h
    )
ENDIF(${ModuleEnabled})

SET(ThisModStatus STABLE)
SET(ThisModExternalDependencies TRUE)
