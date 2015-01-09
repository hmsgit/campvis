# CMake file for Velocity Field module

IF(${ModuleEnabled})
	# Source files:
	FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
		modules/vectorfield/processors/*.cpp
		modules/vectorfield/pipelines/*.cpp
	)

	# Header files
	FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
		modules/vectorfield/glsl/*.vert
		modules/vectorfield/glsl/*.frag
		modules/vectorfield/processors/*.h
		modules/vectorfield/pipelines/*.h
	)

	SET(ThisModShaderDirectories "modules/vectorfield/glsl")
	SET(ThisModShaderDirectories "modules/vectorfield/sampledata")
	SET(ThisModDependencies io vis)
ENDIF(${ModuleEnabled})

SET(ThisModStatus STABLE)
SET(ThisModExternalDependencies FALSE)
