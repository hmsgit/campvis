# CMake file for IXPV module

IF(ModuleEnabled)
	# Source files:
	FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
		modules/ixpv/pipelines/*.cpp
		modules/ixpv/processors/*.cpp
	)

	# Header files (including GLSL files so that they'll appear in VS projects)
	FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
		modules/ixpv/glsl/*.frag
		modules/ixpv/glsl/*.vert
		modules/ixpv/pipelines/*.h
		modules/ixpv/processors/*.h
	)

	SET(ThisModShaderDirectories "modules/ixpv/glsl")
	SET(ThisModDependencies vis devil io)
ENDIF(ModuleEnabled)

SET(ThisModStatus TESTING)
SET(ThisModExternalDependencies FALSE)
