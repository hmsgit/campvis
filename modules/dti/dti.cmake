# CMake file for dti module

IF(${ModuleEnabled})
	# Source files:
	FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
		modules/dti/datastructures/*.cpp
		modules/dti/pipelines/*.cpp
		modules/dti/processors/*.cpp
	)

	# Header files (including GLSL files so that they'll appear in VS projects)
	FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
		modules/dti/datastructures/*.h
		modules/dti/glsl/*.frag
		modules/dti/glsl/*.geom
		modules/dti/glsl/*.vert
		modules/dti/pipelines/*.h
		modules/dti/processors/*.h
	)

	LIST(APPEND ThisModShaderDirectories "modules/dti/glsl")
ENDIF(${ModuleEnabled})

SET(ThisModStatus TESTING)
SET(ThisModExternalDependencies FALSE)
