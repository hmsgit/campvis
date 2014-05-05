# CMake file for manualsegmentation module

IF(${ModuleEnabled})
	# Source files:
	FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
		modules/manualsegmentation/pipelines/*.cpp
		modules/manualsegmentation/processors/*.cpp
		modules/manualsegmentation/tools/*.cpp
	)

	# Header files (including GLSL files so that they'll appear in VS projects)
	FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
		modules/manualsegmentation/pipelines/*.h
		modules/manualsegmentation/processors/*.h
		modules/manualsegmentation/tools/*.h
	)
	SET(ThisModDependencies io itk vis)
ENDIF(${ModuleEnabled})

SET(ThisModStatus EXPERIMENTAL)
SET(ThisModExternalDependencies FALSE)