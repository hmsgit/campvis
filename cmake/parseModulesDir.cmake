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

# Iterate over all enabled modules and their dependencies.
# A WHILE loop is used here because FOREACH doesn't see changes to the list it processes.
# As a result, transitive dependencies would require several CMake runs to be resolved.
WHILE(CampvisEnabledModules)
    LIST(GET CampvisEnabledModules 0 Mod)
    LIST(REMOVE_AT CampvisEnabledModules 0)

    FOREACH(Dep ${${Mod}ModDependencies})
        # Check if the dependency exists
        LIST(FIND CampvisModules ${Dep} DepExists)
        STRING(TOUPPER ${Dep} DepUpper)

        IF(DepExists EQUAL -1)
            MESSAGE(WARNING "Dependency '${Dep}' of module '${Mod}' not found!")
        ELSEIF(NOT CAMPVIS_BUILD_MODULE_${DepUpper})
            # Enable the dependency if required
            MESSAGE(STATUS "Enabling module '${Dep}' (required by '${Mod}')")
            SET(CAMPVIS_BUILD_MODULE_${DepUpper} ON CACHE BOOL "Build Module ${Dep} (required by ${Mod})" FORCE)
            SET(ModFile ${ModulesDir}/${Dep}/${Dep}.cmake)
            INCLUDE_MODULE(${Dep} ${ModFile})
        ENDIF(DepExists EQUAL -1)
    ENDFOREACH(Dep ${${Mod}ModDependencies})

    UNSET(${Mod}ModDependencies)
ENDWHILE(CampvisEnabledModules)

SET(ModulesDirsParsed TRUE)
ENDIF(NOT ModulesDirsParsed)
