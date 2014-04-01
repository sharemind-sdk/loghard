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
#include <cassert>
#include <sstream>
#include <string>
#include <type_traits>
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
        template <class T> inline NullLogHelper & operator<<(const T &) noexcept
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

        inline void setAsPrefix() noexcept {
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

        inline PrefixedLogHelperBase<priority> & appendPrefix() noexcept
        { return *this; }

        template <typename Arg, typename ... Args>
        inline PrefixedLogHelperBase<priority> & appendPrefix(Arg && arg,
                                                              Args && ... args)
                noexcept
        {
            typedef PrefixedLogHelperBase<priority> Self;
            return static_cast<Self &>((*this) << std::forward<Arg>(arg))
                       .appendPrefix(std::forward<Args>(args)...);
        }

    };

public: /* Types: */

    template <LogPriority priority = LOGPRIORITY_DEBUG>
    class LogHelper
            : public std::conditional<priority <= SHAREMIND_LOGLEVEL_MAXDEBUG,
                                      PrefixedLogHelperBase<priority>,
                                      NullLogHelper>::type
    {

    public: /* Methods: */

        template <typename ... Args>
        inline LogHelper(ILogger & logger, Args && ... args) noexcept
            : std::conditional<priority <= SHAREMIND_LOGLEVEL_MAXDEBUG,
                               PrefixedLogHelperBase<priority>,
                               NullLogHelper>::type(logger,
                                                    std::forward<Args>(args)...)
        {}

    };

    template <class ILogger__ = ILogger>
    class PrefixedWrapper {

    public: /* Types: */

        typedef PrefixedWrapper<PrefixedWrapper<ILogger__> > Wrapped;

    public: /* Methods: */

        template <typename ... Args>
        inline PrefixedWrapper(ILogger__ & logger, Args && ... args) noexcept
            : m_logger(logger)
            , m_prefix(std::forward<Args>(args)...) /// \bug might throw
        {}

        inline LogHelper<LOGPRIORITY_FATAL> fatal() noexcept
        { return getHelper<LOGPRIORITY_FATAL, &ILogger__::fatal>(); }

        inline LogHelper<LOGPRIORITY_ERROR> error() noexcept
        { return getHelper<LOGPRIORITY_ERROR, &ILogger__::error>(); }

        inline LogHelper<LOGPRIORITY_WARNING> warning() noexcept
        { return getHelper<LOGPRIORITY_WARNING, &ILogger__::warning>(); }

        inline LogHelper<LOGPRIORITY_NORMAL> info() noexcept
        { return getHelper<LOGPRIORITY_NORMAL, &ILogger__::info>(); }

        inline LogHelper<LOGPRIORITY_DEBUG> debug() noexcept
        { return getHelper<LOGPRIORITY_DEBUG, &ILogger__::debug>(); }

        inline LogHelper<LOGPRIORITY_FULLDEBUG> fullDebug() noexcept
        { return getHelper<LOGPRIORITY_FULLDEBUG, &ILogger__::fullDebug>(); }

        template <typename ... PrefixTypes>
        inline Wrapped wrap(PrefixTypes && ... prefix) noexcept {
            return Wrapped(*this, std::forward<PrefixTypes>(prefix)...);
        }

    private: /* Methods: */

        template <LogPriority priority,
                  LogHelper<priority> (ILogger__::*helperGetter)() noexcept>
        inline LogHelper<priority> getHelper() noexcept {
            LogHelper<priority> logger((m_logger.*helperGetter)());
            (logger << m_prefix).setAsPrefix();
            return logger;
        }

    private: /* Fields: */

        ILogger__ & m_logger;
        const std::string m_prefix;

    };

    typedef PrefixedWrapper<ILogger> Wrapped;

public: /* Methods: */

    inline LogHelper<LOGPRIORITY_FATAL> fatal() noexcept
    { return LogHelper<LOGPRIORITY_FATAL>(*this); }

    inline LogHelper<LOGPRIORITY_ERROR> error() noexcept
    { return LogHelper<LOGPRIORITY_ERROR>(*this); }

    inline LogHelper<LOGPRIORITY_WARNING> warning() noexcept
    { return LogHelper<LOGPRIORITY_WARNING>(*this); }

    inline LogHelper<LOGPRIORITY_NORMAL> info() noexcept
    { return LogHelper<LOGPRIORITY_NORMAL>(*this); }

    inline LogHelper<LOGPRIORITY_DEBUG> debug() noexcept
    { return LogHelper<LOGPRIORITY_DEBUG>(*this); }

    inline LogHelper<LOGPRIORITY_FULLDEBUG> fullDebug() noexcept
    { return LogHelper<LOGPRIORITY_FULLDEBUG>(*this); }

    template <typename ... PrefixTypes>
    inline Wrapped wrap(PrefixTypes && ... prefixes) noexcept {
        return Wrapped(*this, std::forward<PrefixTypes>(prefixes)...);
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

} /* namespace sharemind { */

#endif /* SHAREMINDCOMMON_ILOGGER_H */
