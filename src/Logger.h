/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef LOGHARD_LOGGER_H
#define LOGHARD_LOGGER_H

#include <cassert>
#include <sharemind/compiler-support/GccVersion.h>
#include <sstream>
#include <string>
#include <sys/time.h>
#include <type_traits>
#include <utility>
#include "Backend.h"
#include "Priority.h"


#ifndef LOGHARD_LOGLEVEL_MAXDEBUG
#define LOGHARD_LOGLEVEL_MAXDEBUG ::LogHard::Priority::Debug
#endif

/*
  TLS is Clang is only available when using Clang with GNU libstdc++ 4.8 or
  later. However there's no good way to determine the version of libstdc++ using
  preprocessor macros. The __GLIBCXX__ macro gives us a date, but unfortunately
  its value in libstdc++ 4.7.4 is greater than in 4.8.0, hence it is not
  entirely reliable.
*/
#if (defined(__clang__) && (!defined(__GLIBCXX__) \
                            || __GLIBCXX__ < 20130322)) || \
    (defined(SHAREMIND_GCC_VERSION) && SHAREMIND_GCC_VERSION < 40800)
#define LOGHARD_HAVE_TLS 0
#else
#define LOGHARD_HAVE_TLS 1
#endif

namespace LogHard {

namespace Detail {

static constexpr const size_t MAX_MESSAGE_SIZE = 1024u * 16u;
static_assert(MAX_MESSAGE_SIZE >= 1u, "Invalid MAX_MESSAGE_SIZE");
static constexpr const size_t STACK_BUFFER_SIZE = MAX_MESSAGE_SIZE + 4u;
static_assert(STACK_BUFFER_SIZE > MAX_MESSAGE_SIZE, "Overflow");

#if LOGHARD_HAVE_TLS
extern thread_local timeval tl_time;
extern thread_local Backend * tl_backend;
extern thread_local size_t tl_offset;
extern thread_local char tl_message[STACK_BUFFER_SIZE];
#endif

template <typename Arg>
inline std::string concatBase(std::ostringstream && oss, Arg && arg) {
    oss << std::forward<Arg>(arg);
    return oss.str();
}

template <typename Arg, typename ... Args>
inline std::string concatBase(std::ostringstream && oss,
                              Arg && arg,
                              Args && ... args)
{
    oss << std::move(arg);
    return concatBase(std::move(oss), std::forward<Args>(args)...);
}

inline std::string concat() { return std::string(); }
inline std::string concat(const char * const s) { return s; }
inline std::string concat(std::string && s) { return std::move(s); }
inline std::string concat(const std::string & s) { return s; }

template <typename ... T>
inline std::string concat(T && ... args)
{ return Detail::concatBase(std::ostringstream(), std::forward<T>(args)...); }

} /* namespace Detail { */

class Logger {

public: /* Types: */

    template <typename T> struct Hex {
        static_assert(std::is_arithmetic<T>::value, "T is not arithmetic!");
        static_assert(std::is_unsigned<T>::value, "T is not unsigned!");
        T const value;
    };

    template <Priority> class LogHelper;

    class NullLogHelperBase {

        template <Priority> friend class LogHelper;

    public : /* Methods: */

        template <class T>
        inline NullLogHelperBase & operator<<(T &&) noexcept
        { return *this; }

    private: /* Methods: */

        inline NullLogHelperBase(const Backend &, const char * const) noexcept
        {}

    }; /* class NullLogHelperBase { */

    template <Priority priority>
    class LogHelperBase {

        template <Priority> friend class LogHelper;

    public: /* Methods: */

        LogHelperBase(const LogHelperBase<priority> &) = delete;
        LogHelperBase<priority> & operator=(const LogHelperBase<priority> &) =
                delete;

        inline LogHelperBase(LogHelperBase<priority> && move) noexcept
            : m_operational(move.m_operational)
            #if ! LOGHARD_HAVE_TLS
            , tl_time(std::move(move.tl_time))
            , tl_backend(move.tl_backend)
            , tl_offset(move.tl_offset)
            #endif
        {
            move.m_operational = false;
            #if ! LOGHARD_HAVE_TLS
            using namespace ::LogHard::Detail;
            memcpy(tl_message, move.tl_message, STACK_BUFFER_SIZE);
            #endif
        }

        inline LogHelperBase<priority> & operator=(
                const LogHelperBase<priority> && move)
        {
            m_operational = move.m_operational;
            move.m_operational = false;
            #if ! LOGHARD_HAVE_TLS
            tl_time = std::move(move.tl_time);
            tl_backend = move.tl_backend;
            tl_offset = move.tl_offset;
            using namespace ::LogHard::Detail;
            memcpy(tl_message, move.tl_message, STACK_BUFFER_SIZE);
            #endif
        }

