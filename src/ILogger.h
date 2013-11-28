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
#include <boost/mpl/if.hpp>
#include <cassert>
#include <string>
#include "../SmartStringStream.h"

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

#if defined(SHAREMIND_LOGLEVEL_FULLDEBUG)
#define SHAREMIND_LOGLEVEL_MAXDEBUG LOGPRIORITY_FULLDEBUG
#elif defined(SHAREMIND_LOGLEVEL_DEBUG)
#define SHAREMIND_LOGLEVEL_MAXDEBUG LOGPRIORITY_DEBUG
#elif defined(SHAREMIND_LOGLEVEL_NORMAL)
#define SHAREMIND_LOGLEVEL_MAXDEBUG LOGPRIORITY_NORMAL
#elif defined(SHAREMIND_LOGLEVEL_WARNING)
#define SHAREMIND_LOGLEVEL_MAXDEBUG LOGPRIORITY_WARNING
#else
#define SHAREMIND_LOGLEVEL_MAXDEBUG LOGPRIORITY_ERROR
#endif

#ifdef __cplusplus
} /* extern "C" { */

namespace sharemind {

class SmartStringStream;

typedef SharemindLogPriority LogPriority;

class ILogger {

public: /* Types: */

    struct LogNoPrefixType {};

    struct NullLogHelper {
        NullLogHelper(ILogger &) {}
        template <class PrefixType>
        NullLogHelper(ILogger &, const PrefixType &) {}
        template <class T> NullLogHelper & operator<<(const T &) { return *this; }
    };

    template <class PrefixType, LogPriority priority = LOGPRIORITY_DEBUG>
    class PrefixedLogHelperBase {

    public: /* Methods: */

        inline PrefixedLogHelperBase(ILogger & logger, const PrefixType & prefix)
            : m_logger(logger)
        {
            m_stream << prefix;
        }

        inline ~PrefixedLogHelperBase() {
            m_logger.logMessage(priority, m_stream);
        }

        template <class T>
        inline PrefixedLogHelperBase & operator<<(const T & v) {
            m_stream << v;
            return *this;
        }

    private: /* Methods: */

        PrefixedLogHelperBase(const PrefixedLogHelperBase<PrefixType, priority> & copy); // disable
        PrefixedLogHelperBase & operator=(const PrefixedLogHelperBase<PrefixType, priority> & rhs); // disable

    private: /* Fields: */

        SmartStringStream m_stream;
        ILogger & m_logger;

    };

    template <LogPriority priority>
    class NoPrefixLogHelperBase {

    public: /* Methods: */

        inline NoPrefixLogHelperBase(ILogger & logger)
            : m_logger(logger)
            , m_string(NULL)
            , m_stream(NULL) {}

        inline ~NoPrefixLogHelperBase() {
            assert(!(m_string && m_stream));
            if (m_stream) {
                m_logger.logMessage(priority, *m_stream);
                delete m_stream;
            } else if (m_string) {
                m_logger.logMessage(priority, m_string);
            }
        }

        inline NoPrefixLogHelperBase & operator<<(const char * v) {
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
        inline NoPrefixLogHelperBase & operator<<(const T & v) {
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

        NoPrefixLogHelperBase(const NoPrefixLogHelperBase<priority> & copy); // disable
        NoPrefixLogHelperBase & operator=(const NoPrefixLogHelperBase<priority> & rhs); // disable

    private: /* Fields: */

        ILogger & m_logger;
        const char * m_string;
        SmartStringStream * m_stream;

    };

    template <LogPriority priority = LOGPRIORITY_DEBUG, class PrefixType = LogNoPrefixType>
    class LogHelper
            : public boost::mpl::if_c<priority <= SHAREMIND_LOGLEVEL_MAXDEBUG,
                                      PrefixedLogHelperBase<PrefixType, priority>,
                                      NullLogHelper>::type
    {

    public: /* Methods: */

        inline LogHelper(ILogger & logger, const PrefixType & prefix)
            : boost::mpl::if_c<priority <= SHAREMIND_LOGLEVEL_MAXDEBUG,
                               PrefixedLogHelperBase<PrefixType, priority>,
                               NullLogHelper>::type(logger, prefix) {}

    };

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


template <LogPriority priority>
class ILogger::LogHelper<priority, ILogger::LogNoPrefixType>
        : public boost::mpl::if_c<priority <= SHAREMIND_LOGLEVEL_MAXDEBUG,
                                  ILogger::NoPrefixLogHelperBase<priority>,
                                  NullLogHelper>::type
{

public: /* Methods: */

    inline LogHelper(ILogger & logger)
        : boost::mpl::if_c<priority <= SHAREMIND_LOGLEVEL_MAXDEBUG,
                           ILogger::NoPrefixLogHelperBase<priority>,
                           NullLogHelper>::type(logger) {}

};

} /* namespace sharemind { */

#endif /* #ifdef __cplusplus */

#endif /* SHAREMINDCOMMON_ILOGGER_H */
