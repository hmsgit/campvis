# CMake file for OpenCL module

# Source files:
FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
	modules/opencl/processors/*.cpp
)

# Header files (including CL files so that they'll appear in VS projects)
FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
    modules/opencl/glsl/*.cl
	modules/opencl/processors/*.h
)


LIST(APPEND ThisModExternalLibs kisscl)