# CMake file for vis module

IF(${ModuleEnabled})
	# Source files:
	FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
		modules/vis/pipelines/*.cpp
		modules/vis/processors/*.cpp
		modules/vis/tools/*.cpp
	)

	# Header files (including GLSL files so that they'll appear in VS projects)
	FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
		modules/vis/glsl/*.frag
		modules/vis/glsl/*.geom
		modules/vis/glsl/*.vert
		modules/vis/pipelines/*.h
		modules/vis/processors/*.h
		modules/vis/tools/*.h
	)

	LIST(APPEND ThisModShaderDirectories "modules/vis/glsl")
	LIST(APPEND ThisModShaderDirectories "modules/vis/sampledata")
	SET(ThisModDependencies base io)
ENDIF(${ModuleEnabled})

SET(ThisModStatus STABLE)
SET(ThisModExternalDependencies FALSE)
