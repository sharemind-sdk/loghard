/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef LOGHARD_PRIORITY_H
#define LOGHARD_PRIORITY_H

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

/// \todo Use syslog levels
typedef enum LogHardPriority_ {
    LOGHARD_PRIORITY_FATAL = 0,
    LOGHARD_PRIORITY_ERROR = 1,
    LOGHARD_PRIORITY_WARNING = 2,
    LOGHARD_PRIORITY_NORMAL = 3,
    LOGHARD_PRIORITY_DEBUG = 4,
    LOGHARD_PRIORITY_FULLDEBUG = 5
} LogHardPriority;

#ifdef __cplusplus
} /* extern "C" { */

namespace LogHard {
/// \todo Use syslog levels
enum class Priority : unsigned {
    Fatal = LOGHARD_PRIORITY_FATAL,
    Error = LOGHARD_PRIORITY_ERROR,
    Warning = LOGHARD_PRIORITY_WARNING,
    Normal = LOGHARD_PRIORITY_NORMAL,
    Debug = LOGHARD_PRIORITY_DEBUG,
    FullDebug = LOGHARD_PRIORITY_FULLDEBUG
};
}

#endif

#endif /* LOGHARD_PRIORITY_H */
