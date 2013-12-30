# CMake file for Tensor module

# Source files:
FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
	modules/tensor/processors/*.cpp
	modules/tensor/pipelines/*.cpp
)

# Header files
FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
	modules/tensor/glsl/*.frag
	modules/tensor/processors/*.h
	modules/tensor/pipelines/*.h
)

SET(ThisModShaderDirectories "modules/pipelines/glsl")
SET(ThisModDependencies io vis)
