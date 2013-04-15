# CMake file for scr_msk module

FIND_PACKAGE(cisst REQUIRED cisstCommon cisstVector)
FIND_PACKAGE(PCL 1.6 REQUIRED COMPONENTS common)

SET(MskSourceDirectory CACHE PATH "Path to MSK source")
SET(MskLibraryDirectory CACHE PATH "Path to MSK build")
SET(MskBuildDirectory CACHE PATH "Path to MSK build")
SET(RegEngineSourceDir CACHE PATH "Path to RegistrationEngine source")
SET(RegEngineBuildDir CACHE PATH "Path to RegistrationEngine Build")

IF(cisst_FOUND AND MskSourceDirectory AND MskBuildDirectory AND RegEngineSourceDir AND RegEngineBuildDir)
	include (${CISST_USE_FILE})
	
	LIST(APPEND ThisModIncludeDirs ${MskSourceDirectory} "${MskSourceDirectory}/usinterface/include" "${MskSourceDirectory}/extern/include" ${RegEngineSourceDir} ${QT_QTXML_INCLUDE_DIR} ${QT_QTXMLPATTERNS_INCLUDE_DIR})
	LIST(APPEND ThisModExternalLibs debug RegistrationEngined optimized RegistrationEngine)
	LIST(APPEND ThisModExternalLibs debug usinterface-vc10d optimized usinterface-vc10)
	LIST(APPEND ThisModLinkDirectories ${MskLibraryDirectory} ${PCL_LIBRARY_DIRS} "${MSK_SOURCE_DIR}/extern/lib" ${RegEngineBuildDir})
	
	LIST(APPEND ThisModExternalDllsDebug "${RegEngineBuildDir}/Debug/RegistrationEngined.dll" "${cisst_DIR}/lib/Debug/cisstCommon.dll" "${cisst_DIR}/lib/Debug/cisstVector.dll" "${PCL_DIR}/bin/pcl_common_debug.dll" "${MskBuildDirectory}/Debug/usinterface-vc10d.dll")
	LIST(APPEND ThisModExternalDllsRelease "${RegEngineBuildDir}/Release/RegistrationEngine.dll" "${cisst_DIR}/lib/Release/cisstCommon.dll" "${cisst_DIR}/lib/Release/cisstVector.dll" "${PCL_DIR}/bin/pcl_common_release.dll" "${MskBuildDirectory}/Release/usinterface-vc10.dll")
ELSE()
	MESSAGE(FATAL_ERROR "Please specify the path to the MSK project and its dependencies!")
ENDIF()

# Source files:
FILE(GLOB ThisModSources RELATIVE ${ModulesDir}
	modules/scr_msk/datastructures/*.cpp
	modules/scr_msk/pipelines/*.cpp
	modules/scr_msk/processors/*.cpp
)

# Header files (including GLSL files so that they'll appear in VS projects)
FILE(GLOB ThisModHeaders RELATIVE ${ModulesDir}
    modules/scr_msk/glsl/*.frag
    modules/scr_msk/glsl/*.vert
	modules/scr_msk/datastructures/*.h
	modules/scr_msk/pipelines/*.h
	modules/scr_msk/processors/*.h
)
