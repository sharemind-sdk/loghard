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
#include "../Abort.h"
#include "../Concat.h"
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

namespace Detail {
namespace Logger {

static constexpr const size_t MAX_MESSAGE_SIZE = 1024u * 16u;
static_assert(MAX_MESSAGE_SIZE >= 1u, "Invalid MAX_MESSAGE_SIZE");
static constexpr const size_t STACK_BUFFER_SIZE = MAX_MESSAGE_SIZE + 4u;
static_assert(STACK_BUFFER_SIZE > MAX_MESSAGE_SIZE, "Overflow");

#if !defined(SHAREMIND_GCC_VERSION) || SHAREMIND_GCC_VERSION >= 40800
extern thread_local timeval tl_time;
extern thread_local LogBackend * tl_backend;
extern thread_local size_t tl_offset;
extern thread_local char tl_message[STACK_BUFFER_SIZE];
#endif

} /* namespace Logger { */
} /* namespace Detail { */

class Logger {

public: /* Types: */

    template <typename T> struct Hex {
        static_assert(std::is_arithmetic<T>::value, "T is not arithmetic!");
        static_assert(std::is_unsigned<T>::value, "T is not unsigned!");
        T const value;
    };

    template <LogPriority> class LogHelper;

    class NullLogHelperBase {

        template <LogPriority> friend class LogHelper;

    public : /* Methods: */

        template <class T>
        inline NullLogHelperBase & operator<<(T &&) noexcept
        { return *this; }

    private: /* Methods: */

        inline NullLogHelperBase(const LogBackend &,
                                 const char * const) noexcept {}

    }; /* class NullLogHelperBase { */

    template <LogPriority priority>
    class LogHelperBase {

        template <LogPriority> friend class LogHelper;

    public: /* Methods: */

        LogHelperBase(const LogHelperBase<priority> &) = delete;
        LogHelperBase<priority> & operator=(const LogHelperBase<priority> &) =
                delete;

        inline LogHelperBase(LogHelperBase<priority> && move) noexcept
            : m_operational(move.m_operational)
            #if defined(SHAREMIND_GCC_VERSION) && SHAREMIND_GCC_VERSION < 40800
            , tl_time(std::move(move.tl_time))
            , tl_backend(move.tl_backend)
            , tl_offset(move.tl_offset)
            #endif
        {
            move.m_operational = false;
            #if defined(SHAREMIND_GCC_VERSION) && SHAREMIND_GCC_VERSION < 40800
            using namespace Detail::Logger;
            memcpy(tl_message, move.tl_message, STACK_BUFFER_SIZE);
            #endif
        }

        inline LogHelperBase<priority> & operator=(
                const LogHelperBase<priority> && move)
        {
            m_operational = move.m_operational;
            move.m_operational = false;
            #if defined(SHAREMIND_GCC_VERSION) && SHAREMIND_GCC_VERSION < 40800
            tl_time = std::move(move.tl_time);
            tl_backend = move.tl_backend;
            tl_offset = move.tl_offset;
            using namespace Detail::Logger;
            memcpy(tl_message, move.tl_message, STACK_BUFFER_SIZE);
            #endif
        }

        inline ~LogHelperBase() noexcept {
            if (!m_operational)
                return;
            using namespace Detail::Logger;
            assert(tl_offset <= STACK_BUFFER_SIZE);
            assert(tl_offset < STACK_BUFFER_SIZE
                   || tl_message[STACK_BUFFER_SIZE - 1u] == '\0');
            if (tl_offset < STACK_BUFFER_SIZE)
                tl_message[tl_offset] = '\0';
            assert(tl_backend);
            tl_backend->doLog<priority>(std::move(tl_time), tl_message);
        }

        inline LogHelperBase & operator<<(const char v) noexcept {
            assert(m_operational);
            using namespace Detail::Logger;
            if (tl_offset <= MAX_MESSAGE_SIZE) {
                if (tl_offset == MAX_MESSAGE_SIZE)
                    return elide();
                tl_message[tl_offset] = v;
                tl_offset++;
            }
            return *this;
        }

