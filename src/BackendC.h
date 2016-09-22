/*
 * Copyright (C) 2015 Cybernetica
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

#ifndef LOGHARD_BACKENDC_H
#define LOGHARD_BACKENDC_H

#include <sharemind/DebugOnly.h>
#include <sharemind/extern_c.h>
#include <sharemind/lasterror.h>
#include <stdbool.h>
#include <stdio.h>
#include "ErrorC.h"


SHAREMIND_EXTERN_C_BEGIN

struct LogHardLogger;
struct LogHardBackend;

LogHardBackend * LogHardBackend_new(LogHardError * error,
                                    const char ** errorStr)
        __attribute__ ((warn_unused_result));

void LogHardBackend_free(LogHardBackend * backend) __attribute__ ((nonnull(1)));


SHAREMIND_LASTERROR_PUBLIC_FUNCTIONS_DECLARE(LogHardBackend,, LogHardError,)


void * LogHardBackend_cxx(LogHardBackend * backend)
         SHAREMIND_NDEBUG_ONLY(__attribute__ ((nonnull(1))));
const void * LogHardBackend_cxxConst(const LogHardBackend * backend)
         SHAREMIND_NDEBUG_ONLY(__attribute__ ((nonnull(1))));

bool LogHardBackend_addStdAppender(LogHardBackend * backend)
         SHAREMIND_NDEBUG_ONLY(__attribute__ ((nonnull(1))));

bool LogHardBackend_addCFileAppender(LogHardBackend * backend,
                                     FILE * file)
         SHAREMIND_NDEBUG_ONLY(__attribute__ ((nonnull(1))));

bool LogHardBackend_addFileAppender(LogHardBackend * backend,
                                    const char * filename,
                                    bool append)
         SHAREMIND_NDEBUG_ONLY(__attribute__ ((nonnull(1, 2))));

LogHardLogger * LogHardBackend_newLogger(LogHardBackend * backend,
                                         const char * prefix)
         __attribute__ ((SHAREMIND_NDEBUG_ONLY(nonnull(1, 2),)
                         warn_unused_result));

SHAREMIND_EXTERN_C_END

#endif /* LOGHARD_BACKENDC_H */
