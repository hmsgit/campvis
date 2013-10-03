# CMake file for vis module

# Source files:
FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
	modules/vis/pipelines/*.cpp
	modules/vis/processors/*.cpp
)

# Header files (including GLSL files so that they'll appear in VS projects)
FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
    modules/vis/glsl/*.frag
    modules/vis/glsl/*.vert
	modules/vis/pipelines/*.h
	modules/vis/processors/*.h
)

SET(ThisModShaderDirectories "modules/vis/glsl")

ADD_PIPELINE_REGISTRATION("modules/vis/pipelines/dvrvis.h" "DVRVis")
ADD_PIPELINE_REGISTRATION("modules/vis/pipelines/ixpvdemo.h" "IxpvDemo")
ADD_PIPELINE_REGISTRATION("modules/vis/pipelines/slicevis.h" "SliceVis")
ADD_PIPELINE_REGISTRATION("modules/vis/pipelines/volumeexplorerdemo.h" "VolumeExplorerDemo")
ADD_PIPELINE_REGISTRATION("modules/vis/pipelines/volumerendererdemo.h" "VolumeRendererDemo")
