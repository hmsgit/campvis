# CMake file for Preprocessing module

# Source files:
FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
	modules/preprocessing/processors/*.cpp
)

# Header files
FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
	modules/preprocessing/processors/*.h
)
