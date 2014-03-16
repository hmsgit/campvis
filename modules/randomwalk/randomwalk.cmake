# CMake file for Randomwalk module

IF(ModuleEnabled)
	# build RadomWalksLib library
	ADD_SUBDIRECTORY(modules/randomwalk/ext/RandomWalksLib)

	SET(ThisModExternalLibs "RandomWalksLib")

	#add additional processors:
	# Source files:
	FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
		modules/randomwalk/processors/*.cpp
	)

	# Header files
	FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
		modules/randomwalk/processors/*.h
	)
ENDIF(ModuleEnabled)

SET(ThisModStatus STABLE)
SET(ThisModExternalDependencies FALSE)
