# CMake file for neuro module

IF(${ModuleEnabled})
	# Source files:
	FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
		modules/neuro/pipelines/*.cpp
		modules/neuro/processors/*.cpp
		modules/neuro/tools/*.cpp
	)

	# Header files (including GLSL files so that they'll appear in VS projects)
	FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
		modules/neuro/glsl/*.frag
		modules/neuro/glsl/*.geom
		modules/neuro/glsl/*.vert
		modules/neuro/pipelines/*.h
		modules/neuro/processors/*.h
		modules/neuro/tools/*.h
	)

	LIST(APPEND ThisModShaderDirectories "modules/neuro/glsl")
	LIST(APPEND ThisModShaderDirectories "modules/neuro/sampledata")
	SET(ThisModDependencies base io preprocessing vis)
ENDIF(${ModuleEnabled})

SET(ThisModStatus TESTING)
SET(ThisModExternalDependencies FALSE)