        inline ~LogHelperBase() noexcept {
            if (!m_operational)
                return;
            using namespace ::LogHard::Detail;
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
            using namespace ::LogHard::Detail;
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

#define LOGHARD_LHB_OP(valueType,valueGetter,formatString) \
    inline LogHelperBase & operator<<(valueType const v) noexcept { \
        assert(m_operational); \
        using namespace ::LogHard::Detail; \
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
            return elide(); \
        if (static_cast<size_t>(r) > spaceLeft) { \
            tl_offset = MAX_MESSAGE_SIZE; \
            return elide(); \
        } \
        tl_offset += static_cast<unsigned>(r); \
        return *this; \
    }

        LOGHARD_LHB_OP(signed char,, "%hhd")
        LOGHARD_LHB_OP(unsigned char,, "%hhu")
        LOGHARD_LHB_OP(short,, "%hd")
        LOGHARD_LHB_OP(unsigned short,, "%hu")
        LOGHARD_LHB_OP(int,, "%d")
        LOGHARD_LHB_OP(unsigned int,, "%u")
        LOGHARD_LHB_OP(long,, "%ld")
        LOGHARD_LHB_OP(unsigned long,, "%lu")
        LOGHARD_LHB_OP(long long,, "%lld")
        LOGHARD_LHB_OP(unsigned long long,, "%llu")

        LOGHARD_LHB_OP(Logger::Hex<unsigned char>,.value,"%hhx")
        LOGHARD_LHB_OP(Logger::Hex<unsigned short>,.value, "%hx")
        LOGHARD_LHB_OP(Logger::Hex<unsigned int>,.value, "%x")
        LOGHARD_LHB_OP(Logger::Hex<unsigned long>,.value, "%lx")
        LOGHARD_LHB_OP(Logger::Hex<unsigned long long>,.value,"%llx")

        LOGHARD_LHB_OP(double,, "%f")
        LOGHARD_LHB_OP(long double,, "%Lf")

        inline LogHelperBase & operator<<(const float v) noexcept
        { return this->operator<<(static_cast<const double>(v)); }

        inline LogHelperBase & operator<<(const char * v) noexcept {
            assert(v);
            assert(m_operational);
            using namespace ::LogHard::Detail;
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

        LOGHARD_LHB_OP(void *,, "%p")

        inline LogHelperBase & operator<<(const void * const v) noexcept
        { return this->operator<<(const_cast<void *>(v)); }

#undef LOGHARD_LHB_OP

    private: /* Methods: */

        inline LogHelperBase(Backend & backend,
                             const char * prefix) noexcept
        {
            using namespace ::LogHard::Detail;
            { // For accuracy, take timestamp before everything else:
                #ifndef NDEBUG
                const int r =
                #endif
                        gettimeofday(&tl_time, nullptr);
                assert(r == 0);
            }

            assert(prefix);

            // Initialize other fields after timestamp:
            tl_backend = &backend;
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
            using namespace ::LogHard::Detail;
            assert(tl_offset <= MAX_MESSAGE_SIZE);
            assert(m_operational);
            memcpy(&tl_message[tl_offset], "...", 4u);
            tl_offset = STACK_BUFFER_SIZE;
            return *this;
        }

    private: /* Fields: */

        bool m_operational;
        #if ! LOGHARD_HAVE_TLS
        timeval tl_time;
        Backend * tl_backend;
        size_t tl_offset;
        char tl_message[::LogHard::Detail::STACK_BUFFER_SIZE];
        #endif

    }; /* class LogHelperBase { */

    template <Priority priority = Priority::Debug>
    class LogHelper
            : public std::conditional<priority <= LOGHARD_LOGLEVEL_MAXDEBUG,
                                      LogHelperBase<priority>,
                                      NullLogHelperBase>::type
    {

        friend class Logger;

    private: /* Methods: */

        inline LogHelper(Backend & backend,
                         const char * const prefix) noexcept
            : std::conditional<priority <= LOGHARD_LOGLEVEL_MAXDEBUG,
                               LogHelperBase<priority>,
                               NullLogHelperBase>::type(backend, prefix)
        {}

    }; /* class LogHelper */

    template <Priority> friend class LogHelper;

public: /* Methods: */

    inline Logger(Backend & backend)
            noexcept
        : m_backend(backend)
    {}

    template <typename Arg, typename ... Args>
    inline Logger(Backend & backend, Arg && arg, Args && ... args)
            noexcept
        : m_backend(backend)
        , m_prefix(Detail::concat(std::forward<Arg>(arg),
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
                   ? Detail::concat(std::forward<Arg>(arg),
                                    std::forward<Args>(args)...,
                                    ' ')
                   : (*(logger.m_prefix.crbegin()) != ' ')
                     ? Detail::concat(logger.m_prefix,
                                      std::forward<Arg>(arg),
                                      std::forward<Args>(args)...,
                                      ' ')
                     : Detail::concat(std::string(logger.m_prefix.cbegin(),
                                                  logger.m_prefix.cend() - 1),
                                      std::forward<Arg>(arg),
                                      std::forward<Args>(args)...,
                                      ' '))
    {}

    Backend & backend() const noexcept { return m_backend; }
    const std::string & prefix() const noexcept { return m_prefix; }

    inline LogHelper<Priority::Fatal> fatal() const noexcept
    { return {m_backend, m_prefix.c_str()}; }

    inline LogHelper<Priority::Error> error() const noexcept
    { return {m_backend, m_prefix.c_str()}; }

    inline LogHelper<Priority::Warning> warning() const noexcept
    { return {m_backend, m_prefix.c_str()}; }

    inline LogHelper<Priority::Normal> info() const noexcept
    { return {m_backend, m_prefix.c_str()}; }

    inline LogHelper<Priority::Debug> debug() const noexcept
    { return {m_backend, m_prefix.c_str()}; }

    inline LogHelper<Priority::FullDebug> fullDebug() const noexcept
    { return {m_backend, m_prefix.c_str()}; }

    template <typename T>
    inline static Hex<T> hex(T const value) noexcept { return {value}; }

private: /* Fields: */

    Backend & m_backend;
    const std::string m_prefix;

}; /* class Logger { */

} /* namespace LogHard { */

#endif /* LOGHARD_LOGGER_H */
