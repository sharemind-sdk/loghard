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
#include "../Concat.h"
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

class ILogger {

private: /* Types: */

    struct NullLogHelperBase {
        inline NullLogHelperBase(ILogger &) noexcept {}
        inline NullLogHelperBase(ILogger &, const std::string &) noexcept {}
        template <class T>
        inline NullLogHelperBase & operator<<(const T &) noexcept
        { return *this; }
    };

    template <LogPriority priority>
    class LogHelperBase {

    private: /* Types: */

        enum Status { NO_LOG, OPERATIONAL, NOT_OPERATIONAL };

    public: /* Methods: */

        inline LogHelperBase(ILogger & logger) noexcept
            : m_logger(&logger)
        {}

        inline LogHelperBase(ILogger & logger,
                             const std::string & prefix) noexcept
            : m_logger(&logger)
        { m_stream << prefix; }

        LogHelperBase(const LogHelperBase<priority> &) = delete;
        LogHelperBase<priority> & operator=(
                const LogHelperBase<priority> &) = delete;

        inline LogHelperBase(LogHelperBase<priority> && move)
                noexcept
            : /* m_stream(std::move(move.m_stream))
            , */ m_logger(move.m_logger)
            , m_operational(move.m_operational)
            , m_haveData(move.m_haveData)
        {
            /// \bug http://gcc.gnu.org/bugzilla/show_bug.cgi?id=54316
            m_stream << move.m_stream.str();
            move.m_stream.str("");
            move.m_stream.clear();

            move.m_operational = false;
        }

        inline LogHelperBase<priority> & operator=(
                LogHelperBase<priority> && move) noexcept
        {
            /// \bug http://gcc.gnu.org/bugzilla/show_bug.cgi?id=54316
            // m_stream = std::move(move.m_stream);
            m_stream << move.m_stream.str();
            move.m_stream.str("");
            move.m_stream.clear();

            m_logger = move.m_logger;
            m_operational = move.m_operational;
            m_haveData = move.m_haveData;
            move.m_operational = false;
        }

        inline ~LogHelperBase() noexcept {
            if (m_operational && m_haveData)
                m_logger->logMessage(priority,
                                     m_stream.str()); /// \bug might throw
        }

        template <class T>
        inline LogHelperBase & operator<<(T && v) noexcept {
            assert(m_operational);
            m_stream << std::forward<T>(v); /// \bug might throw
            m_haveData = true;
            return *this;
        }

    private: /* Fields: */

        std::ostringstream m_stream;
        ILogger * m_logger;
        bool m_operational = true;
        bool m_haveData = false;

    }; /* class LogHelperBase { */

public: /* Types: */

    template <LogPriority priority = LOGPRIORITY_DEBUG>
    class LogHelper
            : public std::conditional<priority <= SHAREMIND_LOGLEVEL_MAXDEBUG,
                                      LogHelperBase<priority>,
                                      NullLogHelperBase>::type
    {

    public: /* Methods: */

        inline LogHelper(ILogger & logger) noexcept
            : std::conditional<priority <= SHAREMIND_LOGLEVEL_MAXDEBUG,
                               LogHelperBase<priority>,
                               NullLogHelperBase>::type(logger)
        {}

        inline LogHelper(ILogger & logger, const std::string & prefix) noexcept
            : std::conditional<priority <= SHAREMIND_LOGLEVEL_MAXDEBUG,
                               LogHelperBase<priority>,
                               NullLogHelperBase>::type(logger, prefix)
        {}

    };

    class PrefixedWrapper {

    public: /* Types: */

        typedef PrefixedWrapper Wrapped;

    public: /* Methods: */

        template <typename Arg, typename ... Args>
        inline PrefixedWrapper(ILogger & logger, Arg && arg, Args && ... args)
                noexcept
            : m_iLogger(logger)
            , m_prefix(concat(std::forward<Arg>(arg),
                              std::forward<Args>(args)...,
                              ' '))
        {}

        inline PrefixedWrapper(const PrefixedWrapper & logger) noexcept
            : m_iLogger(logger.m_iLogger)
            , m_prefix(logger.m_prefix)
        {}

        template <typename ... Args>
        inline PrefixedWrapper(const PrefixedWrapper & logger,
                               Args && ... args) noexcept
            : m_iLogger(logger.m_iLogger)
            , m_prefix(concat(trimLastChar(logger.m_prefix),
                              std::forward<Args>(args)..., ' '))
        {}

        ILogger & iLogger() const noexcept { return m_iLogger; }

        inline LogHelper<LOGPRIORITY_FATAL> fatal() noexcept
        { return {m_iLogger, m_prefix}; }

        inline LogHelper<LOGPRIORITY_ERROR> error() noexcept
        { return {m_iLogger, m_prefix}; }

        inline LogHelper<LOGPRIORITY_WARNING> warning() noexcept
        { return {m_iLogger, m_prefix}; }

        inline LogHelper<LOGPRIORITY_NORMAL> info() noexcept
        { return {m_iLogger, m_prefix}; }

        inline LogHelper<LOGPRIORITY_DEBUG> debug() noexcept
        { return {m_iLogger, m_prefix}; }

        inline LogHelper<LOGPRIORITY_FULLDEBUG> fullDebug() noexcept
        { return {m_iLogger, m_prefix}; }

    private: /* Methods: */

        static inline std::string trimLastChar(const std::string & s)
        { return std::string(s.cbegin(), s.cend() - 1); }

    private: /* Fields: */

        ILogger & m_iLogger;
        const std::string m_prefix;

    };

    typedef PrefixedWrapper Wrapped;

public: /* Methods: */

    inline LogHelper<LOGPRIORITY_FATAL> fatal() noexcept { return *this; }
    inline LogHelper<LOGPRIORITY_ERROR> error() noexcept { return *this; }
    inline LogHelper<LOGPRIORITY_WARNING> warning() noexcept { return *this; }
    inline LogHelper<LOGPRIORITY_NORMAL> info() noexcept { return *this; }
    inline LogHelper<LOGPRIORITY_DEBUG> debug() noexcept { return *this; }
    inline LogHelper<LOGPRIORITY_FULLDEBUG> fullDebug() noexcept
    { return *this; }

protected: /* Methods: */

    /**
     Logs a message with the specified priority.

     \param[in] priority the priority level.
     \param[in] message the message to log.
    */
    virtual void logMessage(LogPriority priority,
                            const char * message) noexcept = 0;

    /**
     Logs a message with the specified priority.

     \param[in] priority the priority level.
     \param[in] message the message to log.
    */
    virtual void logMessage(LogPriority priority,
                            const std::string & message) noexcept = 0;

    /**
     Logs a message with the specified priority.

     \param[in] priority the priority level.
     \param[in] message the message to log.
    */
    virtual void logMessage(LogPriority priority,
                            std::string && message) noexcept = 0;

}; /* class ILogger { */

} /* namespace sharemind { */

#endif /* SHAREMINDCOMMON_ILOGGER_H */
