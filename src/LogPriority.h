/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SHAREMINDCOMMON_LOGPRIORITY_H
#define SHAREMINDCOMMON_LOGPRIORITY_H

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

typedef enum SharemindLogPriority_ {
    LOGPRIORITY_FATAL = 0,
    LOGPRIORITY_ERROR = 1,
    LOGPRIORITY_WARNING = 2,
    LOGPRIORITY_NORMAL = 3,
    LOGPRIORITY_DEBUG = 4,
    LOGPRIORITY_FULLDEBUG = 5
} SharemindLogPriority;

#ifdef __cplusplus
} /* extern "C" { */
namespace sharemind { typedef SharemindLogPriority LogPriority; }
#endif

#endif /* SHAREMINDCOMMON_LOGPRIORITY_H */
