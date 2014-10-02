/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef LOGHARD_PRIORITYC_H
#define LOGHARD_PRIORITYC_H

#include <sharemind/extern_c.h>


SHAREMIND_EXTERN_C_BEGIN

/// \todo Use syslog levels
typedef enum LogHardPriority_ {
    LOGHARD_PRIORITY_FATAL = 0,
    LOGHARD_PRIORITY_ERROR = 1,
    LOGHARD_PRIORITY_WARNING = 2,
    LOGHARD_PRIORITY_NORMAL = 3,
    LOGHARD_PRIORITY_DEBUG = 4,
    LOGHARD_PRIORITY_FULLDEBUG = 5
} LogHardPriority;

SHAREMIND_EXTERN_C_END

#endif /* LOGHARD_PRIORITYC_H */
