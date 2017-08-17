#
# Copyright (C) 2015 Cybernetica
#
# Research/Commercial License Usage
# Licensees holding a valid Research License or Commercial License
# for the Software may use this file according to the written
# agreement between you and Cybernetica.
#
# GNU General Public License Usage
# Alternatively, this file may be used under the terms of the GNU
# General Public License version 3.0 as published by the Free Software
# Foundation and appearing in the file LICENSE.GPL included in the
# packaging of this file.  Please review the following information to
# ensure the GNU General Public License version 3.0 requirements will be
# met: http://www.gnu.org/copyleft/gpl-3.0.html.
#
# For further information, please contact us at sharemind@cyber.ee.
#

CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)
PROJECT(LogHard CXX)

SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(LogHard_VERSION
    "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(LogHard_COMPAT_VERSION "${LogHard_VERSION}")

INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/config.local" OPTIONAL)
INCLUDE("${CMAKE_CURRENT_BINARY_DIR}/config.local" OPTIONAL)

FIND_PACKAGE(SharemindCHeaders REQUIRED)
FIND_PACKAGE(SharemindCxxHeaders REQUIRED)
FIND_PROGRAM(objcopy NAMES "objcopy" "${CMAKE_LIBRARY_ARCHITECTURE}-objcopy")
IF(NOT objcopy)
    MESSAGE(FATAL_ERROR "Required program not found: objcopy")
ELSE()
    MESSAGE(STATUS "Found objcopy: ${objcopy}")
ENDIF()

SET(LOGHARD_INSTALL_LINK_LIBRARIES "")
LIST(APPEND LOGHARD_INSTALL_LINK_LIBRARIES
    ${SharemindCxxHeaders_LIBRARIES}
    "-L${CMAKE_INSTALL_PREFIX}/lib" "-lLogHard"
)
LIST(REMOVE_DUPLICATES LOGHARD_INSTALL_LINK_LIBRARIES)

SET(LOGHARD_INSTALL_INCLUDE_DIRS "")
LIST(APPEND LOGHARD_INSTALL_INCLUDE_DIRS
    "${CMAKE_INSTALL_PREFIX}/include"
    ${SharemindCHeaders_INCLUDE_DIRS}
    ${SharemindCxxHeaders_INCLUDE_DIRS}
)
LIST(REMOVE_DUPLICATES LOGHARD_INSTALL_INCLUDE_DIRS)

SET(LOGHARD_INSTALL_DEFINITIONS "")
LIST(APPEND LOGHARD_INSTALL_DEFINITIONS
    ${SharemindCHeaders_DEFINITIONS}
    ${SharemindCxxHeaders_DEFINITIONS}
)
IF(APPLE)
    LIST(APPEND LOGHARD_INSTALL_DEFINITIONS "-D_DARWIN_C_SOURCE")
ENDIF()
LIST(REMOVE_DUPLICATES LOGHARD_INSTALL_DEFINITIONS)

ADD_COMPILE_OPTIONS(
  "-std=c++11" "-Wall" "-Wextra" "-O2"
  "$<$<NOT:$<STREQUAL:$<CONFIGURATION>,Release>>:-ggdb>"
  "$<$<NOT:$<STREQUAL:$<CONFIGURATION>,Release>>:-fno-omit-frame-pointer>"
  "$<$<STREQUAL:$<CONFIGURATION>,Release>:-DNDEBUG>"
  "$<$<STREQUAL:$<CONFIGURATION>,Release>:-fomit-frame-pointer>"
)
INCLUDE_DIRECTORIES(${SharemindCHeaders_INCLUDE_DIRS}
                    ${SharemindCxxHeaders_INCLUDE_DIRS})
ADD_DEFINITIONS(${SharemindCHeaders_DEFINITIONS}
                ${SharemindCxxHeaders_DEFINITIONS})

FILE(GLOB_RECURSE AllHeaders "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
ADD_CUSTOM_TARGET("include_AllHeaders_in_IDE" SOURCES ${AllHeaders})
SET(AllHeaders_P "${CMAKE_CURRENT_SOURCE_DIR}/src/CAPI.h")
LIST(REMOVE_ITEM AllHeaders ${AllHeaders_P})
INSTALL(FILES ${AllHeaders} DESTINATION "include/LogHard" COMPONENT "dev")

FILE(GLOB_RECURSE LogHard_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
ADD_LIBRARY(LogHard SHARED ${LogHard_SOURCES})
TARGET_LINK_LIBRARIES(LogHard ${SharemindCHeaders_LIBRARIES}
                              ${SharemindCxxHeaders_LIBRARIES})
STRING(REGEX REPLACE "(([^\\.]+\\.[^\\.]+)\\.).*" "\\2" LogHard_SOVERSION
                     "${LOGHARD_VERSION}")
SET_TARGET_PROPERTIES(LogHard PROPERTIES
                      OUTPUT_NAME "LogHard"
                      VERSION "${LogHard_VERSION}"
                      SOVERSION "${LogHard_SOVERSION}")
INSTALL(TARGETS LogHard LIBRARY DESTINATION "lib" COMPONENT "lib")

ADD_CUSTOM_COMMAND(TARGET LogHard POST_BUILD
    COMMAND "${objcopy}" "--compress-debug-sections"
                         "$<TARGET_FILE:LogHard>"
                         "$<TARGET_FILE:LogHard>.debug"
    COMMAND "${objcopy}" "--strip-unneeded"
                         "--remove-section=.comment"
                         "--remove-section=.note"
                         "--remove-section=.note.*"
                         "--add-gnu-debuglink=$<TARGET_FILE:LogHard>.debug"
                         "$<TARGET_FILE:LogHard>"
)
INSTALL(FILES "$<TARGET_FILE:LogHard>.debug" DESTINATION "lib/debug"
        COMPONENT "debug")

FILE(GLOB_RECURSE LogHard_INFILES "${CMAKE_CURRENT_SOURCE_DIR}/*.in")
ADD_CUSTOM_TARGET("include_LogHard_INFILES_in_IDE" SOURCES ${LogHard_INFILES})

CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/LogHardConfig.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/LogHardConfig.cmake" @ONLY)
INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/LogHardConfig.cmake"
        DESTINATION "lib/LogHard"
        COMPONENT "dev")
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/LogHardConfigVersion.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/LogHardConfigVersion.cmake"
               @ONLY)
INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/LogHardConfigVersion.cmake"
        DESTINATION "lib/LogHard"
        COMPONENT "dev")


