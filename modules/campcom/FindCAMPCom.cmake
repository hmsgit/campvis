# - Find CAMPCOM
# Find the native CAMPCOM includes and library.
# Once done this will define
#
#  CAMPCOM_INCLUDE_DIRS   - where to find the CAMPCom header files etc.
#  CAMPCOM_LIBRARIES      - List of libraries when using CAMPCOM.
#  CAMPCOM_FOUND          - True if CAMPCOM found.
#
#  CAMPCOM_VERSION_STRING - The version of CAMPCOM found (x.y.z)
#  CAMPCOM_VERSION_MAJOR  - The major version of CAMPCOM
#  CAMPCOM_VERSION_MINOR  - The minor version of CAMPCOM
#  CAMPCOM_VERSION_PATCH  - The patch version of CAMPCOM
#  CAMPCOM_VERSION_TWEAK  - The tweak version of CAMPCOM
#
# The following variable are provided for backward compatibility
#
#  CAMPCOM_MAJOR_VERSION  - The major version of CAMPCOM
#  CAMPCOM_MINOR_VERSION  - The minor version of CAMPCOM
#  CAMPCOM_PATCH_VERSION  - The patch version of CAMPCOM
#
# An includer may set CAMPCOM_ROOT to a CAMPCOM installation root to tell
# this module where to look.


set(_CAMPCOM_SEARCHES)

# Search CAMPCOM_ROOT first if it is set.
if(CAMPCOM_ROOT)
  set(_CAMPCOM_SEARCH_ROOT PATHS ${CAMPCOM_ROOT} NO_DEFAULT_PATH)
  list(APPEND _CAMPCOM_SEARCHES _CAMPCOM_SEARCH_ROOT)
endif()

# Normal search.
set(_CAMPCOM_SEARCH_NORMAL
  PATHS "[HKEY_LOCAL_MACHINE\\SOFTWARE\\GnuWin32\\CAMPCOM;InstallPath]"
        "$ENV{PROGRAMFILES}/CAMPCOM"
		"$ENV{CAMPCOM_DIR}"
		"$ENV{CAMPCOM_ROOT}"
        "${CAMPCOM_BUILD}/bin"
        "${CAMPCOM_BUILD}/bin/Debug"
        "${CAMPCOM_BUILD}/bin/Release"
  )
list(APPEND _CAMPCOM_SEARCHES _CAMPCOM_SEARCH_NORMAL)

set(CAMPCOM_NAMES CCClientLib CCCCommonLib CCServerLib)

# Try each search configuration.
foreach(search ${_CAMPCOM_SEARCHES})
  find_path(CAMPCOM_INCLUDE_DIR NAMES CommonLib/Main/CAMPComDefinitions.h       ${${search}} PATH_SUFFIXES include)
  find_library(CAMPCOM_LIBRARY  NAMES ${CAMPCOM_NAMES} ${${search}} PATH_SUFFIXES lib)
endforeach()

mark_as_advanced(CAMPCOM_LIBRARY CAMPCOM_INCLUDE_DIR)

if(CAMPCOM_INCLUDE_DIR AND EXISTS "${CAMPCOM_INCLUDE_DIR}/CommonLib/Main/CAMPComDefinitions.h")
    file(STRINGS "${CAMPCOM_INCLUDE_DIR}/CommonLib/Main/CAMPComDefinitions.h" CAMPCOM_H REGEX "^#define CAMPCOM_VERSION \"[^\"]*\"$")

    string(REGEX REPLACE "^.*CAMPCOM_VERSION \"([0-9]+).*$" "\\1" CAMPCOM_VERSION_MAJOR "${CAMPCOM_H}")
    string(REGEX REPLACE "^.*CAMPCOM_VERSION \"[0-9]+\\.([0-9]+).*$" "\\1" CAMPCOM_VERSION_MINOR  "${CAMPCOM_H}")
    string(REGEX REPLACE "^.*CAMPCOM_VERSION \"[0-9]+\\.[0-9]+\\.([0-9]+).*$" "\\1" CAMPCOM_VERSION_PATCH "${CAMPCOM_H}")
    set(CAMPCOM_VERSION_STRING "${CAMPCOM_VERSION_MAJOR}.${CAMPCOM_VERSION_MINOR}.${CAMPCOM_VERSION_PATCH}")

    # only append a TWEAK version if it exists:
    set(CAMPCOM_VERSION_TWEAK "")
    if( "${CAMPCOM_H}" MATCHES "^.*CAMPCOM_VERSION \"[0-9]+\\.[0-9]+\\.[0-9]+\\.([0-9]+).*$")
        set(CAMPCOM_VERSION_TWEAK "${CMAKE_MATCH_1}")
        set(CAMPCOM_VERSION_STRING "${CAMPCOM_VERSION_STRING}.${CAMPCOM_VERSION_TWEAK}")
    endif()

    set(CAMPCOM_MAJOR_VERSION "${CAMPCOM_VERSION_MAJOR}")
    set(CAMPCOM_MINOR_VERSION "${CAMPCOM_VERSION_MINOR}")
    set(CAMPCOM_PATCH_VERSION "${CAMPCOM_VERSION_PATCH}")
endif()

# handle the QUIETLY and REQUIRED arguments and set CAMPCOM_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CAMPCOM REQUIRED_VARS CAMPCOM_LIBRARY CAMPCOM_INCLUDE_DIR
                                       VERSION_VAR CAMPCOM_VERSION_STRING)

if(CAMPCOM_FOUND)
    set(CAMPCOM_INCLUDE_DIRS ${CAMPCOM_INCLUDE_DIR})
    set(CAMPCOM_LIBRARIES ${CAMPCOM_LIBRARY})
endif()


LIST(APPEND CAMPCOM_INCLUDE_DIRS 
    ${CAMPCOM_INCLUDE_DIR}/CommonLib/MathLib 
    ${CAMPCOM_INCLUDE_DIR}/CommonLib/MathLib/GL 
    ${CAMPCOM_INCLUDE_DIR}/CommonLib/Messages 
    ${CAMPCOM_INCLUDE_DIR}/CommonLib/Util
    ${CAMPCOM_INCLUDE_DIR}/CommonLib/Main
    ${CAMPCOM_INCLUDE_DIR}/CommonLib/Ext
    ${CAMPCOM_INCLUDE_DIR}/CommonLib/DataTypes
    ${CAMPCOM_INCLUDE_DIR}/TrackingLib/src
    ${CAMPCOM_INCLUDE_DIR}/TrackingLib/NDI
    ${CAMPCOM_INCLUDE_DIR}/TrackingLib/Ascension
    ${CAMPCOM_INCLUDE_DIR}/ClientLib/src
    ${CAMPCOM_INCLUDE_DIR}/ServerLib/src)