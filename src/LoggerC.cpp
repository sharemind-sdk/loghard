/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "LoggerC.h"

#include <cassert>
#include "CAPI.h"


extern "C" {

LOGHARD_LASTERROR_FUNCTIONS_DEFINE(LogHardLogger)

LogHardLogger * LogHardBackend_newLogger(LogHardBackend * backend,
                                         const char * prefix)
{
    assert(backend);
    assert(prefix);
    LOGHARD_EXCEPTIONS_TO_C_BEGIN
    LogHardLogger * const logger = new LogHardLogger(backend, prefix);
    LOGHARD_LASTERROR_INIT(logger);
    if (SHAREMIND_RECURSIVE_LOCK_INIT(logger))
        return logger;
    LogHardBackend_setErrorMie(backend);
    return nullptr;
    LOGHARD_EXCEPTIONS_TO_C_END(LogHardBackend, backend, nullptr)
}

void LogHardLogger_free(LogHardLogger * logger) {
    assert(logger);
    LOGHARD_NOEXCEPT_BEGIN
    delete logger;
    LOGHARD_NOEXCEPT_END("LHBd")
}

LogHardBackend * LogHardLogger_backend(const LogHardLogger * logger) {
    assert(logger);
    return logger->backend;
}

LogHardLogger * LogHardLogger_newLogger(LogHardLogger * logger,
                                        const char * prefix)
{
    assert(logger);
    LOGHARD_EXCEPTIONS_TO_C_BEGIN
    LogHardLogger * const l = new LogHardLogger(logger, prefix);
    LOGHARD_LASTERROR_INIT(l);
    if (SHAREMIND_RECURSIVE_LOCK_INIT(l))
        return l;
    LogHardLogger_setErrorMie(logger);
    return nullptr;
    LOGHARD_EXCEPTIONS_TO_C_END(LogHardLogger, logger, nullptr)
}

void LogHardLogger_log(const LogHardLogger * logger,
                       LogHardPriority priority,
                       const char * message)
{
    assert(logger);
    assert(message);
    LOGHARD_NOEXCEPT_BEGIN
    const LogHard::Logger & f = logger->inner;
    static_assert(noexcept(f.fatal() << message),
                  "f.fatal() << message must be noexcept");
    static_assert(noexcept(f.error() << message),
                  "f.error() << message must be noexcept");
    static_assert(noexcept(f.warning() << message),
                  "f.warning() << message must be noexcept");
    static_assert(noexcept(f.info() << message),
                  "f.info() << message must be noexcept");
    static_assert(noexcept(f.debug() << message),
                  "f.debug() << message must be noexcept");
    static_assert(noexcept(f.fullDebug() << message),
                  "f.fullDebug() << message must be noexcept");
    switch (priority) {
        case LOGHARD_PRIORITY_FATAL:     f.fatal() << message;     break;
        case LOGHARD_PRIORITY_ERROR:     f.error() << message;     break;
        case LOGHARD_PRIORITY_WARNING:   f.warning() << message;   break;
        case LOGHARD_PRIORITY_NORMAL:    f.info() << message;      break;
        case LOGHARD_PRIORITY_DEBUG:     f.debug() << message;     break;
        case LOGHARD_PRIORITY_FULLDEBUG: f.fullDebug() << message; break;
        default: SHAREMIND_ABORT("SLlm");
    }
    LOGHARD_NOEXCEPT_END("SLlm2")
}

} // extern "C" {
