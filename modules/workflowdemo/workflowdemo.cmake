# CMake file for workflowdemo module

IF(${ModuleEnabled})
	# Source files:
	FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
		modules/workflowdemo/pipelines/*.cpp
		modules/workflowdemo/workflows/*.cpp
	)

	# Header files (including GLSL files so that they'll appear in VS projects)
	FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
		modules/workflowdemo/pipelines/*.h
		modules/workflowdemo/workflows/*.h
	)

	SET(ThisModDependencies base io preprocessing vis)
ENDIF(${ModuleEnabled})

SET(ThisModStatus TESTING)
SET(ThisModExternalDependencies FALSE)
