# CMake file for OpenCL module

IF(NOT OPENCL_FOUND)
	MESSAGE(FATAL_ERROR "OpenCL was not found but is mandatory for OpenCL module.")
ENDIF(NOT OPENCL_FOUND)

# Source files:
FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
	modules/opencl/pipelines/*.cpp
	modules/opencl/processors/*.cpp
)

# Header files (including CL files so that they'll appear in VS projects)
FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
    modules/opencl/glsl/*.cl
	modules/opencl/pipelines/*.h
	modules/opencl/processors/*.h
)


LIST(APPEND ThisModExternalLibs kisscl)