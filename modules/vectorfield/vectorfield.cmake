# CMake file for Velocity Field module

# Source files:
FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
	modules/vectorfield/processors/*.cpp
	modules/vectorfield/pipelines/*.cpp
)

# Header files
FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
	modules/vectorfield/glsl/*.vert
	modules/vectorfield/glsl/*.frag
	modules/vectorfield/processors/*.h
	modules/vectorfield/pipelines/*.h
)

SET(ThisModShaderDirectories "modules/pipelines/glsl")
SET(ThisModDependencies io vis)

