/*
 * Copyright (C) Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
 */

#ifndef LOGHARD_CAPI_H
#define LOGHARD_CAPI_H

#include <cstring>
#include <exception>
#include <sharemind/abort.h>
#include <sharemind/comma.h>
#include <sharemind/lasterror.h>
#include <sharemind/recursive_locks.h>
#include "Backend.h"
#include "Logger.h"
#include "ErrorC.h"


#define LOGHARD_LASTERROR_FIELDS \
    SHAREMIND_LASTERROR_DECLARE_FIELDS(LogHardError)

#define LOGHARD_LASTERROR_INIT(className) \
    SHAREMIND_LASTERROR_INIT(className, LOGHARD_OK)

#define LOGHARD_LASTERROR_PRIVATE_FUNCTIONS_DECLARE(ClassName) \
    SHAREMIND_LASTERROR_PRIVATE_FUNCTIONS_DECLARE( \
            ClassName,, \
            LogHardError, \
            SHAREMIND_COMMA visibility("internal")) \
    SHAREMIND_LASTERROR_PRIVATE_SHORTCUT_DECLARE( \
            ClassName, \
            Oom,, \
            SHAREMIND_COMMA visibility("internal")) \
    SHAREMIND_LASTERROR_PRIVATE_SHORTCUT_DECLARE( \
            ClassName, \
            Oor,, \
            SHAREMIND_COMMA visibility("internal")) \
    SHAREMIND_LASTERROR_PRIVATE_SHORTCUT_DECLARE( \
            ClassName, \
            Mie,, \
            SHAREMIND_COMMA visibility("internal")) \
    SHAREMIND_LASTERROR_PRIVATE_SHORTCUT_DECLARE( \
            ClassName, \
            Unknown,, \
            SHAREMIND_COMMA visibility("internal"))

#define LOGHARD_LASTERROR_FUNCTIONS_DEFINE(ClassName) \
    SHAREMIND_LASTERROR_PUBLIC_FUNCTIONS_DEFINE( \
            ClassName,, \
            LogHardError, \
            LOGHARD_OK) \
    SHAREMIND_LASTERROR_PRIVATE_FUNCTIONS_DEFINE( \
            ClassName,, \
            LogHardError, \
            LOGHARD_OK) \
    SHAREMIND_LASTERROR_PRIVATE_SHORTCUT_DEFINE( \
            ClassName, \
            Oom,, \
            LOGHARD_OUT_OF_MEMORY, \
            "Out of memory!") \
    SHAREMIND_LASTERROR_PRIVATE_SHORTCUT_DEFINE( \
            ClassName, \
            Oor,, \
            LOGHARD_IMPLEMENTATION_LIMITS_REACHED, \
            "Implementation limits reached!") \
    SHAREMIND_LASTERROR_PRIVATE_SHORTCUT_DEFINE( \
            ClassName, \
            Mie,, \
            LOGHARD_MUTEX_ERROR, \
            "Mutex initialization error!") \
    SHAREMIND_LASTERROR_PRIVATE_SHORTCUT_DEFINE( \
            ClassName, \
            Unknown,, \
            LOGHARD_UNKNOWN_ERROR, \
            "Unknown error!")


struct LogHardBackend {

    std::shared_ptr<LogHard::Backend> const inner{
        std::make_shared<LogHard::Backend>()};
    SHAREMIND_RECURSIVE_LOCK_DECLARE_FIELDS;
    LOGHARD_LASTERROR_FIELDS;

};

SHAREMIND_RECURSIVE_LOCK_FUNCTIONS_DECLARE_DEFINE(
        LogHardBackend,
        inline,
        SHAREMIND_COMMA visibility("internal"));
LOGHARD_LASTERROR_PRIVATE_FUNCTIONS_DECLARE(LogHardBackend)


struct LogHardLogger {

    LogHardLogger(LogHardBackend * const b, const char * const prefix)
        : inner(b->inner, prefix)
        , backend(b)
    {}

    LogHardLogger(const LogHardLogger * const l,
                         const char * const prefix)
        : inner(l->inner, prefix)
        , backend(l->backend)
    {}

    LogHard::Logger inner;
    LogHardBackend * const backend;

    SHAREMIND_RECURSIVE_LOCK_DECLARE_FIELDS;
    LOGHARD_LASTERROR_FIELDS;

};

SHAREMIND_RECURSIVE_LOCK_FUNCTIONS_DECLARE_DEFINE(
        LogHardLogger,
        inline,
        SHAREMIND_COMMA visibility("internal"));
LOGHARD_LASTERROR_PRIVATE_FUNCTIONS_DECLARE(LogHardLogger)


#define LOGHARD_EXCEPTIONS_TO_C_BEGIN try {
#define LOGHARD_EXCEPTIONS_TO_C_END(ClassName,className,returnValue) \
    } catch (const std::length_error &) { \
        ClassName ## _setErrorOor(className); \
        return (returnValue); \
    } catch (const std::bad_alloc &) { \
        ClassName ## _setErrorOom(className); \
        return (returnValue); \
    } catch (...) { \
        ClassName ## _setErrorUnknown(className); \
        return (returnValue); \
    }
#define LOGHARD_NOEXCEPT_BEGIN try {
#define LOGHARD_NOEXCEPT_END(...) \
    } catch (...) { \
        SHAREMIND_ABORT(__VA_ARGS__); \
    }

#endif /* LOGHARD_CAPI_H */
