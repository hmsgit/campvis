# CMake file for fontrendering module

IF(${ModuleEnabled})
	# dirty, dirty hack to force freetype to build stastic lib
	SET(foo ${BUILD_SHARED_LIBS})
	SET(BUILD_SHARED_LIBS 0)
	
	# build RadomWalksLib library
	ADD_SUBDIRECTORY(modules/fontrendering/ext/freetype-2.5.4)
	SET(BUILD_SHARED_LIBS ${foo})

	SET(ThisModExternalLibs "freetype")
	SET(ThisModIncludeDirs "${CampvisHome}/modules/fontrendering/ext/freetype-2.5.4/include")
	
	# Source files:
	FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
		modules/fontrendering/pipelines/*.cpp
		modules/fontrendering/processors/*.cpp
		modules/fontrendering/tools/*.cpp
	)

	# Header files (including GLSL files so that they'll appear in VS projects)
	FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
		modules/fontrendering/pipelines/*.h
		modules/fontrendering/processors/*.h
		modules/fontrendering/tools/*.h
	)

	SET(ThisModDependencies base)
ENDIF(${ModuleEnabled})

SET(ThisModStatus TESTING)
SET(ThisModExternalDependencies FALSE)
