PROJECT(test-campvis)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
MESSAGE(STATUS "Configuring googletest for campvis Modules")

FILE(GLOB TestCampvisHeaders RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    *.h
)
FILE(GLOB TestCampvisSources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    *.cpp
    core/datastructures/*.cpp
    core/properties/*.cpp
    core/tools/*.cpp
    core/pipeline/*.cpp
    modules/*.cpp
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
TARGET_LINK_LIBRARIES(test-campvis campvis-modules campvis-core tgt gtest sigslot ${CampvisGlobalExternalLibs} ${CampvisModulesExternalLibs} ${QT_LIBRARIES})

if (CAMPVIS_GROUP_SOURCE_FILES)
    DEFINE_SOURCE_GROUPS_FROM_SUBDIR(TestCampvisSources ${CampvisHome}/test "")
    DEFINE_SOURCE_GROUPS_FROM_SUBDIR(TestCampvisHeaders ${CampvisHome}/test "")
ENDIF()

