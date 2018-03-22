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

#include "BackendC.h"

#include <memory>
#include "CAPI.h"
#include "CFileAppender.h"
#include "FileAppender.h"
#include "StdAppender.h"


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
    } catch (std::bad_alloc const &) {
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
    backend->inner->addAppender(std::make_shared<LogHard::StdAppender>());
    return true;
    LOGHARD_EXCEPTIONS_TO_C_END(LogHardBackend,backend, false)
}

bool LogHardBackend_addCFileAppender(LogHardBackend * backend, FILE * file) {
    assert(backend);
    LOGHARD_EXCEPTIONS_TO_C_BEGIN
    backend->inner->addAppender(std::make_shared<LogHard::CFileAppender>(file));
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
    backend->inner->addAppender(
                std::make_shared<LogHard::FileAppender>(
                    filename,
                    append
                    ? LogHard::FileAppender::APPEND
                    : LogHard::FileAppender::OVERWRITE));
    return true;
    LOGHARD_EXCEPTIONS_TO_C_END(LogHardBackend,backend, false)
}

void LogHardBackend_setPriority(LogHardBackend * backend,
                                const LogHardPriority priority)
{
    assert(backend);
    LOGHARD_NOEXCEPT_BEGIN
    LogHard::Priority p = LogHard::Priority::Normal;
    switch (priority) {
        case LOGHARD_PRIORITY_FATAL: p = LogHard::Priority::Fatal; break;
        case LOGHARD_PRIORITY_ERROR: p = LogHard::Priority::Error; break;
        case LOGHARD_PRIORITY_WARNING: p = LogHard::Priority::Warning; break;
        case LOGHARD_PRIORITY_NORMAL: p = LogHard::Priority::Normal; break;
        case LOGHARD_PRIORITY_DEBUG: p = LogHard::Priority::Debug; break;
        case LOGHARD_PRIORITY_FULLDEBUG: p = LogHard::Priority::FullDebug; break;
    }
    backend->inner->setPriority(p);
    LOGHARD_NOEXCEPT_END("LHB setPriority")
}

} // extern "C" {
