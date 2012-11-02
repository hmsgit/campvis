# CMake file for IO module

# Source files:
FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
	modules/io/processors/*.cpp
)

# Header files
FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
	modules/io/processors/*.h
)
