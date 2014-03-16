# CMake file for Preprocessing module

IF(ModuleEnabled)
	# Source files:
	FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
		modules/preprocessing/pipelines/*.cpp
		modules/preprocessing/processors/*.cpp
		modules/preprocessing/tools/*.cpp
	)

	# Header files
	FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
		modules/preprocessing/glsl/*.frag
		modules/preprocessing/pipelines/*.h
		modules/preprocessing/processors/*.h
		modules/preprocessing/tools/*.h
	)

	SET(ThisModShaderDirectories "modules/preprocessing/glsl")
	SET(ThisModDependencies vis)
ENDIF(ModuleEnabled)

SET(ThisModStatus STABLE)
SET(ThisModExternalDependencies FALSE)
