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

void LogHardLogger_free(LogHardLogger * logger);

SHAREMIND_LASTERROR_PUBLIC_FUNCTIONS_DECLARE(LogHardLogger,, LogHardError,)

LogHardBackend * LogHardLogger_backend(LogHardLogger * logger);

LogHardLogger * LogHardLogger_newLogger(LogHardLogger * logger,
                                        const char * prefix);

void LogHardLogger_log(LogHardLogger * logger,
                       LogHardPriority priority,
                       const char * message);

SHAREMIND_EXTERN_C_END

#endif /* LOGHARD_BACKENDC_H */
