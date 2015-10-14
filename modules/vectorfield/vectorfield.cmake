# CMake file for Velocity Field module

IF(${ModuleEnabled})
	# Source files:
	FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
        modules/vectorfield/*.cpp
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

	LIST(APPEND ThisModShaderDirectories "modules/vectorfield/glsl")
	LIST(APPEND ThisModShaderDirectories "modules/vectorfield/sampledata")
	SET(ThisModDependencies io vis)
ENDIF(${ModuleEnabled})

SET(ThisModStatus STABLE)
SET(ThisModExternalDependencies FALSE)
