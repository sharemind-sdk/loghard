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

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(LogHard CXX)
SET(LOGHARD_VERSION "0.1.0")
SET(LOGHARD_COMPAT_VERSION "0.1.0")
SET(LOGHARDC_VERSION "0.1.0")
SET(LOGHARDC_COMPAT_VERSION "0.1.0")

INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/config.local" OPTIONAL)
INCLUDE("${CMAKE_CURRENT_BINARY_DIR}/config.local" OPTIONAL)

FIND_PACKAGE(SharemindCHeaders REQUIRED)
FIND_PACKAGE(SharemindCxxHeaders REQUIRED)

SET(LOGHARD_INSTALL_LINK_LIBRARIES "")
LIST(APPEND LOGHARD_INSTALL_LINK_LIBRARIES
    ${SharemindCxxHeaders_LIBRARIES}
    "-L${CMAKE_INSTALL_PREFIX}/lib" "-lLogHard"
)
LIST(REMOVE_DUPLICATES LOGHARD_INSTALL_LINK_LIBRARIES)

SET(LOGHARDC_INSTALL_LINK_LIBRARIES "")
LIST(APPEND LOGHARDC_INSTALL_LINK_LIBRARIES
    ${LOGHARD_INSTALL_LINK_LIBRARIES}
    "-L${CMAKE_INSTALL_PREFIX}/lib" "-lLogHardC"
)
LIST(REMOVE_DUPLICATES LOGHARDC_INSTALL_LINK_LIBRARIES)

SET(LOGHARD_INSTALL_INCLUDE_DIRS "")
LIST(APPEND LOGHARD_INSTALL_INCLUDE_DIRS
    "${CMAKE_INSTALL_PREFIX}/include"
    ${SharemindCHeaders_INCLUDE_DIRS}
    ${SharemindCxxHeaders_INCLUDE_DIRS}
)
LIST(REMOVE_DUPLICATES LOGHARD_INSTALL_INCLUDE_DIRS)
SET(LOGHARDC_INSTALL_INCLUDE_DIRS "")
LIST(APPEND LOGHARDC_INSTALL_INCLUDE_DIRS ${LOGHARD_INSTALL_INCLUDE_DIRS})

SET(LOGHARD_INSTALL_DEFINITIONS "")
LIST(APPEND LOGHARD_INSTALL_DEFINITIONS
    ${SharemindCHeaders_DEFINITIONS}
    ${SharemindCxxHeaders_DEFINITIONS}
)
IF(APPLE)
    LIST(APPEND LOGHARD_INSTALL_DEFINITIONS "-D_DARWIN_C_SOURCE")
ENDIF()
LIST(REMOVE_DUPLICATES LOGHARD_INSTALL_DEFINITIONS)
SET(LOGHARDC_INSTALL_DEFINITIONS "")
LIST(APPEND LOGHARDC_INSTALL_DEFINITIONS ${LOGHARD_INSTALL_DEFINITIONS})

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
INSTALL(FILES ${AllHeaders} DESTINATION "include/LogHard")

FILE(GLOB_RECURSE LogHard_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp")
ADD_LIBRARY(LogHard SHARED ${LogHard_SOURCES})
TARGET_LINK_LIBRARIES(LogHard ${SharemindCHeaders_LIBRARIES}
                              ${SharemindCxxHeaders_LIBRARIES})
STRING(REGEX REPLACE "(([^\\.]+\\.[^\\.]+)\\.).*" "\\2" LOGHARD_SOVERSION
                     "${LOGHARD_VERSION}")
SET_TARGET_PROPERTIES(LogHard PROPERTIES
                      OUTPUT_NAME "LogHard"
                      VERSION "${LOGHARD_VERSION}"
                      SOVERSION "${LOGHARD_SOVERSION}")
INSTALL(TARGETS LogHard LIBRARY DESTINATION "lib")

FILE(GLOB_RECURSE LogHardC_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
LIST(REMOVE_ITEM LogHardC_SOURCES ${LogHard_SOURCES})
ADD_LIBRARY(LogHardC SHARED ${LogHardC_SOURCES})
TARGET_LINK_LIBRARIES(LogHardC LogHard)
STRING(REGEX REPLACE "(([^\\.]+\\.[^\\.]+)\\.).*" "\\2" LOGHARDC_SOVERSION
                     "${LOGHARDC_VERSION}")
SET_TARGET_PROPERTIES(LogHardC PROPERTIES
                      OUTPUT_NAME "LogHardC"
                      VERSION "${LOGCHARD_VERSION}"
                      SOVERSION "${LOGHARDC_SOVERSION}")
INSTALL(TARGETS LogHardC LIBRARY DESTINATION "lib")

FILE(GLOB_RECURSE LogHard_INFILES "${CMAKE_CURRENT_SOURCE_DIR}/*.in")
ADD_CUSTOM_TARGET("include_LogHard_INFILES_in_IDE" SOURCES ${LogHard_INFILES})

FUNCTION(ConfigureFiles name)
    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/${name}Config.cmake.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/${name}Config.cmake" @ONLY)
    INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/${name}Config.cmake"
            DESTINATION "lib/${name}")
    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/${name}ConfigVersion.cmake.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/${name}ConfigVersion.cmake"
                   @ONLY)
    INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/${name}ConfigVersion.cmake"
            DESTINATION "lib/${name}")
ENDFUNCTION()
ConfigureFiles("LogHard")
ConfigureFiles("LogHardC")
