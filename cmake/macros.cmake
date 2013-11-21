MACRO(ADD_PIPELINE_REGISTRATION IncludeFile ClassName)
    LIST(APPEND PipelineRegistrationIncludeFiles ${IncludeFile})
    LIST(APPEND PipelineRegistrationClassNames ${ClassName})
ENDMACRO(ADD_PIPELINE_REGISTRATION)

MACRO(WRITE_PIPELINE_REGISTRATION FileName)
    MESSAGE(STATUS "* Generating pipeline registration header: ${FileName}")
    SET(PipelineRegistrationSource "// WARNING: This file is automatically generated by CMake, do not modify!\n\n" )

    LIST(APPEND PipelineRegistrationSource "// Include Pipeline Headers:\n" )
    FOREACH(IncludeFile ${PipelineRegistrationIncludeFiles})
        LIST(APPEND PipelineRegistrationSource "#include \"${IncludeFile}\"\n" )
    ENDFOREACH()
    
    LIST(APPEND PipelineRegistrationSource "\nnamespace campvis {\n" )
    LIST(APPEND PipelineRegistrationSource "\t// Instantiate templated PipelineRegistrars to register the pipelines.\n" )
    FOREACH(ClassName ${PipelineRegistrationClassNames})
        LIST(APPEND PipelineRegistrationSource "\ttemplate class PipelineRegistrar<${ClassName}>\;\n" )
    ENDFOREACH()

    LIST(APPEND PipelineRegistrationSource "}\n\n" )

    FILE(WRITE ${FileName} ${PipelineRegistrationSource})
ENDMACRO(WRITE_PIPELINE_REGISTRATION)

MACRO(PARSE_HEADER_FOR_PIPELINE FileName)
    FILE(READ ${FileName} content)

    # Build a regex matching pipeline declarations and extracting their names
    SET(NameRegex "[A-Za-z0-9_]+")
    SET(FullyQualifiedNameRegex "(::)?(${NameRegex}::)*${NameRegex}")
    SET(BaseClassListRegex "((public|private|protected)( virtual)? ${FullyQualifiedNameRegex}, )*")
    SET(ClassRegex "class (${NameRegex}) ?: ${BaseClassListRegex}public ${FullyQualifiedNameRegex}Pipeline")

    # Find all class definitions inheriting from a Pipeline
    STRING(REGEX MATCHALL ${ClassRegex} matches ${content})
    
    FOREACH(m ${matches})
        # Extract class name and register
        STRING(REGEX REPLACE ${ClassRegex} "\\1" RESULT ${m})
        ADD_PIPELINE_REGISTRATION(${FileName} ${RESULT})
    ENDFOREACH()
ENDMACRO(PARSE_HEADER_FOR_PIPELINE)

MACRO(INCLUDE_MODULE ModuleDirectory ModuleListFile)
    STRING(TOUPPER ${ModuleDirectory} ModuleDirectoryUpper)
    
    LIST(APPEND CampvisEnabledModules ${ModuleDirectory})
    SET(ThisModDir ${ModulesDir}/${ModuleDirectory})

    # load .cmake file
    INCLUDE(${ModuleListFile})

    # merge module settings into global settings
    LIST(APPEND CampvisModulesDefinitions ${ThisModDefinitions})
    LIST(APPEND CampvisModulesIncludeDirs ${ThisModIncludeDirs})
    LIST(APPEND CampvisModulesExternalLibs ${ThisModExternalLibs})
    LIST(APPEND CampvisModulesLinkDirectories ${ThisModLinkDirectories})
    LIST(APPEND CampvisModulesSources ${ThisModSources})
    LIST(APPEND CampvisModulesHeaders ${ThisModHeaders})
    LIST(APPEND CampvisModulesCoreSources ${ThisModCoreSources})
    LIST(APPEND CampvisModulesCoreHeaders ${ThisModCoreHeaders})
    LIST(APPEND CampvisExternalDllsDebug ${ThisModExternalDllsDebug})
    LIST(APPEND CampvisExternalDllsRelease ${ThisModExternalDllsRelease})

    # save dependencies in a variable to resolve them later
    SET(${ModuleDirectory}ModDependencies ${ThisModDependencies})

    # add shader directory to deployment list
    LIST(APPEND CampvisShaderDirectories ${ThisModShaderDirectories})

    # add definition that this module is activated
    LIST(APPEND CampvisGlobalDefinitions -DCAMPVIS_HAS_MODULE_${ModuleDirectoryUpper})

    # parse all header files for pipeline classes to add them to the pipeline registration
    FOREACH(HeaderFile ${ThisModHeaders})
        PARSE_HEADER_FOR_PIPELINE("modules/${HeaderFile}")
    ENDFOREACH()

    # unset module settings to avoid duplicates if module cmake file misses sth.
    UNSET(ThisModDefinitions)
    UNSET(ThisModIncludeDirs)
    UNSET(ThisModExternalLibs)
    UNSET(ThisModLinkDirectories)
    UNSET(ThisModSources)
    UNSET(ThisModHeaders)
    UNSET(ThisModCoreSources)
    UNSET(ThisModCoreHeaders)
    UNSET(ThisModExternalDllsDebug)
    UNSET(ThisModExternalDllsRelease)
    UNSET(ThisModShaderDirectories)
    UNSET(ThisModDependencies)
