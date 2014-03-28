/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SHAREMINDCOMMON_DEBUG_H
#define SHAREMINDCOMMON_DEBUG_H

#include <string>
#include "ILogger.h"


namespace sharemind {

#define SHAREMIND_DEFINE_LOG(name,priority) \
    typedef sharemind::ILogger::ILogger::LogHelper<priority> name

#define SHAREMIND_DEFINE_LOGS() \
    SHAREMIND_DEFINE_LOG(LogFullDebug,LOGPRIORITY_FULLDEBUG); \
    SHAREMIND_DEFINE_LOG(LogDebug,LOGPRIORITY_DEBUG); \
    SHAREMIND_DEFINE_LOG(LogNormal,LOGPRIORITY_NORMAL); \
    SHAREMIND_DEFINE_LOG(LogWarning,LOGPRIORITY_WARNING); \
    SHAREMIND_DEFINE_LOG(LogError,LOGPRIORITY_ERROR); \
    SHAREMIND_DEFINE_LOG(LogFatal,LOGPRIORITY_FATAL)

#define SHAREMIND_DEFINE_PREFIXED_LOG(name,priority,constPrefix) \
    class name: public sharemind::ILogger::LogHelper<priority> { \
        public: /* Methods: */ \
            inline name (sharemind::ILogger & logger, const char * const prefix = (constPrefix)) \
                : sharemind::ILogger::LogHelper<priority>(logger, prefix) {} \
            inline name (sharemind::ILogger & logger, const std::string & prefix) \
                : sharemind::ILogger::LogHelper<priority>(logger, prefix) {} \
    }

#define SHAREMIND_DEFINE_PREFIXED_LOGS(constPrefix) \
    SHAREMIND_DEFINE_PREFIXED_LOG(LogFullDebug,LOGPRIORITY_FULLDEBUG,(constPrefix)); \
    SHAREMIND_DEFINE_PREFIXED_LOG(LogDebug,LOGPRIORITY_DEBUG,(constPrefix)); \
    SHAREMIND_DEFINE_PREFIXED_LOG(LogNormal,LOGPRIORITY_NORMAL,(constPrefix)); \
    SHAREMIND_DEFINE_PREFIXED_LOG(LogWarning,LOGPRIORITY_WARNING,(constPrefix)); \
    SHAREMIND_DEFINE_PREFIXED_LOG(LogError,LOGPRIORITY_ERROR,(constPrefix)); \
    SHAREMIND_DEFINE_PREFIXED_LOG(LogFatal,LOGPRIORITY_FATAL,(constPrefix))

} /* namespace sharemind { */

#endif /* SHAREMINDCOMMON_DEBUG_H */
