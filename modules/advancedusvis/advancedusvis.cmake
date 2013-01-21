# CMake file for AdvancedUsVis module

# Source files:
FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
	modules/advancedusvis/pipelines/*.cpp
	modules/advancedusvis/processors/*.cpp
)

# Header files (including GLSL files so that they'll appear in VS projects)
FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
    modules/advancedusvis/glsl/*.frag
    modules/advancedusvis/glsl/*.vert
	modules/advancedusvis/pipelines/*.h
	modules/advancedusvis/processors/*.h
)
