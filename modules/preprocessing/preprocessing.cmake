# CMake file for Preprocessing module

# Source files:
FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
	modules/preprocessing/processors/*.cpp
	modules/preprocessing/tools/*.cpp
)

# Header files
FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
	modules/preprocessing/glsl/*.frag
	modules/preprocessing/processors/*.h
	modules/preprocessing/tools/*.h
)

SET(ThisModShaderDirectories "modules/preprocessing/glsl")
