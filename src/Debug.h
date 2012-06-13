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

#include <cassert>
#include <string>
#include "../SmartStringStream.h"
#include "ILogger.h"


namespace sharemind {

struct LogNoPrefixType {};

struct NullLogHelper {
    NullLogHelper(ILogger &) {}
    template <class PrefixType>
    NullLogHelper(ILogger &, const PrefixType &) {}
    template <class T> NullLogHelper & operator<<(const T &) { return *this; }
};

template <LogPriority priority = LOGPRIORITY_DEBUG, class PrefixType = LogNoPrefixType>
class LogHelper {

public: /* Methods: */

    inline LogHelper(ILogger & logger, const PrefixType & prefix)
        : m_logger(logger)
    {
        m_stream << prefix;
    }

    inline ~LogHelper() {
        m_logger.logMessage(priority, m_stream);
    }

    template <class T>
    inline LogHelper & operator<<(const T & v) {
        m_stream << v;
        return *this;
    }

private: /* Methods: */

    LogHelper(const LogHelper<priority, PrefixType> & copy); // disable
    void operator=(const LogHelper<priority, PrefixType> & rhs); // disable

private: /* Fields: */

    SmartStringStream m_stream;
    ILogger & m_logger;

};

template <LogPriority priority>
class LogHelper<priority, LogNoPrefixType> {

public: /* Methods: */

    inline LogHelper(ILogger & logger)
        : m_logger(logger)
        , m_string(NULL)
        , m_stream(NULL) {}

    inline ~LogHelper() {
        assert(!(m_string && m_stream));
        if (m_stream) {
            m_logger.logMessage(priority, *m_stream);
            delete m_stream;
        } else if (m_string) {
            m_logger.logMessage(priority, m_string);
        }
    }

    inline LogHelper & operator<<(const char * v) {
        assert(!(m_string && m_stream));
        if (m_stream) {
            (*m_stream) << v;
        } else if (m_string) {
            m_stream = new SmartStringStream;
            (*m_stream) << m_string << v;
            m_string = NULL;
        } else {
            m_string = v;
        }
        return *this;
    }

    template <class T>
    inline LogHelper & operator<<(const T & v) {
        assert(!(m_string && m_stream));
        if (m_stream) {
            (*m_stream) << v;
        } else {
            m_stream = new SmartStringStream;
            if (m_string) {
                (*m_stream) << m_string << v;
                m_string = NULL;
            } else {
                (*m_stream) << v;
            }
        }
        return *this;
    }

private: /* Methods: */

    LogHelper(const LogHelper<priority, LogNoPrefixType> & copy); // disable
    void operator=(const LogHelper<priority, LogNoPrefixType> & rhs); // disable

private: /* Fields: */