# Packaging:
SET(CPACK_PACKAGE_VENDOR "Cybernetica AS")
SET(CPACK_PACKAGE_CONTACT "Sharemind packaging <sharemind-packaging@cyber.ee>")
SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_NAME "libloghard")
SET(CPACK_DEBIAN_PACKAGE_RELEASE "cyber1")
SET(CPACK_DEBIAN_COMPRESSION_TYPE "xz")
SET(CPACK_DEB_COMPONENT_INSTALL "ON")
SET(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")

SET(CPACK_DEBIAN_LIB_PACKAGE_NAME "libloghard")
SET(CPACK_COMPONENT_DEV_DESCRIPTION "LogHard development headers")
SET(CPACK_COMPONENT_LIB_DESCRIPTION "LogHard logging library")
SET(CPACK_COMPONENT_DEBUG_DESCRIPTION "LogHard logging library debug symbols")
SET(CPACK_DEBIAN_DEV_PACKAGE_DEPENDS
    "libloghard (= ${LogHard_VERSION}), libsharemind-cheaders-dev (>= 0.1.0), libsharemind-cxxheaders-dev (>= 0.1.0), libc6-dev (>= 2.19), libstdc++-dev, libgcc-4.8-dev | libgcc-4.9-dev | libgcc-5-dev | libgcc-6-dev | libgcc-7-dev")
SET(CPACK_DEBIAN_LIB_PACKAGE_DEPENDS "libstdc++6 (>= 4.8.0), libc6 (>= 2.19)")
SET(CPACK_DEBIAN_DEBUG_PACKAGE_DEPENDS "libloghard (= ${LogHard_VERSION})")
SET(CPACK_DEBIAN_DEV_PACKAGE_SECTION "libdevel")
SET(CPACK_DEBIAN_LIB_PACKAGE_SECTION "libs")
SET(CPACK_DEBIAN_DEBUG_PACKAGE_SECTION "debug")
INCLUDE(CPack)
