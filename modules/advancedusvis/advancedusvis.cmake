# CMake file for AdvancedUsVis module

IF(ModuleEnabled)
	# Source files:
	FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
		modules/advancedusvis/pipelines/*.cpp
		modules/advancedusvis/processors/*.cpp
	)

	# Header files (including GLSL files so that they'll appear in VS projects)
	FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
		modules/advancedusvis/glsl/*.frag
		modules/advancedusvis/glsl/*.vert
		modules/advancedusvis/pipelines/*.h
		modules/advancedusvis/processors/*.h
	)

	SET(ThisModShaderDirectories "modules/advancedusvis/glsl")
ENDIF(ModuleEnabled)

SET(ThisModStatus EXPERIMENTAL)
SET(ThisModExternalDependencies FALSE)
