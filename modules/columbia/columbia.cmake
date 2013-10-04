# CMake file for columbia module

# Source files:
FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
	modules/columbia/datastructures/*.cpp
	modules/columbia/pipelines/*.cpp
	modules/columbia/processors/*.cpp
)

# Header files (including GLSL files so that they'll appear in VS projects)
FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
    modules/columbia/datastructures/*.h
    modules/columbia/glsl/*.frag
    modules/columbia/glsl/*.geom
    modules/columbia/glsl/*.vert
	modules/columbia/pipelines/*.h
	modules/columbia/processors/*.h
)

SET(ThisModShaderDirectories "modules/columbia/glsl")

