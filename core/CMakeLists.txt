PROJECT(campvis-core)
INCLUDE(../cmake/commonconf.cmake)

MESSAGE(STATUS "Configuring CAMPVis-Core")

# glob sources from core directories
FILE(GLOB CampvisCoreSources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    *.cpp
    classification/*.cpp
    datastructures/*.cpp
    eventhandlers/*.cpp
    pipeline/*.cpp
    properties/*.cpp
    tools/*.cpp
)

# add additional sources from the activated modules
FOREACH(CampvisModulesCoreSource ${CampvisModulesCoreSources})
    # adjust relative path
    SET(ModCoreFile ../modules/${CampvisModulesCoreSource})
    LIST(APPEND CampvisCoreSources ${ModCoreFile})
ENDFOREACH()


FILE(GLOB CampvisCoreHeaders RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    *.h
    classification/*.h
    datastructures/*.h
    eventhandlers/*.h
    pipeline/*.h
    properties/*.h
    tools/*.h
    glsl/*.vert
    glsl/*.frag
    glsl/tools/*.vert
    glsl/tools/*.frag
)

# add additional sources from the activated modules
FOREACH(CampvisModulesCoreHeader ${CampvisModulesCoreHeaders})
    # adjust relative path
    SET(ModHeaderFile ../modules/${CampvisModulesCoreHeader})
    LIST(APPEND CampvisCoreHeaders ${ModHeaderFile})
ENDFOREACH()

ADD_LIBRARY(campvis-core 
    ${CampvisCoreSources} ${CampvisCoreHeaders} 
)
ADD_DEFINITIONS(${CampvisGlobalDefinitions})
INCLUDE_DIRECTORIES(${CampvisGlobalIncludeDirs})
TARGET_LINK_LIBRARIES(campvis-core cgt sigslot ${CampvisGlobalExternalLibs})

# if campvis-core is built as a shared library, CMake will define the following flag to instruct
# the code to export DLL symbols
SET_TARGET_PROPERTIES(campvis-core PROPERTIES DEFINE_SYMBOL "CAMPVIS_CORE_BUILD_DLL")

IF(CAMPVIS_GROUP_SOURCE_FILES)
    DEFINE_SOURCE_GROUPS_FROM_SUBDIR(CampvisCoreSources ${CampvisHome} "")
    DEFINE_SOURCE_GROUPS_FROM_SUBDIR(CampvisCoreHeaders ${CampvisHome} "")
ENDIF()


INSTALL(TARGETS campvis-core DESTINATION exports EXPORT campvis-targets)
