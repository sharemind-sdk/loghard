/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "BackendC.h"

#include "CAPI.h"


extern "C" {

LOGHARD_LASTERROR_FUNCTIONS_DEFINE(LogHardBackend)

LogHardBackend * LogHardBackend_new(LogHardError * error,
                                    const char ** errorStr)
{
    const auto setError = [=](const LogHardError e, const char * msg) {
        if (error)
            (*error) = e;
        if (errorStr)
            (*errorStr) = msg;
    };

    try {
        LogHardBackend * const backend = new LogHardBackend();
        LOGHARD_LASTERROR_INIT(backend);
        if (SHAREMIND_RECURSIVE_LOCK_INIT(backend))
            return backend;
        setError(LOGHARD_MUTEX_ERROR, "Mutex initialization error!");
    } catch (const std::bad_alloc & e) {
        setError(LOGHARD_OUT_OF_MEMORY, "Out of memory!");
    } catch (...) {
        setError(LOGHARD_UNKNOWN_ERROR, "Unknown error!");
    }
    return nullptr;
}

void LogHardBackend_free(LogHardBackend * backend) {
    assert(backend);
    LOGHARD_NOEXCEPT_BEGIN
    delete backend;
    LOGHARD_NOEXCEPT_END("LHBd")
}

bool LogHardBackend_addStdAppender(LogHardBackend * backend) {
    assert(backend);
    LOGHARD_EXCEPTIONS_TO_C_BEGIN
    backend->inner.addStdAppender();
    return true;
    LOGHARD_EXCEPTIONS_TO_C_END(LogHardBackend,backend, false)
}

bool LogHardBackend_addCFileAppender(LogHardBackend * backend, FILE * file) {
    assert(backend);
    LOGHARD_EXCEPTIONS_TO_C_BEGIN
    backend->inner.addCFileAppender(file);
    return true;
    LOGHARD_EXCEPTIONS_TO_C_END(LogHardBackend,backend, false)
}

bool LogHardBackend_addFileAppender(LogHardBackend * backend,
                                    const char * filename,
                                    bool append)
{
    assert(backend);
    assert(filename);
    LOGHARD_EXCEPTIONS_TO_C_BEGIN
    backend->inner.addFileAppender(filename,
                                   append
                                   ? LogHard::Backend::FileAppender::APPEND
                                   : LogHard::Backend::FileAppender::OVERWRITE);
    return true;
    LOGHARD_EXCEPTIONS_TO_C_END(LogHardBackend,backend, false)
}

} // extern "C" {
