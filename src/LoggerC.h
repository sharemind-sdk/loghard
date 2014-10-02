/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef LOGHARD_BACKENDC_H
#define LOGHARD_BACKENDC_H

#include <sharemind/extern_c.h>
#include <sharemind/lasterror.h>
#include "ErrorC.h"
#include "PriorityC.h"


SHAREMIND_EXTERN_C_BEGIN

struct LogHardLogger;
struct LogHardBackend;

void LogHardLogger_free(LogHardLogger * logger) __attribute__ ((nonnull(1)));

SHAREMIND_LASTERROR_PUBLIC_FUNCTIONS_DECLARE(LogHardLogger,, LogHardError,)

LogHardBackend * LogHardLogger_backend(const LogHardLogger * logger)
         __attribute__ ((nonnull(1)));

void * LogHardLogger_cxx(LogHardLogger * logger)
         __attribute__ ((nonnull(1)));
const void * LogHardLogger_cxxConst(const LogHardLogger * logger)
         __attribute__ ((nonnull(1)));

LogHardLogger * LogHardLogger_newLogger(LogHardLogger * logger,
                                        const char * prefix)
         __attribute__ ((nonnull(1, 2), warn_unused_result));

void LogHardLogger_log(const LogHardLogger * logger,
                       LogHardPriority priority,
                       const char * message)
         __attribute__ ((nonnull(1, 3)));

SHAREMIND_EXTERN_C_END

#endif /* LOGHARD_BACKENDC_H */
