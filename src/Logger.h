/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SHAREMINDCOMMON_LOGGER_H
#define SHAREMINDCOMMON_LOGGER_H

#include <cassert>
#include <string>
#include <sys/time.h>
#include <type_traits>
#include <utility>
#include "../Concat.h"
#include "../GccIsNothrowDestructible.h"
#include "../GccVersion.h"
#include "LogBackend.h"
#include "LogPriority.h"


#ifndef SHAREMIND_LOGLEVEL_MAXDEBUG
#if defined(SHAREMIND_LOGLEVEL_FULLDEBUG)
#define SHAREMIND_LOGLEVEL_MAXDEBUG ::sharemind::LogPriority::FullDebug
#elif defined(SHAREMIND_LOGLEVEL_DEBUG)
#define SHAREMIND_LOGLEVEL_MAXDEBUG ::sharemind::LogPriority::Debug
#elif defined(SHAREMIND_LOGLEVEL_NORMAL)
#define SHAREMIND_LOGLEVEL_MAXDEBUG ::sharemind::LogPriority::Normal
#elif defined(SHAREMIND_LOGLEVEL_WARNING)
#define SHAREMIND_LOGLEVEL_MAXDEBUG ::sharemind::LogPriority::Warning
#elif defined(SHAREMIND_LOGLEVEL_ERROR)
#define SHAREMIND_LOGLEVEL_MAXDEBUG ::sharemind::LogPriority::Error
#else
#define SHAREMIND_LOGLEVEL_MAXDEBUG ::sharemind::LogPriority::Debug
#endif
#endif

namespace sharemind {

class Logger {

public: /* Types: */

    template <LogPriority> class LogHelper;

    class NullLogHelperBase {

        template <LogPriority> friend class LogHelper;

    public : /* Methods: */

        template <class T>
        inline NullLogHelperBase & operator<<(T &&) noexcept
        { return *this; }

    private: /* Methods: */

        template <typename Prefix>
        inline NullLogHelperBase(const LogBackend &, Prefix &&) noexcept
        {}

    }; /* class NullLogHelperBase { */

    template <LogPriority priority>
    class LogHelperBase {

        template <LogPriority> friend class LogHelper;

    private: /* Types: */

        using InnerStream = std::ostringstream;
        #if !defined(SHAREMIND_GCC_VERSION) || (SHAREMIND_GCC_VERSION >= 40800)
        #ifndef SHAREMIND_SILENCE_WORKAROUND_WARNINGS
        #warning Disabling std::ostringstream::~ostringstream noexcept check \
                 for g++ older than < 4.8. Define \
                 SHAREMIND_SILENCE_WORKAROUND_WARNINGS to silence.
        #endif
        static_assert(std::is_nothrow_destructible<InnerStream>::value,
                      "std::ostringstream::~ostringstream not noexcept");
        #endif

    public: /* Methods: */

        LogHelperBase(const LogHelperBase<priority> &) = delete;
        LogHelperBase<priority> & operator=(
                const LogHelperBase<priority> &) = delete;

        inline LogHelperBase(LogHelperBase<priority> && move) noexcept {
            if (move.m_operational) {
                try {
                    /// \todo http://gcc.gnu.org/bugzilla/show_bug.cgi?id=54316
                    // new (&m_stream) InnerStream(std::move(move.m_stream));
                    new (&m_stream) InnerStream();
                    try {
                        m_stream << move.m_stream.str();
                        m_time = std::move(move.m_time);
                        m_backend = std::move(move.m_backend);
                        m_haveData = std::move(move.m_haveData);
                        m_operational = true;
                    } catch (...) {
                        m_stream.~InnerStream();
                        throw;
                    }
                } catch (...) {
                    //if (move.m_haveData)
                    //    m_logger->dropMessage(priority);
                    m_operational = false;
                }
                move.m_operational = false;
                move.m_stream.~InnerStream();
            } else {
                m_operational = false;
            }
        }

        inline LogHelperBase<priority> & operator=(
                LogHelperBase<priority> && move) noexcept
        {
            if (m_operational)
                m_stream.~InnerStream();
            new (this) LogHelperBase<priority>(std::move(move));
            return *this;
        }