ENDMACRO(INCLUDE_MODULE)

MACRO(RESOLVE_MODULE_DEPENDENCIES)
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
                SET(CAMPVIS_BUILD_MODULE_${DepUpper} ON CACHE BOOL "Build module ${Dep} (required by ${Mod})" FORCE)
                SET(ModFile ${ModulesDir}/${Dep}/${Dep}.cmake)
                INCLUDE_MODULE(${Dep} ${ModFile})
            ENDIF(DepExists EQUAL -1)
        ENDFOREACH(Dep ${${Mod}ModDependencies})

        UNSET(${Mod}ModDependencies)
    ENDWHILE(CampvisEnabledModules)
ENDMACRO(RESOLVE_MODULE_DEPENDENCIES)

MACRO(SET_DEFAULT_MODULES DefaultModules)
    # Only enable default modules on the first CMake run
    IF(NOT DEFAULT_CAMPVIS_MODULES_SET)
        FOREACH(Mod ${DefaultModules})
            # Check if the module exists
            LIST(FIND CampvisModules ${Mod} ModExists)
            STRING(TOUPPER ${Mod} ModUpper)

            IF(ModExists EQUAL -1)
                MESSAGE(WARNING "Default module '${Mod}' not found!")
            ELSEIF(NOT CAMPVIS_BUILD_MODULE_${ModUpper})
                # Enable the module if required
                MESSAGE(STATUS "Enabling default module '${Mod}'")
                SET(CAMPVIS_BUILD_MODULE_${ModUpper} ON CACHE BOOL "Build default module ${Mod}" FORCE)
                SET(ModFile ${ModulesDir}/${Mod}/${Mod}.cmake)
                INCLUDE_MODULE(${Mod} ${ModFile})
            ENDIF(ModExists EQUAL -1)
        ENDFOREACH(Mod ${DefaultModules})
    ENDIF(NOT DEFAULT_CAMPVIS_MODULES_SET)

    SET(DEFAULT_CAMPVIS_MODULES_SET 1 CACHE INTERNAL "")
ENDMACRO(SET_DEFAULT_MODULES DefaultModules)

# copy and pasted from Voreen...

