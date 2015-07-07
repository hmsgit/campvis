# modules/openigtlink/openigtlink.cmake
# CMake file for the openigtlink module

# Set module status (valid values are STABLE, TESTING and EXPERIMENTAL)
SET(ThisModStatus TESTING)
# Set whether this module has external dependencies that are not shipped with CAMPVis.
SET(ThisModExternalDependencies TRUE)

# The files and assignments need only to be parsed if the module is enabled
IF(ModuleEnabled)
    FIND_PACKAGE(OpenIGTLink REQUIRED)

    IF(OpenIGTLink_FOUND)
        # Source files:
        FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
            modules/openigtlink/pipelines/*.cpp
            modules/openigtlink/processors/*.cpp
            modules/openigtlink/datastructures/*.cpp
            modules/openigtlink/*.cpp
        )

        # Header files (including GLSL files so that they'll appear in VS projects)
        FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
            modules/openigtlink/glsl/*.frag
            modules/openigtlink/glsl/*.geom
            modules/openigtlink/glsl/*.vert
            modules/openigtlink/pipelines/*.h
            modules/openigtlink/processors/*.h
            modules/openigtlink/datastructures/*.h
        )

        # Define the GLSL shader path, so that all needed shaders will be deployed to target directory
        #SET(ThisModShaderDirectories "modules/openigtlink/glsl")

        # Define dependency modules (the pipelines in the vis modules use processors from io)
        #SET(ThisModDependencies io)
        
        LIST(APPEND ThisModIncludeDirs ${OpenIGTLink_INCLUDE_DIRS})
        LIST(APPEND ThisModExternalLibs ${OpenIGTLink_LIBRARIES})
        LIST(APPEND ThisModLinkDirectories ${OpenIGTLink_LIBRARY_DIRS})
        
        LIST(APPEND ThisModExternalDllsDebug "${OpenIGTLink_LIBRARY_DIRS}/Debug/igtlutil.dll" "${OpenIGTLink_LIBRARY_DIRS}/Debug/OpenIGTLink.dll")
        LIST(APPEND ThisModExternalDllsRelease "${OpenIGTLink_LIBRARY_DIRS}/Release/igtlutil.dll" "${OpenIGTLink_LIBRARY_DIRS}/Release/OpenIGTLink.dll")
        
    ELSE()
        MESSAGE(FATAL_ERROR "Could not find OpenIGTLink. Please specify OpenIGTLink_DIR.")
    ENDIF()
ENDIF(ModuleEnabled)