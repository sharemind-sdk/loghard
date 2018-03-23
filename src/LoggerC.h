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

#ifndef LOGHARD_LOGGERC_H
#define LOGHARD_LOGGERC_H

#include <sharemind/DebugOnly.h>
#include <sharemind/extern_c.h>
#include <sharemind/lasterror.h>
#include "ErrorC.h"
#include "PriorityC.h"


SHAREMIND_EXTERN_C_BEGIN

struct LogHardLogger;
struct LogHardBackend;

void LogHardLogger_free(LogHardLogger * logger)
        SHAREMIND_NDEBUG_ONLY(__attribute__ ((nonnull(1))));

SHAREMIND_LASTERROR_PUBLIC_FUNCTIONS_DECLARE(LogHardLogger,, LogHardError,)

LogHardBackend * LogHardLogger_backend(const LogHardLogger * logger)
         SHAREMIND_NDEBUG_ONLY(__attribute__ ((nonnull(1))));

void * LogHardLogger_cxx(LogHardLogger * logger)
         SHAREMIND_NDEBUG_ONLY(__attribute__ ((nonnull(1))));
const void * LogHardLogger_cxxConst(const LogHardLogger * logger)
         SHAREMIND_NDEBUG_ONLY(__attribute__ ((nonnull(1))));

LogHardLogger * LogHardLogger_newLogger(LogHardLogger * logger,
                                        const char * prefix)
         __attribute__ ((SHAREMIND_NDEBUG_ONLY(nonnull(1, 2),)
                         warn_unused_result));

void LogHardLogger_log(const LogHardLogger * logger,
                       LogHardPriority priority,
                       const char * message)
         SHAREMIND_NDEBUG_ONLY(__attribute__ ((nonnull(1, 3))));

SHAREMIND_EXTERN_C_END

#endif /* LOGHARD_LOGGERC_H */