MACRO(LIST_SUBDIRECTORIES Result Directory AbsolutePath)
  FILE(GLOB sub-dirs RELATIVE ${Directory} ${Directory}/*)
  SET(${Result} "")
  FOREACH(d ${sub-dirs})
    IF(IS_DIRECTORY ${Directory}/${d})
        IF(${AbsolutePath})
            LIST(APPEND ${Result} ${Directory}/${d})
        ELSE()
            LIST(APPEND ${Result} ${d})
        ENDIF()
    ENDIF()
  ENDFOREACH()
  LIST(SORT ${Result})
ENDMACRO(LIST_SUBDIRECTORIES)

# copies the passed debug and release DLLs to bin/Debug and bin/Release, resp.
MACRO(COPY_EXTERNAL_DLLS DebugDLLs ReleaseDLLs failOnError)
    MESSAGE(STATUS "Copying external DLLs")
    
    SET(debug_dir "${CMAKE_BINARY_DIR}/bin/Debug")
    IF(NOT EXISTS ${debug_dir})
        FILE(MAKE_DIRECTORY ${debug_dir})    
    ENDIF()
    FOREACH(dllPath ${${DebugDLLs}})
        IF(EXISTS ${dllPath})
            GET_FILENAME_COMPONENT(dllName ${dllPath} NAME)
            IF(EXISTS ${debug_dir}/${dllName})
                FILE(REMOVE ${debug_dir}/${dllName})
            ENDIF()
            FILE(COPY ${dllPath} DESTINATION ${debug_dir})
        ELSEIF(${failOnError})
            MESSAGE(FATAL_ERROR "Debug DLL not found: ${dllPath}")
        ELSE()
            MESSAGE(WARNING "Debug DLL not found: ${dllPath}")
        ENDIF()
    ENDFOREACH()
    
    SET(release_dir "${CMAKE_BINARY_DIR}/bin/Release")
    IF(NOT EXISTS ${release_dir})
        FILE(MAKE_DIRECTORY ${release_dir})    
    ENDIF()
    FOREACH(dllPath ${${ReleaseDLLs}})
        IF(EXISTS ${dllPath})            
            GET_FILENAME_COMPONENT(dllName ${dllPath} NAME)
            IF(EXISTS ${release_dir}/${dllName})
                FILE(REMOVE ${release_dir}/${dllName})
            ENDIF()
            FILE(COPY ${dllPath} DESTINATION ${release_dir})
        ELSEIF(${failOnError})
            MESSAGE(FATAL_ERROR "Release DLL not found: ${dllPath}")
        ELSE()
            MESSAGE(WARNING "Release DLL not found: ${dllPath}")
        ENDIF()
    ENDFOREACH()
ENDMACRO()

MACRO(DEPLOY_DIRECTORY Target DirectoryList)
    MESSAGE(${Target})
    MESSAGE(${DirectoryList})
    FOREACH(Directory ${DirectoryList})
        ADD_CUSTOM_COMMAND(
            TARGET ${Target}
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_PATH}/${Directory}" "$<TARGET_FILE_DIR:${Target}>/${Directory}"
        )
    ENDFOREACH()
ENDMACRO(DEPLOY_DIRECTORY)

# adapted from: http://stackoverflow.com/questions/148570/using-pre-compiled-headers-with-cmake
MACRO(ADD_MSVC_PRECOMPILED_HEADER PrecompiledHeader PrecompiledSource SourcesVar)
  IF(MSVC)
    GET_FILENAME_COMPONENT(PrecompiledBasename ${PrecompiledHeader} NAME_WE)
    SET(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/${PrecompiledBasename}.pch")
    SET(Sources ${${SourcesVar}})

    SET_SOURCE_FILES_PROPERTIES(${PrecompiledSource}
                                PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_OUTPUTS "${PrecompiledBinary}")
    SET_SOURCE_FILES_PROPERTIES(${Sources}
                                PROPERTIES COMPILE_FLAGS "/Yu\"${PrecompiledBinary}\" /FI\"${PrecompiledBinary}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_DEPENDS "${PrecompiledBinary}")  

    # Add precompiled header to SourcesVar
    LIST(APPEND ${SourcesVar} ${PrecompiledSource})
  ENDIF(MSVC)
ENDMACRO(ADD_MSVC_PRECOMPILED_HEADER)

# adapted from: http://www.mail-archive.com/cmake@cmake.org/msg04394.html
MACRO(ADD_GCC_PRECOMPILED_HEADER _targetName _input )

    GET_FILENAME_COMPONENT(_name ${_input} NAME)
    SET(_source "${CMAKE_CURRENT_SOURCE_DIR}/${_input}")
    SET(_outdir "${CMAKE_CURRENT_BINARY_DIR}/${_name}.gch")
    MAKE_DIRECTORY(${_outdir})
    SET(_output "${_outdir}/${CMAKE_BUILD_TYPE}.c++")
    STRING(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" _flags_var_name)
    SET(_compiler_FLAGS ${${_flags_var_name}})

    GET_TARGET_PROPERTY(_type ${_targetName} TYPE)
    IF(${_type} MATCHES SHARED_LIBRARY)
        LIST(APPEND _compiler_FLAGS "-fPIC")
    ENDIF()

    GET_DIRECTORY_PROPERTY(_directory_flags INCLUDE_DIRECTORIES)
    FOREACH(item ${_directory_flags})
    LIST(APPEND _compiler_FLAGS "-I${item}")
    ENDFOREACH(item)


    GET_DIRECTORY_PROPERTY(_directory_flags DEFINITIONS)
    LIST(APPEND _compiler_FLAGS ${_directory_flags})

    SEPARATE_ARGUMENTS(_compiler_FLAGS)
    #MESSAGE("_compiler_FLAGS: ${_compiler_FLAGS}")
    #message("${CMAKE_CXX_COMPILER} ${_compiler_FLAGS} -x c++-header -o ${_output} ${_source}")
    ADD_CUSTOM_COMMAND(
        OUTPUT ${_output}
        COMMAND ${CMAKE_CXX_COMPILER}
                                ${_compiler_FLAGS}
                                -x c++-header
                                -o ${_output} ${_source}
        DEPENDS ${_source} )
        ADD_CUSTOM_TARGET(${_targetName}_gch DEPENDS ${_output})
    ADD_DEPENDENCIES(${_targetName} ${_targetName}_gch)
    #SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-include ${_name} -Winvalid-pch -H")
    #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include ${_name} -Winvalid-pch")
    SET_TARGET_PROPERTIES(${_targetName} PROPERTIES
        COMPILE_FLAGS "-include ${_name} -Winvalid-pch"
    )
        
ENDMACRO(ADD_GCC_PRECOMPILED_HEADER)

# Assigns the passed Files to source groups according to their file path.
# The path prefixes as well as dot operators are removed from the path.
# E.g.: path/to/my/file.cpp               =>  path\\to\\my
#       <PathPrefix>/path/to/my/file.cpp  =>  path\\to\\my
#       ../../path/to/my/file.cpp         =>  path\\to\\my
MACRO(DEFINE_SOURCE_GROUPS_FROM_SUBDIR Files BasePath RemovePathPrefixes)
    FOREACH(f ${${Files}})
        SET(f_cat "")
        
        # extract relative file path
        IF(IS_ABSOLUTE ${f})
            FILE(RELATIVE_PATH f_rel ${BasePath} ${f})
            GET_FILENAME_COMPONENT(f_rel ${f_rel} PATH)
        ELSE()
            GET_FILENAME_COMPONENT(f_rel ${f} PATH)
        ENDIF()
        
        # create source group specifier from rel path
        IF(f_rel)
            # remove ../
            string(REGEX REPLACE "\\.\\./" "" f_cat ${f_rel})
            # remove specified prefixes
            FOREACH(prefix ${${RemovePathPrefixes}})
                IF(f_cat)
                    string(REGEX REPLACE "${prefix}" "" f_cat ${f_cat})
                ENDIF()
            ENDFOREACH()
            
            # convert path separators into source group separators: 
            # path/to/my => path\\to\\my
            IF(f_cat)
                string(REGEX REPLACE "\\\\|/" "\\\\\\\\" f_cat ${f_cat})
            ENDIF()
        ENDIF()
        
        # set extracted source group
        IF(f_cat)
            SOURCE_GROUP("${f_cat}" FILES ${f})
        ELSE()
            SOURCE_GROUP("" FILES ${f})
        ENDIF()
        
#        MESSAGE(STATUS "${f} -> ${f_cat}")
    ENDFOREACH()
ENDMACRO(DEFINE_SOURCE_GROUPS_FROM_SUBDIR)
