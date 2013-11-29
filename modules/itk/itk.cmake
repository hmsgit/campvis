# CMake file for ITK module

FIND_PACKAGE (ITK REQUIRED 
    ITKCommon 
    ITKIOImageBase 
    ITKSmoothing 
    ITKImageFeature 
    ITKAnisotropicSmoothing 
    ITKMathematicalMorphology
    ITKBinaryMathematicalMorphology
    )
    
IF(ITK_FOUND)
    #MESSAGE(STATUS ${ITK_INCLUDE_DIRS})
    #MESSAGE(STATUS ${ITK_LIBRARIES})
    #INCLUDE(${USE_ITK_FILE})
    
    LIST(APPEND CampvisGlobalIncludeDirs ${ITK_INCLUDE_DIRS})
    LIST(APPEND CampvisGlobalExternalLibs ${ITK_LIBRARIES})
    #SET(ThisModIncludeDirs ${ITK_INCLUDE_DIRS})
    #SET(ThisModExternalLibs ${ITK_LIBRARIES})
    
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
	modules/itk/tools/*.cpp
)

# Header files
FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
	modules/itk/processors/*.h
	modules/itk/tools/*.h
	modules/itk/core/*.h
)