    ILogger & m_logger;
    const char * m_string;
    SmartStringStream * m_stream;

};

#define SHAREMIND_DEFINE_LOG(name,priority) \
    typedef sharemind::LogHelper<priority> name

#define SHAREMIND_DEFINE_PREFIXED_LOG(name,priority,constPrefix) \
    class name: public sharemind::LogHelper<priority, const char *> { \
        public: /* Methods: */ \
            inline name (sharemind::ILogger & logger, const char * prefix = (constPrefix)) \
                : sharemind::LogHelper<priority, const char *>(logger, prefix) {} \
            inline name (sharemind::ILogger & logger, const std::string & prefix) \
                : sharemind::LogHelper<priority, const char *>(logger, prefix.c_str()) {} \
    }

#define SHAREMIND_UNDEFINE_LOG(name) \
    typedef sharemind::NullLogHelper name

#define SHAREMIND_UNDEFINE_PREFIXED_LOG(name) \
    typedef sharemind::NullLogHelper name

#define SHAREMIND_DEFINE_LOGS__ \
    SHAREMIND_DEFINE_LOG(LogError,LOGPRIORITY_ERROR); \
    SHAREMIND_DEFINE_LOG(LogFatal,LOGPRIORITY_FATAL)

#define SHAREMIND_DEFINE_PREFIXED_LOGS__(constPrefix) \
    SHAREMIND_DEFINE_PREFIXED_LOG(LogError,LOGPRIORITY_ERROR,(constPrefix)); \
    SHAREMIND_DEFINE_PREFIXED_LOG(LogFatal,LOGPRIORITY_FATAL,(constPrefix))


#if defined(SHAREMIND_LOGLEVEL_FULLDEBUG)
    #define SHAREMIND_DEFINE_LOGS() \
        SHAREMIND_DEFINE_LOG(LogFullDebug,LOGPRIORITY_FULLDEBUG); \
        SHAREMIND_DEFINE_LOG(LogDebug,LOGPRIORITY_DEBUG); \
        SHAREMIND_DEFINE_LOG(LogNormal,LOGPRIORITY_NORMAL); \
        SHAREMIND_DEFINE_LOG(LogWarning,LOGPRIORITY_WARNING); \
        SHAREMIND_DEFINE_LOGS__
    #define SHAREMIND_DEFINE_PREFIXED_LOGS(constPrefix) \
        SHAREMIND_DEFINE_PREFIXED_LOG(LogFullDebug,LOGPRIORITY_FULLDEBUG,(constPrefix)); \
        SHAREMIND_DEFINE_PREFIXED_LOG(LogDebug,LOGPRIORITY_DEBUG,(constPrefix)); \
        SHAREMIND_DEFINE_PREFIXED_LOG(LogNormal,LOGPRIORITY_NORMAL,(constPrefix)); \
        SHAREMIND_DEFINE_PREFIXED_LOG(LogWarning,LOGPRIORITY_WARNING,(constPrefix)); \
        SHAREMIND_DEFINE_PREFIXED_LOGS__(constPrefix)
#elif defined(SHAREMIND_LOGLEVEL_DEBUG)
    #define SHAREMIND_DEFINE_LOGS() \
        SHAREMIND_UNDEFINE_LOG(LogFullDebug); \
        SHAREMIND_DEFINE_LOG(LogDebug,LOGPRIORITY_DEBUG); \
        SHAREMIND_DEFINE_LOG(LogNormal,LOGPRIORITY_NORMAL); \
        SHAREMIND_DEFINE_LOG(LogWarning,LOGPRIORITY_WARNING); \
        SHAREMIND_DEFINE_LOGS__
    #define SHAREMIND_DEFINE_PREFIXED_LOGS(constPrefix) \
        SHAREMIND_UNDEFINE_PREFIXED_LOG(LogFullDebug); \
        SHAREMIND_DEFINE_PREFIXED_LOG(LogDebug,LOGPRIORITY_DEBUG,(constPrefix)); \
        SHAREMIND_DEFINE_PREFIXED_LOG(LogNormal,LOGPRIORITY_NORMAL,(constPrefix)); \
        SHAREMIND_DEFINE_PREFIXED_LOG(LogWarning,LOGPRIORITY_WARNING,(constPrefix)); \
        SHAREMIND_DEFINE_PREFIXED_LOGS__(constPrefix)
#elif defined(SHAREMIND_LOGLEVEL_NORMAL)
    #define SHAREMIND_DEFINE_LOGS() \
        SHAREMIND_UNDEFINE_LOG(LogFullDebug); \
        SHAREMIND_UNDEFINE_LOG(LogDebug); \
        SHAREMIND_DEFINE_LOG(LogNormal,LOGPRIORITY_NORMAL); \
        SHAREMIND_DEFINE_LOG(LogWarning,LOGPRIORITY_WARNING); \
        SHAREMIND_DEFINE_LOGS__
    #define SHAREMIND_DEFINE_PREFIXED_LOGS(constPrefix) \
        SHAREMIND_UNDEFINE_PREFIXED_LOG(LogFullDebug); \
        SHAREMIND_UNDEFINE_PREFIXED_LOG(LogDebug); \
        SHAREMIND_DEFINE_PREFIXED_LOG(LogNormal,LOGPRIORITY_NORMAL,(constPrefix)); \
        SHAREMIND_DEFINE_PREFIXED_LOG(LogWarning,LOGPRIORITY_WARNING,(constPrefix)); \
        SHAREMIND_DEFINE_PREFIXED_LOGS__(constPrefix)
#elif defined(SHAREMIND_LOGLEVEL_WARNING)
    #define SHAREMIND_DEFINE_LOGS() \
        SHAREMIND_UNDEFINE_LOG(LogFullDebug); \
        SHAREMIND_UNDEFINE_LOG(LogDebug); \
        SHAREMIND_UNDEFINE_LOG(LogNormal); \
        SHAREMIND_DEFINE_LOG(LogWarning,LOGPRIORITY_WARNING); \
        SHAREMIND_DEFINE_LOGS__
    #define SHAREMIND_DEFINE_PREFIXED_LOGS(constPrefix) \
        SHAREMIND_UNDEFINE_PREFIXED_LOG(LogFullDebug); \
        SHAREMIND_UNDEFINE_PREFIXED_LOG(LogDebug); \
        SHAREMIND_UNDEFINE_PREFIXED_LOG(LogNormal); \
        SHAREMIND_DEFINE_PREFIXED_LOG(LogWarning,LOGPRIORITY_WARNING,(constPrefix)); \
        SHAREMIND_DEFINE_PREFIXED_LOGS__(constPrefix)
#else
    #define SHAREMIND_DEFINE_LOGS() \
        SHAREMIND_UNDEFINE_LOG(LogFullDebug); \
        SHAREMIND_UNDEFINE_LOG(LogDebug); \
        SHAREMIND_UNDEFINE_LOG(LogNormal); \
        SHAREMIND_UNDEFINE_LOG(LogWarning); \
        SHAREMIND_DEFINE_LOGS__
    #define SHAREMIND_DEFINE_PREFIXED_LOGS(constPrefix) \
        SHAREMIND_UNDEFINE_PREFIXED_LOG(LogFullDebug); \
        SHAREMIND_UNDEFINE_PREFIXED_LOG(LogDebug); \
        SHAREMIND_UNDEFINE_PREFIXED_LOG(LogNormal); \
        SHAREMIND_UNDEFINE_PREFIXED_LOG(LogWarning); \
        SHAREMIND_DEFINE_PREFIXED_LOGS__(constPrefix)
#endif

} /* namespace sharemind { */

#endif /* SHAREMINDCOMMON_DEBUG_H */