        inline LogHelperBase & operator<<(const bool v) noexcept
        { return this->operator<<(v ? '1' : '0'); }

#define SHAREMINDCOMMON_LOGGER_LHB_OP(valueType,valueGetter,formatString) \
    inline LogHelperBase & operator<<(valueType const v) noexcept { \
        assert(m_operational); \
        using namespace Detail::Logger; \
        if (tl_offset > MAX_MESSAGE_SIZE) { \
            assert(tl_offset == STACK_BUFFER_SIZE); \
            return *this; \
        } \
        const size_t spaceLeft = MAX_MESSAGE_SIZE - tl_offset; \
        if (!spaceLeft) \
            return elide(); \
        const int r = snprintf(&tl_message[tl_offset], \
                               spaceLeft, \
                               (formatString), \
                               v valueGetter); \
        if (r < 0) \
            SHAREMIND_ABORT("LLHBo"); \
        if (static_cast<size_t>(r) > spaceLeft) { \
            tl_offset = MAX_MESSAGE_SIZE; \
            return elide(); \
        } \
        tl_offset += r; \
        return *this; \
    }

        SHAREMINDCOMMON_LOGGER_LHB_OP(signed char,, "%hhd")
        SHAREMINDCOMMON_LOGGER_LHB_OP(unsigned char,, "%hhu")
        SHAREMINDCOMMON_LOGGER_LHB_OP(short,, "%hd")
        SHAREMINDCOMMON_LOGGER_LHB_OP(unsigned short,, "%hu")
        SHAREMINDCOMMON_LOGGER_LHB_OP(int,, "%d")
        SHAREMINDCOMMON_LOGGER_LHB_OP(unsigned int,, "%u")
        SHAREMINDCOMMON_LOGGER_LHB_OP(long,, "%ld")
        SHAREMINDCOMMON_LOGGER_LHB_OP(unsigned long,, "%lu")
        SHAREMINDCOMMON_LOGGER_LHB_OP(long long,, "%lld")
        SHAREMINDCOMMON_LOGGER_LHB_OP(unsigned long long,, "%llu")

        SHAREMINDCOMMON_LOGGER_LHB_OP(Logger::Hex<unsigned char>,
                                      .value,
                                      "%hhx")
        SHAREMINDCOMMON_LOGGER_LHB_OP(Logger::Hex<unsigned short>,.value, "%hx")
        SHAREMINDCOMMON_LOGGER_LHB_OP(Logger::Hex<unsigned int>,.value, "%x")
        SHAREMINDCOMMON_LOGGER_LHB_OP(Logger::Hex<unsigned long>,.value, "%lx")
        SHAREMINDCOMMON_LOGGER_LHB_OP(Logger::Hex<unsigned long long>,
                                      .value,
                                      "%llx")

        SHAREMINDCOMMON_LOGGER_LHB_OP(double,, "%f")
        SHAREMINDCOMMON_LOGGER_LHB_OP(long double,, "%Lf")

        inline LogHelperBase & operator<<(const float v) noexcept
        { return this->operator<<((const double) v); }

        inline LogHelperBase & operator<<(const char * v) noexcept {
            assert(v);
            assert(m_operational);
            using namespace Detail::Logger;
            size_t o = tl_offset;
            if (o > MAX_MESSAGE_SIZE) {
                assert(o == STACK_BUFFER_SIZE);
                return *this;
            }
            if (*v) {
                do {
                    if (o == MAX_MESSAGE_SIZE) {
                        tl_offset = o;
                        return elide();
                    }
                    tl_message[o] = *v;
                } while ((++o, *++v));
                tl_offset = o;
            }
            return *this;
        }

        inline LogHelperBase & operator<<(const std::string & v) noexcept
        { return this->operator<<(v.c_str()); }

        SHAREMINDCOMMON_LOGGER_LHB_OP(void *,, "%p")

        inline LogHelperBase & operator<<(const void * const v) noexcept
        { return this->operator<<(const_cast<void *>(v)); }

#undef SHAREMINDCOMMON_LOGGER_LHB_OP

    private: /* Methods: */

