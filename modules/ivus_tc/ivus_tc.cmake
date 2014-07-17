# CMake file for ivus_tc module

IF(${ModuleEnabled})
	# Source files:
	FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
		modules/ivus_tc/pipelines/*.cpp
		modules/ivus_tc/processors/*.cpp
	)

	# Header files (including GLSL files so that they'll appear in VS projects)
	FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
		modules/ivus_tc/glsl/*.frag
		modules/ivus_tc/glsl/*.geom
		modules/ivus_tc/glsl/*.vert
		modules/ivus_tc/pipelines/*.h
		modules/ivus_tc/processors/*.h
	)

	SET(ThisModShaderDirectories "modules/ivus_tc/glsl")
	SET(ThisModDependencies base io devil vis)
ENDIF(${ModuleEnabled})

SET(ThisModStatus EXPERIMENTAL)
SET(ThisModExternalDependencies FALSE)
