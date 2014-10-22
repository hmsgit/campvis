# CMake file for 3D Microscopy Image Segmentation module

IF(${ModuleEnabled})
	# build AlgLib library

	# dirty, dirty hack to force AlgLib to build stastic lib on MSVC
	SET(bsl ${BUILD_SHARED_LIBS})
	IF(${CampvisSharedStaticModulesFix} STREQUAL "STATIC")
		SET(BUILD_SHARED_LIBS OFF)
	ENDIF()
	
	ADD_SUBDIRECTORY(modules/microscopysegmentation/ext/alglib)
	SET(BUILD_SHARED_LIBS ${bsl})

	SET(ThisModExternalLibs "alglib")

	#add additional processors:
	# Source files:
	FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
		modules/microscopysegmentation/processors/*.cpp
		modules/microscopysegmentation/pipelines/*.cpp
	)

	# Header files
	FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
		modules/microscopysegmentation/processors/*.h
		modules/microscopysegmentation/pipelines/*.h
	)
ENDIF(${ModuleEnabled})

SET(ThisModStatus STABLE)
SET(ThisModExternalDependencies FALSE)
