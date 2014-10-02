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
#include <stdbool.h>
#include <stdio.h>
#include "ErrorC.h"


SHAREMIND_EXTERN_C_BEGIN

struct LogHardLogger;
struct LogHardBackend;

LogHardBackend * LogHardBackend_new(LogHardError * error,
                                    const char ** errorStr);

void LogHardBackend_free(LogHardBackend * backend);


SHAREMIND_LASTERROR_PUBLIC_FUNCTIONS_DECLARE(LogHardBackend,, LogHardError,)


void * LogHardBackend_cxx(LogHardBackend * backend);
const void * LogHardBackend_cxxConst(const LogHardBackend * backend);

bool LogHardBackend_addStdAppender(LogHardBackend * backend);

bool LogHardBackend_addCFileAppender(LogHardBackend * backend,
                                     FILE * file);

bool LogHardBackend_addFileAppender(LogHardBackend * backend,
                                    const char * filename,
                                    bool append);

LogHardLogger * LogHardBackend_newLogger(LogHardBackend * backend,
                                         const char * prefix);

SHAREMIND_EXTERN_C_END

#endif /* LOGHARD_BACKENDC_H */
