/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SHAREMINDCOMMON_ILOGGER_H
#define SHAREMINDCOMMON_ILOGGER_H

#ifdef __cplusplus
#include <string>

extern "C" {
#endif /* #ifdef __cplusplus */

typedef enum SharemindLogPriority_ {
    LOGPRIORITY_FATAL,
    LOGPRIORITY_ERROR,
    LOGPRIORITY_WARNING,
    LOGPRIORITY_NORMAL,
    LOGPRIORITY_DEBUG,
    LOGPRIORITY_FULLDEBUG
} SharemindLogPriority;

#ifdef __cplusplus
} /* extern "C" { */

namespace sharemind {

class SmartStringStream;

typedef SharemindLogPriority LogPriority;

class ILogger {

public: /* Methods: */

    /**
     Logs a message with the specified priority.

     \param[in] priority the priority level.
     \param[in] message the message to log.
    */
    virtual void logMessage(LogPriority priority, const char * message) = 0;

    /**
     Logs a message with the specified priority.

     \param[in] priority the priority level.
     \param[in] message the message to log.
    */
    virtual void logMessage(LogPriority priority, const std::string & message) = 0;

    /**
     Logs a message with the specified priority.

     \param[in] priority the priority level.
     \param[in] message the message to log.
    */
    virtual void logMessage(LogPriority priority, const SmartStringStream & message) = 0;

}; /* class ILogger { */

} /* namespace sharemind { */

#endif /* #ifdef __cplusplus */

#endif /* SHAREMINDCOMMON_ILOGGER_H */
