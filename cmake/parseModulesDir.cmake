IF(NOT ModulesDirsParsed)

# detect modules
MESSAGE(STATUS "--------------------------------------------------------------------------------")
MESSAGE(STATUS "Detecting installed modules:")

# collect list of directories in modules directories
SET(ModulesDir ${CampvisHome}/modules)
LIST_SUBDIRECTORIES(ModDirs ${ModulesDir} false)

# remove CMake and SVN related directories from list
LIST(REMOVE_ITEM ModDirs CMakeFiles campvis-modules.dir .svn)

# go through each subdirectory
FOREACH(ModDir ${ModDirs})
    # check whether module.cmake file exists
    SET(ModFile ${ModulesDir}/${ModDir}/${ModDir}.cmake)
    IF(EXISTS ${ModFile})
        STRING(TOLOWER ${ModDir} ModDirLower)
        STRING(TOUPPER ${ModDir} ModDirUpper)
        LIST(APPEND CampvisModules ${ModDir})

        # check whether the option to build this very module exists and is checked
        IF(CAMPVIS_BUILD_MODULE_${ModDirUpper})
            MESSAGE(STATUS "* Found Module '${ModDir}' : ENABLED")
            INCLUDE_MODULE(${ModDir} ${ModFile})
        ELSE()
            MESSAGE(STATUS "* Found Module '${ModDir}'")
        ENDIF(CAMPVIS_BUILD_MODULE_${ModDirUpper})
        
        IF(NOT DEFINED CAMPVIS_BUILD_MODULE_${ModDirUpper})
            # add a CMake option for building this module
            OPTION(CAMPVIS_BUILD_MODULE_${ModDirUpper}  "Build Module ${ModDir}" OFF)
        ENDIF(NOT DEFINED CAMPVIS_BUILD_MODULE_${ModDirUpper})
        
    ELSE(EXISTS ${ModFile})
        MESSAGE(STATUS "* WARNING: Found Directory ${ModDir} Without CMake file - ignored")
    ENDIF(EXISTS ${ModFile})
    
ENDFOREACH(ModDir ${ModDirs})

SET(ModulesDirsParsed TRUE)
ENDIF(NOT ModulesDirsParsed)
