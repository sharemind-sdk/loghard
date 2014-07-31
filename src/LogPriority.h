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

/// \todo Use syslog levels
typedef enum SharemindLogPriority_ {
    SHAREMIND_LOGPRIORITY_FATAL = 0,
    SHAREMIND_LOGPRIORITY_ERROR = 1,
    SHAREMIND_LOGPRIORITY_WARNING = 2,
    SHAREMIND_LOGPRIORITY_NORMAL = 3,
    SHAREMIND_LOGPRIORITY_DEBUG = 4,
    SHAREMIND_LOGPRIORITY_FULLDEBUG = 5
} SharemindLogPriority;

#ifdef __cplusplus
} /* extern "C" { */

namespace sharemind {
/// \todo Use syslog levels
enum class LogPriority {
    Fatal = SHAREMIND_LOGPRIORITY_FATAL,
    Error = SHAREMIND_LOGPRIORITY_ERROR,
    Warning = SHAREMIND_LOGPRIORITY_WARNING,
    Normal = SHAREMIND_LOGPRIORITY_NORMAL,
    Debug = SHAREMIND_LOGPRIORITY_DEBUG,
    FullDebug = SHAREMIND_LOGPRIORITY_FULLDEBUG
};
}

#endif

#endif /* SHAREMINDCOMMON_LOGPRIORITY_H */
