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
PROJECT(LogHard VERSION 0.1.0 LANGUAGES CXX)

INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/config.local" OPTIONAL)
INCLUDE("${CMAKE_CURRENT_BINARY_DIR}/config.local" OPTIONAL)

FIND_PACKAGE(SharemindCMakeHelpers REQUIRED)

SharemindSetProjectVersion()
SET(LogHard_COMPAT_VERSION "${LogHard_VERSION}")

FIND_PACKAGE(SharemindCHeaders REQUIRED)
FIND_PACKAGE(SharemindCxxHeaders REQUIRED)

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

SharemindSetCxx11CompileOptions()
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
SharemindAddSharedLibrary(LogHard SOURCES ${LogHard_SOURCES})
TARGET_LINK_LIBRARIES(LogHard ${SharemindCHeaders_LIBRARIES}
                              ${SharemindCxxHeaders_LIBRARIES})

SharemindUseCMakeFindFiles()


# Packaging:
SharemindSetupPackaging()
SharemindAddComponentPackage(lib
    NAME "libloghard"
    DESCRIPTION "LogHard logging library"
    DEB_SECTION "libs"
    DEB_DEPENDS "libstdc++6 (>= 4.8.0)" "libc6 (>= 2.19)")
SharemindAddComponentPackage(dev
    NAME "libloghard-dev"
    DESCRIPTION "LogHard logging library development headers"
    DEB_SECTION "libdevel"
    DEB_DEPENDS
        "libloghard (= ${LogHard_VERSION})"
        "libsharemind-cheaders-dev (>= 0.1.0)"
        "libsharemind-cxxheaders-dev (>= 0.1.0)"
        "libc6-dev (>= 2.19)"
        "libstdc++-dev"
        "libgcc-4.8-dev"
        "| libgcc-4.9-dev"
        "| libgcc-5-dev"
        "| libgcc-6-dev"
        "| libgcc-7-dev"
)
SharemindAddComponentPackage(debug
    NAME "libloghard-dbg"
    DESCRIPTION "LogHard logging library debug symbols"
    DEB_SECTION "debug"
    DEB_DEPENDS "libloghard (= ${LogHard_VERSION})")
SharemindPackagingFinalize()
