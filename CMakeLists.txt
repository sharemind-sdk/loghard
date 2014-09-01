#
# Copyright (C) Cybernetica AS
#
# All rights are reserved. Reproduction in whole or part is prohibited
# without the written consent of the copyright owner. The usage of this
# code is subject to the appropriate license agreement.
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(LogHard CXX)

INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/config.local" OPTIONAL)
INCLUDE("${CMAKE_CURRENT_BINARY_DIR}/config.local" OPTIONAL)

SET(LOGHARD_VERSION "0.1.0")

FIND_PACKAGE(Fluffy REQUIRED)
FIND_PACKAGE(SharemindCompilerSupport REQUIRED)
FIND_PACKAGE(SharemindCxxHeaders REQUIRED)

SET(LOGHARD_INSTALL_LINK_LIBRARIES "")
LIST(APPEND LOGHARD_INSTALL_LINK_LIBRARIES
    ${SharemindCompilerSupport_LIBRARIES}
    ${SharemindCxxHeaders_LIBRARIES}
    ${Fluffy_Mutex_LIBRARIES}
    "-L${CMAKE_INSTALL_PREFIX}/lib" "-lLogHard"
)
LIST(REMOVE_DUPLICATES LOGHARD_INSTALL_LINK_LIBRARIES)

SET(LOGHARD_INSTALL_INCLUDE_DIRS "")
LIST(APPEND LOGHARD_INSTALL_INCLUDE_DIRS
    "${CMAKE_INSTALL_PREFIX}/include"
    ${SharemindCompilerSupport_INCLUDE_DIRS}
    ${SharemindCxxHeaders_INCLUDE_DIRS}
    ${Fluffy_INCLUDE_DIRS}
)
LIST(REMOVE_DUPLICATES LOGHARD_INSTALL_INCLUDE_DIRS)

SET(LOGHARD_INSTALL_DEFINITIONS "")
LIST(APPEND LOGHARD_INSTALL_DEFINITIONS
    ${SharemindCompilerSupport_DEFINITIONS}
    ${SharemindCxxHeaders_DEFINITIONS}
    ${Fluffy_DEFINITIONS}
)
LIST(REMOVE_DUPLICATES LOGHARD_INSTALL_DEFINITIONS)

ADD_COMPILE_OPTIONS(
  "-std=c++11" "-Wall" "-Wextra" "-O2"
  "$<$<NOT:$<STREQUAL:$<CONFIGURATION>,Release>>:-ggdb>"
  "$<$<NOT:$<STREQUAL:$<CONFIGURATION>,Release>>:-fno-omit-frame-pointer>"
  "$<$<STREQUAL:$<CONFIGURATION>,Release>:-DNDEBUG>"
  "$<$<STREQUAL:$<CONFIGURATION>,Release>:-fomit-frame-pointer>"
)
INCLUDE_DIRECTORIES(${SharemindCompilerSupport_INCLUDE_DIRS}
                    ${Fluffy_INCLUDE_DIRS})
ADD_DEFINITIONS(${SharemindCompilerSupport_DEFINITIONS}
                ${Fluffy_DEFINITIONS})

FILE(GLOB_RECURSE LogHard_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
ADD_CUSTOM_TARGET("include_LogHard_HEADERS_in_IDE" SOURCES ${LogHard_HEADERS})
INSTALL(FILES ${LogHard_HEADERS} DESTINATION "include/LogHard")

FILE(GLOB_RECURSE LogHard_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
ADD_LIBRARY(LogHard SHARED ${LogHard_SOURCES})
TARGET_LINK_LIBRARIES(LogHard ${SharemindCompilerSupport_LIBRARIES}
                              ${Fluffy_Mutex_LIBRARIES})
SET_TARGET_PROPERTIES(LogHard PROPERTIES
                      OUTPUT_NAME "LogHard"
                      VERSION "${LOGHARD_VERSION}"
                      SOVERSION "0.1")
INSTALL(TARGETS LogHard LIBRARY DESTINATION "lib")

FILE(GLOB_RECURSE LogHard_INFILES "${CMAKE_CURRENT_SOURCE_DIR}/*.in")
ADD_CUSTOM_TARGET("include_LogHard_INFILES_in_IDE" SOURCES ${LogHard_INFILES})

CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/LogHardConfig.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/LogHardConfig.cmake" @ONLY)
INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/LogHardConfig.cmake"
        DESTINATION "lib/LogHard")
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/LogHardConfigVersion.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/LogHardConfigVersion.cmake" @ONLY)
INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/LogHardConfigVersion.cmake"
        DESTINATION "lib/LogHard")
