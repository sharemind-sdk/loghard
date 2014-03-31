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

#include <algorithm>
#include <boost/mpl/if.hpp>
#include <cassert>
#include <sstream>
#include <string>
#include <utility>
#include "LogPriority.h"


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


namespace sharemind {

class SmartStringStream;

typedef SharemindLogPriority LogPriority;

class ILogger {

private: /* Types: */

    struct NullLogHelper {
        template <typename ... Args>
        inline NullLogHelper(ILogger &, Args && ...) noexcept {}
        template <class T> NullLogHelper & operator<<(const T &) noexcept
        { return *this; }
        inline void setAsPrefix() const noexcept {}
    };

    template <LogPriority priority>
    class NoPrefixLogHelperBase {

    private: /* Types: */

        enum Status { NO_LOG, OPERATIONAL, NOT_OPERATIONAL };

    public: /* Methods: */

        inline NoPrefixLogHelperBase(ILogger & logger) noexcept
            : m_logger(&logger)
            , m_status(NO_LOG) {}

        NoPrefixLogHelperBase(const NoPrefixLogHelperBase<priority> &) = delete;
        NoPrefixLogHelperBase<priority> & operator=(
                const NoPrefixLogHelperBase<priority> &) = delete;

        inline NoPrefixLogHelperBase(NoPrefixLogHelperBase<priority> && move)
                noexcept
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

        inline NoPrefixLogHelperBase<priority> & operator=(
                NoPrefixLogHelperBase<priority> && move) noexcept
        {
            /// \bug http://gcc.gnu.org/bugzilla/show_bug.cgi?id=54316
            // m_stream = std::move(move.m_stream);
            m_stream << move.m_stream.str();
            move.m_stream.str("");
            move.m_stream.clear();

            m_logger = std::move(move.m_logger);
            m_status = std::move(move.m_status);
            move.m_status = NOT_OPERATIONAL;
        }

        inline ~NoPrefixLogHelperBase() noexcept {
            if (m_status == OPERATIONAL)
                m_logger->logMessage(priority,
                                     m_stream.str()); /// \bug might throw
        }

        template <class T>
        inline NoPrefixLogHelperBase & operator<<(T && v) noexcept {
            if (m_status != NOT_OPERATIONAL) {
                m_stream << std::forward<T>(v); /// \bug might throw
                if (m_status == NO_LOG)
                    m_status = OPERATIONAL;
            }
            return *this;
        }

        void setAsPrefix() noexcept {
            if (m_status == OPERATIONAL)
                m_status = NO_LOG;
        }

    private: /* Fields: */

        std::ostringstream m_stream;
        ILogger * m_logger;
        Status m_status;

    }; /* class NoPrefixLogHelperBase { */

    template <LogPriority priority = LOGPRIORITY_DEBUG>
    class PrefixedLogHelperBase: public NoPrefixLogHelperBase<priority> {

    public: /* Methods: */

        template <typename ... Args>
        inline PrefixedLogHelperBase(ILogger & logger,
                                     Args && ... args) noexcept
            : NoPrefixLogHelperBase<priority>(logger)
        {
            appendPrefix(std::forward<Args>(args)...).setAsPrefix();
        }

    private: /* Methods: */

        inline NoPrefixLogHelperBase<priority> & appendPrefix() noexcept
        { return *this; }

        template <typename Arg>
        inline NoPrefixLogHelperBase<priority> & appendPrefix(Arg && arg)
                noexcept
        { return (*this) << std::forward<Arg>(arg); }

        template <typename Arg, typename ... Args>
        inline NoPrefixLogHelperBase<priority> & appendPrefix(Arg && arg,
                                                              Args && ... args)
                noexcept
        {
            return ((*this) << std::forward<Arg>(arg))
                    .appendPrefix(std::forward<Args>(args)...);
        }

    };

public: /* Types: */

    template <LogPriority priority = LOGPRIORITY_DEBUG>
    class LogHelper
            : public boost::mpl::if_c<priority <= SHAREMIND_LOGLEVEL_MAXDEBUG,
                                      PrefixedLogHelperBase<priority>,
                                      NullLogHelper>::type
    {

    public: /* Methods: */

        template <typename ... Args>
        inline LogHelper(ILogger & logger, Args && ... args) noexcept
            : boost::mpl::if_c<priority <= SHAREMIND_LOGLEVEL_MAXDEBUG,
                               PrefixedLogHelperBase<priority>,
                               NullLogHelper>::type(logger,
                                                    std::forward<Args>(args)...)
        {}

    };
    template <LogPriority priority> friend class LogHelper;

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

        template <typename ... Args>
        PrefixedWrapper(ILogger__ & logger, Args && ... args) noexcept
            : m_logger(logger)
            , m_prefix(std::forward<Args>(args)...) /// \bug might throw
        {}

        inline typename Helper<LOGPRIORITY_FATAL>::type fatal() noexcept;
        inline typename Helper<LOGPRIORITY_ERROR>::type error() noexcept;
        inline typename Helper<LOGPRIORITY_WARNING>::type warning() noexcept;
        inline typename Helper<LOGPRIORITY_NORMAL>::type info() noexcept;
        inline typename Helper<LOGPRIORITY_DEBUG>::type debug() noexcept;
        inline typename Helper<LOGPRIORITY_FULLDEBUG>::type fullDebug() noexcept;

        template <typename PrefixType>
        inline Wrapped wrap(PrefixType && prefix) noexcept {
            return Wrapped(*this, std::forward<PrefixType>(prefix));
        }

