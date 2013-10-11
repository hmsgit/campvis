# CMake file for campcom module

LIST(APPEND CMAKE_MODULE_PATH "${ModulesDir}/campcom")
FIND_PACKAGE(CAMPCom REQUIRED)

IF(CAMPCOM_FOUND)
	LIST(APPEND ThisModIncludeDirs ${CAMPCOM_INCLUDE_DIRS})
	LIST(APPEND ThisModExternalLibs ${CAMPCOM_LIBRARIES})
ELSE()
	MESSAGE(FATAL_ERROR "Could not find CAMPCom. Please specify CAMPCOM_ROOT.")
ENDIF()

# Source files:
FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
	modules/campcom/pipelines/*.cpp
	modules/campcom/processors/*.cpp
)

# Header files (including GLSL files so that they'll appear in VS projects)
FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
	modules/campcom/pipelines/*.h
	modules/campcom/processors/*.h
)

