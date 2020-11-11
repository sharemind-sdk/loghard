#
# Copyright (C) Cybernetica
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
PROJECT(LogHard VERSION 0.5.0 LANGUAGES CXX)

INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/config.local" OPTIONAL)
INCLUDE("${CMAKE_CURRENT_BINARY_DIR}/config.local" OPTIONAL)

FIND_PACKAGE(SharemindCMakeHelpers 1.6 REQUIRED)

FIND_PACKAGE(Boost 1.62 COMPONENTS filesystem program_options system REQUIRED)
FIND_PACKAGE(SharemindCHeaders 1.3.0 REQUIRED)
FIND_PACKAGE(SharemindCxxHeaders 0.8.0 REQUIRED)


SharemindSetCxx14CompileOptions()
FILE(GLOB_RECURSE LogHard_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
SET(LogHard_HEADERS_P "${CMAKE_CURRENT_SOURCE_DIR}/src/CAPI.h")
SET(LogHard_HEADERS_INSTALL "${LogHard_HEADERS}")
LIST(REMOVE_ITEM LogHard_HEADERS_INSTALL ${LogHard_HEADERS_P})
INSTALL(FILES ${LogHard_HEADERS_INSTALL} DESTINATION "include/LogHard"
        COMPONENT "dev")

FILE(GLOB_RECURSE LogHard_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
SharemindAddSharedLibrary(LogHard
    SOURCES
        ${LogHard_SOURCES}
        ${LogHard_HEADERS}
    )
TARGET_INCLUDE_DIRECTORIES(LogHard
    INTERFACE
        # $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src> # TODO
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
    )
TARGET_LINK_LIBRARIES(LogHard
    PUBLIC
        Boost::boost
        Sharemind::CHeaders
        Sharemind::CxxHeaders
    PRIVATE
        Boost::filesystem
        Boost::program_options
        Boost::system
)
IF(APPLE)
    TARGET_COMPILE_DEFINITIONS(LogHard PUBLIC "_DARWIN_C_SOURCE")
ENDIF()
SharemindCreateCMakeFindFilesForTarget(LogHard NAMESPACE LogHard
    DEPENDENCIES
        "Boost 1.62"
        "SharemindCHeaders 1.3.0"
        "SharemindCxxHeaders 0.8.0"
    )


# Tests:
FILE(GLOB LogHard_TESTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/Test*.cpp")
FOREACH(testFile IN LISTS LogHard_TESTS)
    GET_FILENAME_COMPONENT(testName "${testFile}" NAME_WE)
    SharemindAddTest("${testName}" SOURCES "${testFile}")
    TARGET_LINK_LIBRARIES("${testName}" PRIVATE LogHard)
ENDFOREACH()


# Packaging:
SharemindSetupPackaging()
SET(BV
    "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}"
)
SharemindAddComponentPackage(lib
    NAME "libloghard"
    DESCRIPTION "LogHard logging library"
    DEB_SECTION "libs"
    DEB_DEPENDS
        "libboost-filesystem${BV}"
        "libboost-program-options${BV}"
        "libboost-system${BV}"
        "libstdc++6 (>= 4.8.0)"
        "libc6 (>= 2.19)"
)
SharemindAddComponentPackage(dev
    NAME "libloghard-dev"
    DESCRIPTION "LogHard logging library development headers"
    DEB_SECTION "libdevel"
    DEB_DEPENDS
        "libloghard (= ${LogHard_DEB_lib_PACKAGE_VERSION})"
        "libboost-dev (>= 1.62)"
        "libsharemind-cheaders-dev (>= 1.3.0)"
        "libsharemind-cxxheaders-dev (>= 0.8.0)"
        "libc6-dev (>= 2.19)"
        "libstdc++-dev"
)
SharemindAddComponentPackage(debug
    NAME "libloghard-dbg"
    DESCRIPTION "LogHard logging library debug symbols"
    DEB_SECTION "debug"
    DEB_DEPENDS "libloghard (= ${LogHard_DEB_lib_PACKAGE_VERSION})")
SharemindPackagingFinalize()