        inline LogHelperBase(LogBackend & logBackend,
                             const char * prefix) noexcept
        {
            using namespace Detail::Logger;
            { // For accuracy, take timestamp before everything else:
                #ifndef NDEBUG
                const int r =
                #endif
                        gettimeofday(&tl_time, nullptr);
                assert(r == 0);
            }

            assert(prefix);

            // Initialize other fields after timestamp:
            tl_backend = &logBackend;
            if (*prefix) {
                size_t o = 0u;
                do {
                    if (o == MAX_MESSAGE_SIZE)
                        break;
                    tl_message[o] = *prefix;
                } while ((++o, *++prefix));
                tl_offset = o;
            } else {
                tl_offset = 0u;
            }
            m_operational = true;
        }

        inline LogHelperBase & elide() noexcept {
            using namespace Detail::Logger;
            assert(tl_offset <= MAX_MESSAGE_SIZE);
            assert(m_operational);
            memcpy(&tl_message[tl_offset], "...", 4u);
            tl_offset = STACK_BUFFER_SIZE;
            return *this;
        }

    private: /* Fields: */

        bool m_operational;
        #if defined(SHAREMIND_GCC_VERSION) && SHAREMIND_GCC_VERSION < 40800
        timeval tl_time;
        LogBackend * tl_backend;
        size_t tl_offset;
        char tl_message[Detail::Logger::STACK_BUFFER_SIZE];
        #endif

    }; /* class LogHelperBase { */

    template <LogPriority priority = LogPriority::Debug>
    class LogHelper
            : public std::conditional<priority <= SHAREMIND_LOGLEVEL_MAXDEBUG,
                                      LogHelperBase<priority>,
                                      NullLogHelperBase>::type
    {

        friend class Logger;

    private: /* Methods: */

        inline LogHelper(LogBackend & backend,
                         const char * const prefix) noexcept
            : std::conditional<priority <= SHAREMIND_LOGLEVEL_MAXDEBUG,
                               LogHelperBase<priority>,
                               NullLogHelperBase>::type(backend, prefix)
        {}

    }; /* class LogHelper */

    template <LogPriority> friend class LogHelper;

public: /* Methods: */

    inline Logger(LogBackend & backend)
            noexcept
        : m_backend(backend)
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

    template <typename Arg, typename ... Args>
    inline Logger(const Logger & logger, Arg && arg, Args && ... args) noexcept
        : m_backend(logger.m_backend)
        , m_prefix(logger.m_prefix.empty()
                   ? concat(std::forward<Arg>(arg),
                            std::forward<Args>(args)...,
                            ' ')
                   : (*(logger.m_prefix.crbegin()) != ' ')
                     ? concat(logger.m_prefix,
                              std::forward<Arg>(arg),
                              std::forward<Args>(args)...,
                              ' ')
                     : concat(std::string(logger.m_prefix.cbegin(),
                                          logger.m_prefix.cend() - 1),
                              std::forward<Arg>(arg),
                              std::forward<Args>(args)...,
                              ' '))
    {}

    LogBackend & backend() const noexcept { return m_backend; }
    const std::string & prefix() const noexcept { return m_prefix; }

    inline LogHelper<LogPriority::Fatal> fatal() const noexcept
    { return {m_backend, m_prefix.c_str()}; }

    inline LogHelper<LogPriority::Error> error() const noexcept
    { return {m_backend, m_prefix.c_str()}; }

    inline LogHelper<LogPriority::Warning> warning() const noexcept
    { return {m_backend, m_prefix.c_str()}; }

    inline LogHelper<LogPriority::Normal> info() const noexcept
    { return {m_backend, m_prefix.c_str()}; }

    inline LogHelper<LogPriority::Debug> debug() const noexcept
    { return {m_backend, m_prefix.c_str()}; }

    inline LogHelper<LogPriority::FullDebug> fullDebug() const noexcept
    { return {m_backend, m_prefix.c_str()}; }

    template <typename T>
    inline static Hex<T> hex(T const value) noexcept { return {value}; }

private: /* Fields: */

    LogBackend & m_backend;
    const std::string m_prefix;

}; /* class Logger { */

} /* namespace sharemind { */

#endif /* SHAREMINDCOMMON_LOGGER_H */
