# CMake file for AdvancedUsVis module

IF(${ModuleEnabled})
	# Source files:
	FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
		modules/advancedusvis/datastructures/*.cpp
		modules/advancedusvis/pipelines/*.cpp
		modules/advancedusvis/processors/*.cpp
		modules/advancedusvis/properties/*.cpp
		modules/advancedusvis/*.cpp
	)

	# Header files (including GLSL files so that they'll appear in VS projects)
	FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
		modules/advancedusvis/datastructures/*.h
		modules/advancedusvis/glsl/*.frag
		modules/advancedusvis/glsl/*.vert
		modules/advancedusvis/pipelines/*.h
		modules/advancedusvis/processors/*.h
		modules/advancedusvis/properties/*.h
	)

	# campvis-application files
	FILE(GLOB ThisModApplicationSources RELATIVE ${ModulesDir} modules/advancedusvis/widgets/*.cpp)
	FILE(GLOB ThisModApplicationHeaders RELATIVE ${ModulesDir} modules/advancedusvis/widgets/*.h)
	FILE(GLOB ThisModApplicationToBeMocced RELATIVE ${ModulesDir} modules/advancedusvis/widgets/*.h)

	LIST(APPEND ThisModShaderDirectories "modules/advancedusvis/glsl")
	LIST(APPEND ThisModShaderDirectories "modules/advancedusvis/sampledata")
	SET(ThisModDependencies vis io preprocessing randomwalk)
ENDIF(${ModuleEnabled})

SET(ThisModStatus TESTING)
SET(ThisModExternalDependencies FALSE)
