# CMake file for registration module

# Source files:
FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
	modules/registration/pipelines/*.cpp
	modules/registration/processors/*.cpp
	modules/registration/tools/*.cpp
)

# Header files (including GLSL files so that they'll appear in VS projects)
FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
    modules/registration/glsl/*.frag
    modules/registration/glsl/*.vert
	modules/registration/pipelines/*.h
	modules/registration/processors/*.h
	modules/registration/tools/*.h
)

SET(ThisModShaderDirectories "modules/registration/glsl")

LIST(APPEND ThisModIncludeDirs "C:/Users/Christian/Documents/code/ext/nlopt-2.4/api")
LIST(APPEND ThisModExternalLibs nlopt)
LIST(APPEND ThisModLinkDirectories "C:/Users/Christian/Documents/code/ext/nlopt-2.4/build-x64/Debug" "C:/Users/Christian/Documents/code/ext/nlopt-2.4/build-x64/Release")