    private: /* Fields: */

        ILogger__ & m_logger;
        const std::string m_prefix;

    };
    template <class ILogger__> friend class PrefixedWrapper;

    typedef PrefixedWrapper<ILogger> Wrapped;

public: /* Methods: */

    inline LogHelper<LOGPRIORITY_FATAL> fatal() noexcept;
    inline LogHelper<LOGPRIORITY_ERROR> error() noexcept;
    inline LogHelper<LOGPRIORITY_WARNING> warning() noexcept;
    inline LogHelper<LOGPRIORITY_NORMAL> info() noexcept;
    inline LogHelper<LOGPRIORITY_DEBUG> debug() noexcept;
    inline LogHelper<LOGPRIORITY_FULLDEBUG> fullDebug() noexcept;

    template <typename PrefixType>
    inline Wrapped wrap(PrefixType && prefix) noexcept {
        return Wrapped(*this, std::forward<PrefixType>(prefix));
    }

protected: /* Methods: */

    /**
     Logs a message with the specified priority.

     \param[in] priority the priority level.
     \param[in] message the message to log.
    */
    virtual void logMessage(LogPriority priority, const char * message) noexcept = 0;

    /**
     Logs a message with the specified priority.

     \param[in] priority the priority level.
     \param[in] message the message to log.
    */
    virtual void logMessage(LogPriority priority, const std::string & message) noexcept = 0;

    /**
     Logs a message with the specified priority.

     \param[in] priority the priority level.
     \param[in] message the message to log.
    */
    virtual void logMessage(LogPriority priority, std::string && message) noexcept = 0;

    /**
     Logs a message with the specified priority.

     \param[in] priority the priority level.
     \param[in] message the message to log.
    */
    virtual void logMessage(LogPriority priority, const SmartStringStream & message) noexcept = 0;

}; /* class ILogger { */


inline ILogger::LogHelper<LOGPRIORITY_FATAL> ILogger::fatal() noexcept {
    return LogHelper<LOGPRIORITY_FATAL>(*this);
}

inline ILogger::LogHelper<LOGPRIORITY_ERROR> ILogger::error() noexcept {
    return LogHelper<LOGPRIORITY_ERROR>(*this);
}

inline ILogger::LogHelper<LOGPRIORITY_WARNING> ILogger::warning() noexcept {
    return LogHelper<LOGPRIORITY_WARNING>(*this);
}

inline ILogger::LogHelper<LOGPRIORITY_NORMAL> ILogger::info() noexcept {
    return LogHelper<LOGPRIORITY_NORMAL>(*this);
}

inline ILogger::LogHelper<LOGPRIORITY_DEBUG> ILogger::debug() noexcept {
    return LogHelper<LOGPRIORITY_DEBUG>(*this);
}

inline ILogger::LogHelper<LOGPRIORITY_FULLDEBUG> ILogger::fullDebug() noexcept {
    return LogHelper<LOGPRIORITY_FULLDEBUG>(*this);
}

template <class ILogger__>
inline typename ILogger::PrefixedWrapper<ILogger__>::template Helper<LOGPRIORITY_FATAL>::type ILogger::PrefixedWrapper<ILogger__>::fatal() noexcept {
    typename Helper<LOGPRIORITY_FATAL>::type logger(m_logger.fatal());
    logger << m_prefix;
    logger.setAsPrefix();
    return logger;
}

template <class ILogger__>
inline typename ILogger::PrefixedWrapper<ILogger__>::template Helper<LOGPRIORITY_ERROR>::type ILogger::PrefixedWrapper<ILogger__>::error() noexcept {
    typename Helper<LOGPRIORITY_ERROR>::type logger(m_logger.error());
    logger << m_prefix;
    logger.setAsPrefix();
    return logger;
}

template <class ILogger__>
inline typename ILogger::PrefixedWrapper<ILogger__>::template Helper<LOGPRIORITY_WARNING>::type ILogger::PrefixedWrapper<ILogger__>::warning() noexcept {
    typename Helper<LOGPRIORITY_WARNING>::type logger(m_logger.warning());
    logger << m_prefix;
    logger.setAsPrefix();
    return logger;
}

template <class ILogger__>
inline typename ILogger::PrefixedWrapper<ILogger__>::template Helper<LOGPRIORITY_NORMAL>::type ILogger::PrefixedWrapper<ILogger__>::info() noexcept {
    typename Helper<LOGPRIORITY_NORMAL>::type logger(m_logger.info());
    logger << m_prefix;
    logger.setAsPrefix();
    return logger;
}

template <class ILogger__>
inline typename ILogger::PrefixedWrapper<ILogger__>::template Helper<LOGPRIORITY_DEBUG>::type ILogger::PrefixedWrapper<ILogger__>::debug() noexcept {
    typename Helper<LOGPRIORITY_DEBUG>::type logger(m_logger.debug());
    logger << m_prefix;
    logger.setAsPrefix();
    return logger;
}

template <class ILogger__>
inline typename ILogger::PrefixedWrapper<ILogger__>::template Helper<LOGPRIORITY_FULLDEBUG>::type ILogger::PrefixedWrapper<ILogger__>::fullDebug() noexcept {
    typename Helper<LOGPRIORITY_FULLDEBUG>::type logger(m_logger.fullDebug());
    logger << m_prefix;
    logger.setAsPrefix();
    return logger;
}

} /* namespace sharemind { */

#endif /* SHAREMINDCOMMON_ILOGGER_H */
