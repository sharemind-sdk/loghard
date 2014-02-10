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
#include <algorithm>
#include <boost/mpl/if.hpp>
#include <cassert>
#include <sstream>
#include <string>

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
#elif defined(SHAREMIND_LOGLEVEL_ERROR)
#define SHAREMIND_LOGLEVEL_MAXDEBUG LOGPRIORITY_ERROR
#else
#define SHAREMIND_LOGLEVEL_MAXDEBUG LOGPRIORITY_DEBUG
#endif

#ifdef __cplusplus
} /* extern "C" { */

namespace sharemind {

class SmartStringStream;

typedef SharemindLogPriority LogPriority;

class ILogger {

private: /* Types: */

    struct LogNoPrefixType {};

    struct NullLogHelper {
        inline NullLogHelper(ILogger &) {}
        template <class PrefixType>
        inline NullLogHelper(ILogger &, const PrefixType &) {}
        template <class T> NullLogHelper & operator<<(const T &) { return *this; }
        inline void setAsPrefix() const {}
    };

    template <LogPriority priority>
    class NoPrefixLogHelperBase {

    private: /* Types: */

        enum Status { NO_LOG, OPERATIONAL, NOT_OPERATIONAL };

    public: /* Methods: */

        inline NoPrefixLogHelperBase(ILogger & logger)
            : m_logger(&logger)
            , m_status(NO_LOG) {}

        NoPrefixLogHelperBase(const NoPrefixLogHelperBase<priority> &) = delete;
        NoPrefixLogHelperBase<priority> & operator=(const NoPrefixLogHelperBase<priority> &) = delete;

        inline NoPrefixLogHelperBase(NoPrefixLogHelperBase<priority> && move)
            : /* m_stream(std::move(move.m_stream))
            , */ m_logger(std::move(move.m_logger))
            , m_status(std::move(move.m_status))
        {
            /// \bug http://gcc.gnu.org/bugzilla/show_bug.cgi?id=54316
            m_stream << move.m_stream.str();
            move.m_stream.str("");
            move.m_stream.clear();

            move.m_status = NOT_OPERATIONAL;
        }

        inline NoPrefixLogHelperBase<priority> & operator=(NoPrefixLogHelperBase<priority> && move) {
            /// \bug http://gcc.gnu.org/bugzilla/show_bug.cgi?id=54316
            // m_stream = std::move(move.m_stream);
            m_stream << move.m_stream.str();
            move.m_stream.str("");
            move.m_stream.clear();

            m_logger = std::move(move.m_logger);
            m_status = std::move(move.m_status);
            move.m_status = NOT_OPERATIONAL;
        }

        inline ~NoPrefixLogHelperBase() {
            if (m_status == OPERATIONAL)
                m_logger->logMessage(priority, m_stream.str());
        }

        template <class T>
        inline NoPrefixLogHelperBase & operator<<(const T & v) {
            if (m_status != NOT_OPERATIONAL) {
                m_stream << v;
                if (m_status == NO_LOG)
                    m_status = OPERATIONAL;
            }
            return *this;
        }

        void setAsPrefix() {
            if (m_status == OPERATIONAL)
                m_status = NO_LOG;
        }

    private: /* Fields: */

        std::ostringstream m_stream;
        ILogger * m_logger;
        Status m_status;

    }; /* class NoPrefixLogHelperBase { */

    template <class PrefixType, LogPriority priority = LOGPRIORITY_DEBUG>
    class PrefixedLogHelperBase: public NoPrefixLogHelperBase<priority> {

    public: /* Methods: */

        inline PrefixedLogHelperBase(ILogger & logger, const PrefixType & prefix)
            : NoPrefixLogHelperBase<priority>(logger)
        {
            (*this) << prefix;
            this->setAsPrefix();
        }

    };

public: /* Types: */

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
    template <LogPriority priority, class PrefixType> friend class LogHelper;

    template <class ILogger__ = ILogger>
    class PrefixedWrapper {

    private: /* Types: */

        template <LogPriority priority>
        struct Helper {
            typedef typename boost::mpl::if_c<priority <= SHAREMIND_LOGLEVEL_MAXDEBUG,
                                              ILogger::NoPrefixLogHelperBase<priority>,
                                              NullLogHelper>::type type;
        };

    public: /* Types: */

        typedef PrefixedWrapper<PrefixedWrapper<ILogger__> > Wrapped;

    public: /* Methods: */

        PrefixedWrapper(ILogger__ & logger, const std::string & prefix)
            : m_logger(logger)
            , m_prefix(prefix) {}

