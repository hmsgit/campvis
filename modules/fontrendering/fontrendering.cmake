# CMake file for fontrendering module

IF(${ModuleEnabled})
	# dirty, dirty hack to force freetype to build stastic lib
	SET(bsl ${BUILD_SHARED_LIBS})
	IF(${CampvisSharedStaticModulesFix} STREQUAL "STATIC")
		SET(BUILD_SHARED_LIBS OFF)
	ENDIF()
	
	# build RadomWalksLib library
	ADD_SUBDIRECTORY(modules/fontrendering/ext/freetype-2.5.4)
	SET(BUILD_SHARED_LIBS ${bsl})

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
