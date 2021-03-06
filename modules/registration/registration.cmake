# CMake file for registration module

IF(${ModuleEnabled})
	# add NLOPT project as external dependency
	ADD_SUBDIRECTORY(modules/registration/ext/nlopt-stripped)

	LIST(APPEND ThisModIncludeDirs "registration/ext/nlopt-stripped/api")
	SET(ThisModExternalLibs "nlopt")


	# Source files:
	FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
		modules/registration/pipelines/*.cpp
		modules/registration/processors/*.cpp
		modules/registration/tools/*.cpp
		modules/registration/*.cpp
	)

	# Header files (including GLSL files so that they'll appear in VS projects)
	FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
		modules/registration/glsl/*.frag
		modules/registration/glsl/*.vert
		modules/registration/pipelines/*.h
		modules/registration/processors/*.h
		modules/registration/tools/*.h
	)

	LIST(APPEND ThisModShaderDirectories "modules/registration/glsl")
	SET(ThisModDependencies vis io)
ENDIF(${ModuleEnabled})

SET(ThisModStatus EXPERIMENTAL)
SET(ThisModExternalDependencies FALSE)