        inline ~LogHelperBase() noexcept {
            if (m_operational) {
                if (m_haveData)
                    m_backend->doLog<priority>(std::move(m_time),
                                              m_stream.str());
                m_stream.~InnerStream();
            }
        }

        template <class T>
        inline LogHelperBase & operator<<(T && v) noexcept {
            if (m_operational) {
                try {
                    m_stream << std::forward<T>(v);
                    m_haveData = true;
                } catch (...) {
                    m_stream.~InnerStream();
                    //m_logger->dropMessage(priority);
                    m_operational = false;
                }
            }
            return *this;
        }

    private: /* Methods: */

        template <typename Prefix>
        inline LogHelperBase(LogBackend & logbackend, Prefix && prefix) noexcept {
            #ifndef NDEBUG
            const int r =
            #endif
                    gettimeofday(&m_time, nullptr);
            assert(r == 0);
            try {
                new (&m_stream) InnerStream();
                try {
                    m_stream << std::forward<Prefix>(prefix);
                    m_backend = &logbackend;
                    m_haveData = false;
                    m_operational = true;
                } catch (...) {
                    m_stream.~InnerStream();
                    throw;
                }
            } catch (...) {
                m_operational = false;
            }
        }

    private: /* Fields: */

        timeval m_time;
        union {
            char m_unused;
            InnerStream m_stream;
        };
        LogBackend * m_backend;
        bool m_haveData;
        bool m_operational;

    }; /* class LogHelperBase { */

    template <LogPriority priority = LogPriority::Debug>
    class LogHelper
            : public std::conditional<priority <= SHAREMIND_LOGLEVEL_MAXDEBUG,
                                      LogHelperBase<priority>,
                                      NullLogHelperBase>::type
    {

        friend class Logger;

    private: /* Methods: */

        template <typename Prefix = char>
        inline LogHelper(LogBackend & backend,
                         Prefix && prefix = Prefix(' ')) noexcept
            : std::conditional<priority <= SHAREMIND_LOGLEVEL_MAXDEBUG,
                               LogHelperBase<priority>,
                               NullLogHelperBase>::type(
                                   backend,
                                   std::forward<Prefix>(prefix))
        {}

    }; /* class LogHelper */

    template <LogPriority> friend class LogHelper;

public: /* Methods: */

    inline Logger(LogBackend & backend)
            noexcept
        : m_backend(backend)
        , m_prefix(1, ' ')
    {}

    template <typename Arg, typename ... Args>
    inline Logger(LogBackend & backend, Arg && arg, Args && ... args)
            noexcept
        : m_backend(backend)
        , m_prefix(concat(std::forward<Arg>(arg),
                          std::forward<Args>(args)...,
                          ' '))
    {}

    inline Logger(const Logger & logger) noexcept
        : m_backend(logger.m_backend)
        , m_prefix(logger.m_prefix)
    {}

    template <typename ... Args>
    inline Logger(const Logger & logger, Args && ... args) noexcept
        : m_backend(logger.m_backend)
        , m_prefix(concat(trimLastChar(logger.m_prefix),
                          std::forward<Args>(args)..., ' '))
    {}

    LogBackend & backend() const noexcept { return m_backend; }

    inline LogHelper<LogPriority::Fatal> fatal() const noexcept
    { return {m_backend, m_prefix}; }

    inline LogHelper<LogPriority::Error> error() const noexcept
    { return {m_backend, m_prefix}; }

    inline LogHelper<LogPriority::Warning> warning() const noexcept
    { return {m_backend, m_prefix}; }

    inline LogHelper<LogPriority::Normal> info() const noexcept
    { return {m_backend, m_prefix}; }

    inline LogHelper<LogPriority::Debug> debug() const noexcept
    { return {m_backend, m_prefix}; }

    inline LogHelper<LogPriority::FullDebug> fullDebug() const noexcept
    { return {m_backend, m_prefix}; }

private: /* Methods: */

    static inline std::string trimLastChar(const std::string & s)
    { return std::string(s.cbegin(), s.cend() - 1); }

private: /* Fields: */

    LogBackend & m_backend;
    const std::string m_prefix;

}; /* class Logger { */

} /* namespace sharemind { */

#endif /* SHAREMINDCOMMON_LOGGER_H */
