PROJECT(campvis-modules)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
#INCLUDE(../cmake/commonconf.cmake)

MESSAGE(STATUS "Configuring Campvis Modules")
WRITE_PIPELINE_REGISTRATION("gen_pipelineregistration.h")

LIST(APPEND CampvisModulesHeaders
    gen_pipelineregistration.h
    pipelinefactory.h
)
LIST(APPEND CampvisModulesSources
    pipelinefactory.cpp
)


LINK_DIRECTORIES(${CampvisGlobalLinkDirectories} ${CampvisModulesLinkDirectories})

ADD_LIBRARY(campvis-modules ${CampvisSharedStaticModulesFix}
    ${CampvisModulesSources} ${CampvisModulesHeaders} 
)

ADD_DEFINITIONS(${CampvisGlobalDefinitions} ${CampvisModulesDefinitions})
INCLUDE_DIRECTORIES(${CampvisGlobalIncludeDirs} ${CampvisModulesIncludeDirs})
TARGET_LINK_LIBRARIES(campvis-modules campvis-core tgt sigslot ${CampvisGlobalExternalLibs} ${CampvisModulesExternalLibs})

IF(CAMPVIS_GROUP_SOURCE_FILES)
    DEFINE_SOURCE_GROUPS_FROM_SUBDIR(CampvisModulesSources ${CampvisHome} "")
    DEFINE_SOURCE_GROUPS_FROM_SUBDIR(CampvisModulesHeaders ${CampvisHome} "")
ENDIF()
