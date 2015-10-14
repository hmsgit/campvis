# CMake file for fontrendering module

IF(${ModuleEnabled})
	# dirty, dirty hack to force freetype to build stastic lib
	SET(bsl ${BUILD_SHARED_LIBS})
	IF(CampvisSharedStaticModulesFix)
		IF(${CampvisSharedStaticModulesFix} STREQUAL "STATIC")
			SET(BUILD_SHARED_LIBS OFF)
		ENDIF()
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
		modules/fontrendering/*.cpp
	)

	# Header files (including GLSL files so that they'll appear in VS projects)
	FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
		modules/fontrendering/glsl/*.frag
		modules/fontrendering/glsl/*.geom
		modules/fontrendering/glsl/*.vert
		modules/fontrendering/pipelines/*.h
		modules/fontrendering/processors/*.h
		modules/fontrendering/tools/*.h
	)

	LIST(APPEND ThisModShaderDirectories "modules/fontrendering/fonts")
	LIST(APPEND ThisModShaderDirectories "modules/fontrendering/glsl")

	SET(ThisModDependencies base)
ENDIF(${ModuleEnabled})

SET(ThisModStatus STABLE)
SET(ThisModExternalDependencies FALSE)