        inline typename Helper<LOGPRIORITY_FATAL>::type fatal();
        inline typename Helper<LOGPRIORITY_ERROR>::type error();
        inline typename Helper<LOGPRIORITY_WARNING>::type warning();
        inline typename Helper<LOGPRIORITY_NORMAL>::type info();
        inline typename Helper<LOGPRIORITY_DEBUG>::type debug();
        inline typename Helper<LOGPRIORITY_FULLDEBUG>::type fullDebug();

        inline Wrapped wrap(const std::string & prefix) {
            return Wrapped(*this, prefix);
        }

    private: /* Fields: */

        ILogger__ & m_logger;
        const std::string m_prefix;

    };
    template <class ILogger__> friend class PrefixedWrapper;

    typedef PrefixedWrapper<ILogger> Wrapped;

public: /* Methods: */

    inline LogHelper<LOGPRIORITY_FATAL> fatal();
    inline LogHelper<LOGPRIORITY_ERROR> error();
    inline LogHelper<LOGPRIORITY_WARNING> warning();
    inline LogHelper<LOGPRIORITY_NORMAL> info();
    inline LogHelper<LOGPRIORITY_DEBUG> debug();
    inline LogHelper<LOGPRIORITY_FULLDEBUG> fullDebug();

    inline Wrapped wrap(const std::string & prefix) {
        return Wrapped(*this, prefix);
    }

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

inline ILogger::LogHelper<LOGPRIORITY_FATAL> ILogger::fatal() {
    return LogHelper<LOGPRIORITY_FATAL>(*this);
}

inline ILogger::LogHelper<LOGPRIORITY_ERROR> ILogger::error() {
    return LogHelper<LOGPRIORITY_ERROR>(*this);
}

inline ILogger::LogHelper<LOGPRIORITY_WARNING> ILogger::warning() {
    return LogHelper<LOGPRIORITY_WARNING>(*this);
}

inline ILogger::LogHelper<LOGPRIORITY_NORMAL> ILogger::info() {
    return LogHelper<LOGPRIORITY_NORMAL>(*this);
}

inline ILogger::LogHelper<LOGPRIORITY_DEBUG> ILogger::debug() {
    return LogHelper<LOGPRIORITY_DEBUG>(*this);
}

inline ILogger::LogHelper<LOGPRIORITY_FULLDEBUG> ILogger::fullDebug() {
    return LogHelper<LOGPRIORITY_FULLDEBUG>(*this);
}

template <class ILogger__>
inline typename ILogger::PrefixedWrapper<ILogger__>::template Helper<LOGPRIORITY_FATAL>::type ILogger::PrefixedWrapper<ILogger__>::fatal() {
    typename Helper<LOGPRIORITY_FATAL>::type logger(m_logger.fatal());
    logger << m_prefix;
    logger.setAsPrefix();
    return logger;
}

template <class ILogger__>
inline typename ILogger::PrefixedWrapper<ILogger__>::template Helper<LOGPRIORITY_ERROR>::type ILogger::PrefixedWrapper<ILogger__>::error() {
    typename Helper<LOGPRIORITY_ERROR>::type logger(m_logger.error());
    logger << m_prefix;
    logger.setAsPrefix();
    return logger;
}

template <class ILogger__>
inline typename ILogger::PrefixedWrapper<ILogger__>::template Helper<LOGPRIORITY_WARNING>::type ILogger::PrefixedWrapper<ILogger__>::warning() {
    typename Helper<LOGPRIORITY_WARNING>::type logger(m_logger.warning());
    logger << m_prefix;
    logger.setAsPrefix();
    return logger;
}

template <class ILogger__>
inline typename ILogger::PrefixedWrapper<ILogger__>::template Helper<LOGPRIORITY_NORMAL>::type ILogger::PrefixedWrapper<ILogger__>::info() {
    typename Helper<LOGPRIORITY_NORMAL>::type logger(m_logger.info());
    logger << m_prefix;
    logger.setAsPrefix();
    return logger;
}

template <class ILogger__>
inline typename ILogger::PrefixedWrapper<ILogger__>::template Helper<LOGPRIORITY_DEBUG>::type ILogger::PrefixedWrapper<ILogger__>::debug() {
    typename Helper<LOGPRIORITY_DEBUG>::type logger(m_logger.debug());
    logger << m_prefix;
    logger.setAsPrefix();
    return logger;
}

template <class ILogger__>
inline typename ILogger::PrefixedWrapper<ILogger__>::template Helper<LOGPRIORITY_FULLDEBUG>::type ILogger::PrefixedWrapper<ILogger__>::fullDebug() {
    typename Helper<LOGPRIORITY_FULLDEBUG>::type logger(m_logger.fullDebug());
    logger << m_prefix;
    logger.setAsPrefix();
    return logger;
}

} /* namespace sharemind { */

#endif /* #ifdef __cplusplus */

#endif /* SHAREMINDCOMMON_ILOGGER_H */
