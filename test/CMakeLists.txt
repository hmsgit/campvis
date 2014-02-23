PROJECT(test-campvis)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
#INCLUDE(../cmake/commonconf.cmake)

MESSAGE(STATUS "Configuring googletest for campvis Modules")
#SET_DEFAULT_MODULES("vis")
#RESOLVE_MODULE_DEPENDENCIES()
#WRITE_PIPELINE_REGISTRATION("gen_pipelineregistration.h")

FILE(GLOB TestCampvisHeaders
    *.h
)
FILE(GLOB TestCampvisSources
    *.cpp
)

# Summary of tuple support for Microsoft Visual Studio:
# Compiler    version(MS)  version(cmake)  Support
# ----------  -----------  --------------  -----------------------------
# <= VS 2010  <= 10        <= 1600         Use Google Tests's own tuple.
# VS 2012     11           1700            std::tr1::tuple + _VARIADIC_MAX=10
# VS 2013     12           1800            std::tr1::tuple
if (MSVC AND MSVC_VERSION EQUAL 1700)
  add_definitions(/D _VARIADIC_MAX=10)
endif()


LINK_DIRECTORIES(${CampvisGlobalLinkDirectories} ${CampvisModulesLinkDirectories})
ADD_EXECUTABLE(test-campvis
    ${TestCampvisSources} ${TestCampvisHeaders} 
)

ADD_DEFINITIONS(${CampvisGlobalDefinitions} ${CampvisModulesDefinitions} ${CampvisApplicationDefinitions} ${QT_DEFINITIONS})
INCLUDE_DIRECTORIES(${CampvisGlobalIncludeDirs} ${CampvisModulesIncludeDirs} ${CampvisHome}/ext/gtest-1.7.0/ ${CampvisHome}/ext/gtest-1.7.0/include)
TARGET_LINK_LIBRARIES(test-campvis campvis-modules campvis-core tgt gtest_main ${CampvisGlobalExternalLibs} ${CampvisModulesExternalLibs})

IF(CAMPVIS_GROUP_SOURCE_FILES)
    DEFINE_SOURCE_GROUPS_FROM_SUBDIR(TestCampvisSources ${CampvisHome} "")
    DEFINE_SOURCE_GROUPS_FROM_SUBDIR(TestCampvisHeaders ${CampvisHome} "")
ENDIF()
